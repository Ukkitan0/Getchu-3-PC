"""Original-only Lua/ELF callback reconciliation integration tests (no retail files)."""
from __future__ import annotations

import hashlib
import importlib.util
from pathlib import Path
import sys
import tempfile
import types
import unittest
from unittest.mock import patch

SCRIPT = Path(__file__).resolve().parents[1] / (
    "Plugins/Getchu3Importer/Content/Python/getchu3_original_bindings.py")
spec = importlib.util.spec_from_file_location("getchu3_original_bindings", SCRIPT)
bindings = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = bindings
spec.loader.exec_module(bindings)


class VerifiedOriginalBindingEvidenceTests(unittest.TestCase):
    def setUp(self):
        holder = tempfile.TemporaryDirectory()
        self.addCleanup(holder.cleanup)
        self.root = Path(holder.name)
        self.extracted = self.root / "owner-data"
        self.stage = self.extracted / "vfi/debug/us/stage"
        self.stage.mkdir(parents=True)
        self.script = self.stage / "woods_c/area.luc"
        self.script.parent.mkdir()
        self.script.write_bytes(b"\x1bLua\x50" + b"\x00" * 25)
        self.extra = self.stage / "woods_c/logic.luc"
        self.extra.write_bytes(b"\x1bLua\x50" + b"\x00" * 25)
        self.source = self.extracted / "SCUS_975.01"
        self.source.write_bytes(b"retail game binary (test fake)")
        self.repo = self.root / "repo"
        (self.repo / "native/src").mkdir(parents=True)
        (self.repo / "native/tools").mkdir()
        (self.repo / "native/src/lua_stage.cpp").write_text("registerCallback")
        self.output = self.root / "private/output.json"

    def mock_source_tools(self, *, digest=None, total=2, rows=None):
        checksum = digest or hashlib.sha256(self.source.read_bytes()).hexdigest()
        elf = types.ModuleType("map_original_lua_registrations")
        elf.US_SHA256 = checksum
        elf.inspect = lambda raw: {"original_verified_elf_sha256": checksum}
        lua = types.ModuleType("audit_original_lua50_calls")
        lua.audit = lambda folder, host: {"scripts": [{}] * total}
        lua.host_callbacks = lambda host: ["new_ent"]
        reconciliation = types.ModuleType("reconcile_original_lua_bindings")
        reconciliation.reconcile = lambda calls, elf_record, names: {
            "schema": "getchu3-original-lua50-binding-evidence/v1",
            "retail_scripts_count": total,
            "verified_usa_elf_sha256": checksum,
            "rows": rows or [
                {"name": "new_ent", "priority": "present_name_semantics_not_verified",
                 "original_register_sites": ["0x00255BD8"],
                 "published_original_vtable": None},
                {"name": "missing_native", "priority": "original_direct_call_missing_native_host",
                 "original_register_sites": ["0x00123456"],
                 "published_original_vtable": None}],
            "observed_actor_class_literals": ["ape"],
            "observed_controller_command_literals": ["walk"]}
        return {
            "audit_original_lua50_calls": lua,
            "map_original_lua_registrations": elf,
            "reconcile_original_lua_bindings": reconciliation}

    def test_verified_elf_and_all_original_lua_chunks_are_audited(self):
        with patch.dict(sys.modules, self.mock_source_tools()):
            result = bindings.create_original_binding_report(
                self.extracted, self.repo, self.output)
        self.assertEqual(result["retail_lua_chunks_analyzed"], 2)
        self.assertEqual(result["retail_area_scripts_found"], 1)
        self.assertEqual(result["unresolved_original_bindings"][0]["name"], "missing_native")
        self.assertFalse(result["source_verified_behavior_implemented"])
        self.assertFalse(result["gameplay_ready"])
        self.assertTrue(self.output.is_file())

    def test_unverified_elf_hash_never_generates_evidence(self):
        wrong_hash = "0" * 64
        with patch.dict(sys.modules, self.mock_source_tools(digest=wrong_hash)):
            with self.assertRaisesRegex(ValueError, "differs"):
                bindings.create_original_binding_report(
                    self.extracted, self.repo, self.output)
        self.assertFalse(self.output.exists())

    def test_unaccounted_extra_lua_chunks_rejected(self):
        with patch.dict(sys.modules, self.mock_source_tools(total=1)):
            with self.assertRaisesRegex(RuntimeError, "every original"):
                bindings.create_original_binding_report(
                    self.extracted, self.repo, self.output)
        self.assertFalse(self.output.exists())

    def test_missing_area_script_rejected(self):
        self.script.unlink()
        with patch.dict(sys.modules, self.mock_source_tools()):
            with self.assertRaisesRegex(ValueError, "area.luc"):
                bindings.create_original_binding_report(
                    self.extracted, self.repo, self.output)


if __name__ == "__main__":
    unittest.main()
