"""Original stage Lua/PLC association checks using synthetic structural input only."""
from __future__ import annotations

from pathlib import Path
import importlib.util
import json
import math
import struct
import sys
import tempfile
import unittest


FILE = Path(__file__).resolve().parents[1] / "Plugins/Getchu3Importer/Content/Python/getchu3_stage_catalog.py"
spec = importlib.util.spec_from_file_location("getchu3_stage_catalog", FILE)
catalog = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = catalog
spec.loader.exec_module(catalog)


def plc_fixture(records):
    data = bytearray(0xC0 + 0x80 * len(records))
    struct.pack_into("<I", data, 0, len(records))
    for n, (name, xyz) in enumerate(records):
        at = 0xC0 + n * 0x80
        assert len(name.encode("ascii")) <= 32
        data[at:at + len(name)] = name.encode("ascii")
        struct.pack_into("<fff", data, at + 0x40, *xyz)
    return bytes(data)


class OriginalStageCatalogTests(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = Path(temp.name)
        self.vfi = self.root / "original/vfi"
        self.models = self.root / "converted/models"
        self.vfi.mkdir(parents=True)
        self.models.mkdir(parents=True)
        self.output = self.root / "stage_catalog.json"

    def source(self, path, data, *, model=False):
        base = self.models if model else self.vfi
        file = base / path
        file.parent.mkdir(parents=True, exist_ok=True)
        file.write_bytes(data)
        return file

    def add_stage(self, stage):
        self.source(f"debug/us/stage/{stage}/area.luc", b"\x1bLua\x50" + b"\0" * 20)

    def test_plc_recovery_preserves_unchanged_xyz_and_identifiers(self):
        data = plc_fixture([("ape", (1.0, -3.5, 14.25)), ("monkey", (0.0, 0.0, 2.0))])
        result = catalog.read_original_plc(data)
        self.assertEqual(result["declared_records"], 2)
        self.assertEqual(result["placements"][0]["original_xyz"], [1.0, -3.5, 14.25])
        self.assertEqual(result["placements"][1]["name"], "monkey")
        self.assertTrue(result["rotation_and_scale_not_recovered"])

    def test_invalid_source_rows_remain_counted(self):
        data = plc_fixture([("ok", (0, 0, 0)), ("bad", (math.nan, 0, 0))])
        result = catalog.read_original_plc(data)
        self.assertEqual(result["rejected_record_indices"], [1])
        self.assertEqual(len(result["placements"]), 1)

    def test_unverifiable_or_truncated_plc_fails(self):
        with self.assertRaisesRegex(ValueError, "size"):
            catalog.read_original_plc(b"\0" * 16)
        with self.assertRaisesRegex(ValueError, "truncated"):
            catalog.read_original_plc(b"\x05\0\0\0" + b"\0" * 188)

    def test_real_stage_token_is_resolved_without_guessing(self):
        self.assertEqual(catalog.stage_from_path(Path("debug/us/stage/woods_c/area.luc")), "woods_c")
        self.assertIsNone(catalog.stage_from_path(Path("other/woods_c/area.luc")))
        self.assertIsNone(catalog.stage_from_path(Path("stage/../area.luc")))
        self.assertTrue(catalog.matches_original_placement("NPC_ape", "ape"))
        self.assertFalse(catalog.matches_original_placement("not_an_apeX", "ape"))

    def test_original_catalog_builds_source_links_without_fake_gameplay(self):
        self.add_stage("woods_c")
        self.source("debug/us/stage/woods_c/placeholder.plc",
                    plc_fixture([("ape", (3, 2, 1))]))
        self.source("debug/us/stage/woods_c/npc_ape.glb", b"not a game asset", model=True)
        report = catalog.build_original_stage_catalog(self.vfi, self.models, self.output)
        self.assertEqual(report["stage_count"], 1)
        self.assertEqual(report["stage_scripts"], 1)
        entry = report["stages"][0]
        self.assertEqual(entry["stage"], "woods_c")
        self.assertEqual(entry["translation_only_associations"][0]["resolution"], "unique")
        self.assertFalse(entry["original_gameplay_ready"])
        self.assertFalse(entry["authentic_level_transform_complete"])
        self.assertEqual(report, json.loads(self.output.read_text(encoding="utf-8")))

    def test_ambiguous_models_are_not_assigned_randomly(self):
        self.add_stage("woods_c")
        self.source("stage/woods_c/placeholder.plc", plc_fixture([("ape", (0, 0, 0))]))
        self.source("stage/woods_c/big_ape.glb", b"model1", model=True)
        self.source("stage/woods_c/small_ape.glb", b"model2", model=True)
        report = catalog.build_original_stage_catalog(self.vfi, self.models, self.output)
        placement = report["stages"][0]["translation_only_associations"][0]
        self.assertEqual(placement["resolution"], "ambiguous")
        self.assertEqual(len(placement["matching_models"]), 2)

    def test_no_original_area_scripts_rejected(self):
        self.source("stage/test/placeholder.plc", plc_fixture([("ape", (0, 0, 0))]))
        with self.assertRaisesRegex(RuntimeError, "No original stage area.luc"):
            catalog.build_original_stage_catalog(self.vfi, self.models, self.output)
        self.assertFalse(self.output.exists())

    def test_corrupt_original_lua_rejected(self):
        self.source("stage/test/area.luc", b"not genuine Lua bytecode")
        with self.assertRaisesRegex(ValueError, "not original Lua 5.0"):
            catalog.build_original_stage_catalog(self.vfi, self.models, self.output)

    def test_generated_private_catalog_cannot_overwrite_retail_sources(self):
        self.add_stage("test")
        with self.assertRaisesRegex(ValueError, "outside original"):
            catalog.build_original_stage_catalog(
                self.vfi, self.models, self.vfi / "unsafe.json")


if __name__ == "__main__":
    unittest.main()
