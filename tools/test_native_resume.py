"""Game-data-free tests for durable native EE AOT discovery and resumable rebuilds."""
from __future__ import annotations

import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import aot_cache
import bulk_recompile as bulk


class PersistentGuestEntries(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.out = Path(self.tmp.name)
        self.records = {p: 0x24020001 for p in range(0x100100, 0x100120, 4)}
        self.groups = {"09": len(self.records)}
        self.calls = {0x10010c: 2}
        self.ctors = [(0x300100, 0x100104)]
        self.anchor_patch = patch.object(bulk, "ANCHORS", (0x100100,))
        self.anchor_patch.start()

    def tearDown(self):
        self.anchor_patch.stop()
        self.tmp.cleanup()

    def emit(self, mode="constructors", **kwargs):
        return bulk.write_outputs(self.out, self.records, self.groups,
                                  self.calls, self.ctors, mode, **kwargs)

    def test_previous_main_and_overlay_misses_are_preserved(self):
        report, file = self.emit()
        self.assertEqual(report["total_unique_seed_entries"], 2)
        with file.open("a", encoding="ascii") as dst:
            dst.write("00100108\n00110000_AB12CD34:0\n")
        report, _ = self.emit()
        self.assertEqual(report["total_compile_entries"], 4)
        self.assertEqual(report["preserved_discovered_entries"], 2)
        self.assertEqual(report["preserved_module_entries"], 1)
        self.assertEqual(file.read_text().splitlines(),
                         ["00100100", "00100104", "00100108", "00110000_AB12CD34:0"])

    def test_reseed_upgrade_monotonic_and_stable_mtime(self):
        _, file = self.emit()
        file.write_text(file.read_text() + "00100108\n")
        self.emit("common")
        self.assertEqual(set(file.read_text().splitlines()),
                         {"00100100", "00100104", "00100108", "0010010c"})
        before = file.stat().st_mtime_ns
        self.emit("common")
        self.assertEqual(file.stat().st_mtime_ns, before)

    def test_invalid_stale_inventory_entry_rejected_without_mutation(self):
        _, file = self.emit()
        file.write_text("00100100\n00999990\n")
        original = file.read_bytes()
        with self.assertRaisesRegex(ValueError, "outside original EE inventory"):
            self.emit()
        self.assertEqual(file.read_bytes(), original)

    def test_malformed_entry_rejected_without_mutation(self):
        _, file = self.emit()
        file.write_text("00100100\nnot a valid entry\n")
        original = file.read_bytes()
        with self.assertRaisesRegex(ValueError, "Invalid indirect entry"):
            self.emit()
        self.assertEqual(file.read_bytes(), original)

    def test_explicit_reset_discards_stale_discovery(self):
        _, file = self.emit()
        file.write_text(file.read_text() + "00100108\n00110000_AB12CD34:0\n")
        report, _ = self.emit(reset_entries=True)
        self.assertTrue(report["entries_reset_explicitly"])
        self.assertEqual(file.read_text().splitlines(), ["00100100", "00100104"])


class NativeTranslationCache(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.work = Path(self.tmp.name)
        self.elf = self.work / "synthetic.elf"
        self.compiler = self.work / "eerecomp.exe"
        self.entries = self.work / "entries.txt"
        self.out = self.work / "cpp"
        self.elf.write_bytes(b"synthetic ELF, no commercial code")
        self.compiler.write_bytes(b"mock AOT compiler v1")
        self.entries.write_text("00100100\n", encoding="ascii")
        self.commands = []

    def tearDown(self):
        self.tmp.cleanup()

    def fake_run(self, command, **kwargs):
        self.commands.append(command)
        self.out.mkdir(parents=True, exist_ok=True)
        (self.out / "registration.cpp").write_text("void game() {}\n")

    def compile(self, **kwargs):
        return aot_cache.compile_cached(self.compiler, self.elf,
                                        self.entries, self.out, **kwargs)

    def test_first_translation_then_identical_rebuild_reuses(self):
        with patch.object(aot_cache.subprocess, "run", side_effect=self.fake_run):
            first = self.compile()
            second = self.compile()
        self.assertEqual(first["mode"], "translated")
        self.assertEqual(second["mode"], "reused")
        self.assertEqual(len(self.commands), 1)
        self.assertTrue((self.out / ".ae3-aot-cache.json").is_file())
        self.assertFalse(second["gameplay_verified"])

    def test_any_changed_code_entry_and_compiler_invalidates(self):
        with patch.object(aot_cache.subprocess, "run", side_effect=self.fake_run):
            self.compile()
            self.entries.write_text("00100100\n00100140\n")
            self.assertEqual(self.compile()["mode"], "translated")
            self.compiler.write_bytes(b"mock AOT compiler v2")
            self.assertEqual(self.compile()["mode"], "translated")
        self.assertEqual(len(self.commands), 3)

    def test_snr2_and_dynamic_code_window_content_invalidates(self):
        modules = self.work / "modules"
        overlays = self.work / "overlays"
        modules.mkdir()
        overlays.mkdir()
        module_file = modules / "system.snr"
        image_file = overlays / "00110000_DEADBEEF.bin"
        module_file.write_bytes(b"SNR2-example")
        image_file.write_bytes(b"code image #1")
        with patch.object(aot_cache.subprocess, "run", side_effect=self.fake_run):
            self.compile(modules=modules, overlays=overlays)
            self.assertEqual(self.compile(modules=modules, overlays=overlays)["mode"], "reused")
            module_file.write_bytes(b"SNR2-changed")
            self.assertEqual(self.compile(modules=modules, overlays=overlays)["mode"], "translated")
            image_file.write_bytes(b"code image #2")
            self.assertEqual(self.compile(modules=modules, overlays=overlays)["mode"], "translated")
        self.assertEqual(len(self.commands), 3)
        self.assertIn("--modules", self.commands[0])
        self.assertIn("--overlays", self.commands[0])

    def test_lost_cpp_output_forces_accurate_rebuild(self):
        with patch.object(aot_cache.subprocess, "run", side_effect=self.fake_run):
            self.compile()
            (self.out / "registration.cpp").unlink()
            self.assertEqual(self.compile()["mode"], "translated")
        self.assertEqual(len(self.commands), 2)

    def test_forced_rebuild_bypasses_cache(self):
        with patch.object(aot_cache.subprocess, "run", side_effect=self.fake_run):
            self.compile()
            self.assertEqual(self.compile(force=True)["mode"], "translated")
        self.assertEqual(len(self.commands), 2)

    def test_failed_retranslation_does_not_poison_last_known_good_stamp(self):
        with patch.object(aot_cache.subprocess, "run", side_effect=self.fake_run):
            self.compile()
        previous = (self.out / ".ae3-aot-cache.json").read_bytes()
        self.entries.write_text("00100100\n00100200\n")
        with patch.object(aot_cache.subprocess, "run", side_effect=RuntimeError("failed")):
            with self.assertRaisesRegex(RuntimeError, "failed"):
                self.compile()
        self.assertEqual((self.out / ".ae3-aot-cache.json").read_bytes(), previous)

    def test_success_without_cpp_is_rejected(self):
        with patch.object(aot_cache.subprocess, "run", return_value=None):
            with self.assertRaisesRegex(ValueError, "no C\+\+"):
                self.compile()
        self.assertFalse((self.out / ".ae3-aot-cache.json").exists())


if __name__ == "__main__":
    unittest.main(verbosity=2)
