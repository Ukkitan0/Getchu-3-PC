"""Test original-source to imported Unreal UObject mapping with no retail bytes."""
from __future__ import annotations

from pathlib import Path
import importlib.util
import json
import sys
import tempfile
import unittest

FILE = Path(__file__).resolve().parents[1] / "Plugins/Getchu3Importer/Content/Python/getchu3_asset_registry.py"
spec = importlib.util.spec_from_file_location("getchu3_asset_registry", FILE)
registry = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = registry
spec.loader.exec_module(registry)


def sample_catalog(candidates=None, resolution="unique"):
    candidates = ["stage/woods_c/npc_ape.glb"] if candidates is None else candidates
    return {
        "schema": "getchu3-original-stage-catalog/v1", "gameplay_ready": False,
        "stages": [{
            "stage": "woods_c",
            "area_lua_sources": [{"original_path":"stage/woods_c/area.luc", "sha256":"1" * 64}],
            "translation_only_associations": [{
                "plc": "stage/woods_c/placeholder.plc", "source_record": 7,
                "original_entity_name": "ape", "original_xyz": [1.25, -2.5, 3.75],
                "matching_models": candidates, "resolution": resolution
            }]
        }]
    }


class AssetRegistryTests(unittest.TestCase):
    def setUp(self):
        folder = tempfile.TemporaryDirectory()
        self.addCleanup(folder.cleanup)
        self.dest = Path(folder.name) / "registry.json"

    def test_exact_original_plc_record_links_to_imported_unreal_model(self):
        imported = {"models": [{
            "source_relative": "stage/woods_c/npc_ape.glb",
            "unreal_object_paths": ["/Game/ApeEscape3/Original/Models/npc_ape.npc_ape"]
        }], "textures": [], "streams": [], "collision": []}
        result = registry.build_asset_registry(sample_catalog(), imported, self.dest)
        placement = result["original_stages"][0]["resolved_source_placements"][0]
        self.assertEqual(placement["resolution"], "linked_original_model_source")
        self.assertEqual(placement["original_xyz"], [1.25, -2.5, 3.75])
        self.assertEqual(placement["source_record"], 7)
        self.assertEqual(len(placement["unreal_imported_objects"]), 1)
        self.assertFalse(result["gameplay_ready"])
        self.assertFalse(result["original_scene_placement_implemented"])
        self.assertEqual(json.loads(self.dest.read_text()), result)

    def test_unique_unimported_model_is_blocked(self):
        with self.assertRaisesRegex(RuntimeError, "not imported"):
            registry.build_asset_registry(sample_catalog(), {"models": []}, self.dest)
        self.assertFalse(self.dest.exists())

    def test_ambiguous_models_never_pick_one(self):
        catalog = sample_catalog(["stage/woods_c/npc_ape.glb",
                                  "stage/woods_c/boss_ape.glb"], "ambiguous")
        report = registry.build_asset_registry(catalog, {"models": []}, self.dest)
        placement = report["original_stages"][0]["resolved_source_placements"][0]
        self.assertEqual(placement["resolution"], "ambiguous_unresolved")
        self.assertEqual(placement["unreal_imported_objects"], [])
        self.assertEqual(len(report["ambiguous_placement_matches"]), 1)

    def test_absent_original_model_never_fabricated(self):
        catalog = sample_catalog([], "missing")
        report = registry.build_asset_registry(catalog, {"models": []}, self.dest)
        self.assertEqual(report["original_stages"][0]["resolved_source_placements"][0]["resolution"],
                         "missing_original_model_source")

    def test_source_path_traversal_and_absolute_path_are_rejected(self):
        for name in ["../outside.glb", "/Users/original.glb", "a\\b.glb", "a/\0/b.glb"]:
            with self.subTest(name=name):
                with self.assertRaises(ValueError):
                    registry.normalized_relative(name)

    def test_duplicate_case_insensitive_model_identifiers_rejected(self):
        imported = {"models": [
            {"source_relative": "stage/a/Test.glb",
             "unreal_object_paths": ["/Game/A/A.A"]},
            {"source_relative": "stage/a/test.glb",
             "unreal_object_paths": ["/Game/A/B.B"]}
        ]}
        with self.assertRaisesRegex(ValueError, "case-colliding"):
            registry.build_asset_registry(sample_catalog([], "missing"), imported, self.dest)

    def test_bad_unreal_imported_object_is_rejected(self):
        imported = {"models": [{
            "source_relative": "stage/woods_c/npc_ape.glb",
            "unreal_object_paths": []
        }]}
        with self.assertRaisesRegex(ValueError, "no usable"):
            registry.build_asset_registry(sample_catalog(), imported, self.dest)

    def test_untrusted_catalog_rejected(self):
        with self.assertRaisesRegex(ValueError, "source-validated"):
            registry.build_asset_registry({"schema": "fake", "stages": []},
                                          {"models": []}, self.dest)

    def test_runtime_claim_cannot_be_inserted_by_caller(self):
        data = sample_catalog()
        data["gameplay_ready"] = True
        with self.assertRaisesRegex(ValueError, "cannot certify"):
            registry.build_asset_registry(data, {}, self.dest)


if __name__ == "__main__":
    unittest.main()
