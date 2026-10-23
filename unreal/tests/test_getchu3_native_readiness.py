"""Original Lua ELF native callback readiness audits with synthetic evidence."""
from __future__ import annotations

import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest


SOURCE = Path(__file__).resolve().parents[1] / (
    "Plugins/Getchu3Importer/Content/Python/getchu3_native_readiness.py")
SPEC = importlib.util.spec_from_file_location("getchu3_native_readiness", SOURCE)
readiness = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = readiness
SPEC.loader.exec_module(readiness)


def original_catalog():
    return {
        "schema": "getchu3-original-stage-catalog/v1",
        "gameplay_ready": False,
        "stages": [{"stage": "woods_c", "area_lua_sources": [
            {"original_path": "debug/us/stage/woods_c/area.luc",
             "sha256": "a" * 64}], "translation_only_associations": []}],
    }


def original_registry():
    return {
        "schema": "getchu3-source-asset-registry/v1",
        "gameplay_ready": False,
        "original_stages": [{
            "stage": "woods_c",
            "resolved_source_placements": [
                {"resolution": "linked_original_model_source"},
                {"resolution": "ambiguous_unresolved"}]}]
    }


def original_elf_evidence():
    return {
        "schema": "getchu3-original-lua50-binding-evidence/v1",
        "verified_usa_elf_sha256": "a" * 64,
        "original_stage_inputs": [
            {"path": "woods_c/area.luc",
             "direct_global_calls": ["new_ent", "unknown_native",
                                     "retail_only", "script_function"],
             "observed_actor_class_literals": ["monkey", "player"],
             "observed_controller_command_literals": ["run"]},
            {"path": "woods_c/event.luc",
             "direct_global_calls": ["new_ent", "gw_get_flag"],
             "observed_actor_class_literals": ["monkey"],
             "observed_controller_command_literals": []}
        ],
        "rows": [
            {"name": "new_ent", "defined_by_original_script": False,
             "present_in_native_host": True,
             "registered_at_scanned_original_CreateBind": True},
            {"name": "gw_get_flag", "defined_by_original_script": False,
             "present_in_native_host": True,
             "registered_at_scanned_original_CreateBind": True},
            {"name": "retail_only", "defined_by_original_script": False,
             "present_in_native_host": False,
             "registered_at_scanned_original_CreateBind": True,
             "original_register_sites": ["0x00123400"]},
            {"name": "script_function", "defined_by_original_script": True,
             "present_in_native_host": False,
             "registered_at_scanned_original_CreateBind": False}
        ]
    }


