"""Game-data-free verification of Ape Escape 3 Lua 5.0 stage recovery."""
from __future__ import annotations

from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import Mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import recover_lua50


class LocalLuaStageRecovery(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.input = self.root / "owned"
        self.output = self.root / "private"
        self.jar = self.root / "unluac.jar"
        self.jar.write_bytes(b"synthetic dev-only fixture")
        self.input.mkdir()

    def tearDown(self):
        self.tmp.cleanup()

    def fixture(self, name, contents):
        path = self.input / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(contents)
        return path

    def test_stage_recovery_keeps_original_bytes_untouched(self):
        original = b"\x1bLua\x50\x00\x01" + b"\x19" * 16
        self.fixture("cty_a/area.luc", original)
        self.fixture("cty_b/area.luc", b"\x1bLua\x51fake")
        self.fixture("cty_c/other.luc", original)
        self.fixture("irrelevant.bin", b"do not decode")

        calls = []
        def fake_runner(command, **kwargs):
            calls.append((command, kwargs))
            return subprocess.CompletedProcess(command, 0,
                 stdout=b"function setupArea(opts)\nend\n", stderr=b"")

        result = recover_lua50.recover(
            self.input, self.output, self.jar, runner=fake_runner)
        self.assertEqual(result["script_files_found"], 2)
        self.assertEqual(result["counters"]["recovered"], 1)
        self.assertEqual(result["counters"]["invalid_or_different_version"], 1)
        self.assertEqual(len(calls), 1)
        self.assertEqual(calls[0][0][1:3], ["-jar", str(self.jar)])
        self.assertEqual(
            (self.output / "cty_a/area.lua").read_text(),
            "function setupArea(opts)\nend\n")
        self.assertEqual((self.input / "cty_a/area.luc").read_bytes(), original)
        self.assertFalse((self.output / "cty_c/other.lua").exists())
        self.assertFalse(result["native_runtime_java_dependency"])

    def test_opt_in_other_scripts_and_missing_debug_failure(self):
        self.fixture("area.luc", b"\x1bLua\x50correct")
        self.fixture("other.luc", b"\x1bLua\x50another")
        runner = Mock(return_value=subprocess.CompletedProcess([], 1,
                                 stdout=b"", stderr=b"debug info missing"))
        result = recover_lua50.recover(
            self.input, self.output, self.jar,
            all_scripts=True, runner=runner)
        self.assertEqual(result["script_files_found"], 2)
        self.assertEqual(result["counters"]["failed"], 2)
        self.assertFalse((self.output / "area.lua").exists())

    def test_extracted_folder_must_be_separate_from_destination(self):
        with self.assertRaisesRegex(ValueError, "outside"):
            recover_lua50.recover(self.input, self.input / "recovered", self.jar)

    def test_oversized_stage_skipped_safely(self):
        path = self.fixture("area.luc", b"\x1bLua\x50")
        with path.open("r+b") as stream:
            stream.truncate(recover_lua50.MAX_SCRIPT + 1)
        result = recover_lua50.recover(
            self.input, self.output, self.jar,
            runner=Mock(side_effect=AssertionError("should not invoke Java")))
        self.assertEqual(result["counters"]["skipped_large"], 1)

    def test_no_games_cannot_produce_fake_progress(self):
        result = recover_lua50.recover(self.input, self.output, self.jar)
        self.assertEqual(result["script_files_found"], 0)
        self.assertEqual(result["counters"]["recovered"], 0)


if __name__ == "__main__":
    unittest.main()