class NativeReadinessTests(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.output = Path(temp.name) / "private" / "ready.json"

    def test_reports_stage_specific_missing_calls_without_fake_behaviors(self):
        report = readiness.build_original_native_readiness(
            original_catalog(), original_elf_evidence(),
            original_registry(), self.output)
        self.assertEqual(report["stage_count"], 1)
        self.assertEqual(report["scripts_analyzed"], 2)
        self.assertEqual(report["stages_with_unresolved_calls"], 1)
        stage = report["stages"][0]
        self.assertEqual(stage["original_actor_classes_seen"], ["monkey", "player"])
        self.assertEqual(stage["original_controller_commands_seen"], ["run"])
        self.assertEqual(stage["native_host_name_exists_semantics_unverified"],
                         ["gw_get_flag", "new_ent"])
        self.assertEqual(stage["defined_in_original_lua"], ["script_function"])
        self.assertEqual(stage["unresolved_original_calls"],
                         [{"name": "unknown_native",
                           "reason": "not_in_elf_reconciliation"}])
        self.assertEqual(stage["original_elf_registered_but_not_host_implemented"],
                         [{"name": "retail_only",
                           "original_register_sites": ["0x00123400"]}])
        self.assertEqual(stage["plc_model_sources_linked"], 1)
        self.assertEqual(stage["plc_model_sources_unresolved"], 1)
        self.assertFalse(stage["original_actor_manager_complete"])
        self.assertFalse(report["gameplay_ready"])
        self.assertEqual(report, json.loads(self.output.read_text()))

    def test_false_gameplay_ready_claim_is_rejected(self):
        source = original_catalog()
        source["gameplay_ready"] = True
        with self.assertRaisesRegex(ValueError, "incorrectly claims"):
            readiness.build_original_native_readiness(
                source, original_elf_evidence(), original_registry(), self.output)
        self.assertFalse(self.output.exists())

    def test_unknown_stage_lua_is_not_silently_assigned_to_player_stage(self):
        evidence = original_elf_evidence()
        evidence["original_stage_inputs"][0]["path"] = "unknown/area.luc"
        with self.assertRaisesRegex(ValueError, "unknown stage"):
            readiness.build_original_native_readiness(
                original_catalog(), evidence, original_registry(), self.output)
        self.assertFalse(self.output.exists())

    def test_duplicate_elf_callback_identity_is_rejected(self):
        evidence = original_elf_evidence()
        evidence["rows"].append(dict(evidence["rows"][0]))
        with self.assertRaisesRegex(ValueError, "callback evidence identity"):
            readiness.build_original_native_readiness(
                original_catalog(), evidence, original_registry(), self.output)

    def test_imported_stage_identity_must_match_original_catalog(self):
        asset_report = original_registry()
        asset_report["original_stages"][0]["stage"] = "another_c"
        with self.assertRaisesRegex(ValueError, "does not cover"):
            readiness.build_original_native_readiness(
                original_catalog(), original_elf_evidence(), asset_report, self.output)

    def test_stage_source_path_traversal_is_rejected(self):
        evidence = original_elf_evidence()
        for source in ("../area.luc", "woods_c/../other.luc",
                       "woods_c\\area.luc", "/woods_c/area.luc"):
            with self.subTest(source=source):
                evidence["original_stage_inputs"][0]["path"] = source
                with self.assertRaisesRegex(ValueError, "(Unsafe|Invalid)"):
                    readiness.build_original_native_readiness(
                        original_catalog(), evidence, original_registry(), self.output)

    def test_independent_stages_cannot_inherit_other_stage_callbacks(self):
        catalog = original_catalog()
        catalog["stages"].append({"stage": "toy_c", "area_lua_sources": [],
                                  "translation_only_associations": []})
        registry = original_registry()
        registry["original_stages"].append({
            "stage": "toy_c", "resolved_source_placements": []})
        evidence = original_elf_evidence()
        evidence["original_stage_inputs"].append({
            "path": "toy_c/area.luc",
            "direct_global_calls": ["different_native"],
            "observed_actor_class_literals": ["toy_monkey"],
            "observed_controller_command_literals": ["jump"]})
        result = readiness.build_original_native_readiness(
            catalog, evidence, registry, self.output)
        self.assertEqual(result["stage_count"], 2)
        toy, woods = result["stages"]
        self.assertEqual(toy["original_stage"], "toy_c")
        self.assertEqual(toy["original_actor_classes_seen"], ["toy_monkey"])
        self.assertEqual(toy["unresolved_original_calls"],
                         [{"name": "different_native",
                           "reason": "not_in_elf_reconciliation"}])
        self.assertEqual(woods["original_stage"], "woods_c")
        self.assertEqual(woods["original_controller_commands_seen"], ["run"])
        self.assertNotIn("different_native", woods["direct_global_calls_observed"])

    def test_missing_verified_evidence_cannot_produce_native_ready_report(self):
        evidence = original_elf_evidence()
        evidence["schema"] = "unverified"
        with self.assertRaisesRegex(ValueError, "verified USA ELF"):
            readiness.build_original_native_readiness(
                original_catalog(), evidence, original_registry(), self.output)
        self.assertFalse(self.output.exists())

    def test_no_calls_still_does_not_claim_original_gameplay(self):
        evidence = original_elf_evidence()
        evidence["original_stage_inputs"] = [
            {"path": "woods_c/area.luc",
             "direct_global_calls": [],
             "observed_actor_class_literals": [],
             "observed_controller_command_literals": []}]
        result = readiness.build_original_native_readiness(
            original_catalog(), evidence, original_registry(), self.output)
        self.assertEqual(result["stages_with_unresolved_calls"], 0)
        self.assertFalse(result["stages"][0]["gameplay_ready"])
        self.assertFalse(result["stages"][0]["original_lua_callbacks_semantically_verified"])


if __name__ == "__main__":
    unittest.main()
