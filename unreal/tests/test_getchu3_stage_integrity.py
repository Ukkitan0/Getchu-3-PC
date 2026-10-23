"""Owner-original stage SHA-256 and private path verification contract tests."""
import hashlib
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] /
                       "Plugins/Getchu3Importer/Content/Python"))
from getchu3_stage_integrity import source_path, validate_stage_catalog


class SourceVerificationTests(unittest.TestCase):
    def setUp(self):
        self.work = tempfile.TemporaryDirectory()
        self.addCleanup(self.work.cleanup)
        self.root = Path(self.work.name)
        self.filename = self.root / "debug/us/stage/woods_c/area.luc"
        self.filename.parent.mkdir(parents=True)
        self.payload = b"\x1bLua\x50" + b"\0" * 65
        self.filename.write_bytes(self.payload)
        self.stage = {
            "stage": "woods_c",
            "area_lua_sources": [{
                "original_path": "debug/us/stage/woods_c/area.luc",
                "sha256": hashlib.sha256(self.payload).hexdigest()
            }]
        }
        self.catalog = {
            "schema": "getchu3-original-stage-catalog/v1",
            "gameplay_ready": False, "stages": [self.stage]
        }

    def test_verified_original_script_and_stage_id(self):
        result = validate_stage_catalog(self.root, self.catalog)
        self.assertEqual(result["verified_area_scripts"], 1)
        self.assertFalse(result["gameplay_ready"])
        self.assertEqual(result["stages"][0]["stage"], "woods_c")

    def test_script_tamper_is_rejected(self):
        self.filename.write_bytes(self.payload + b"tampered")
        with self.assertRaisesRegex(ValueError, "mismatch"):
            validate_stage_catalog(self.root, self.catalog)

    def test_stage_id_path_escape_rejected(self):
        for path in (
            "../area.luc", "debug/us/stage/other/area.luc",
            "debug/us/stage/woods_c/../../area.luc",
            r"debug\us\stage\woods_c\area.luc"
        ):
            with self.subTest(path=path):
                with self.assertRaises(ValueError):
                    source_path("woods_c", path)

    def test_duplicate_stage_id_rejected(self):
        self.catalog["stages"].append({**self.stage, "stage": "WOODS_C"})
        with self.assertRaisesRegex(ValueError, "Duplicate"):
            validate_stage_catalog(self.root, self.catalog)

    def test_stage_without_script_is_not_certified(self):
        self.catalog["stages"].append({"stage": "other", "area_lua_sources": []})
        result = validate_stage_catalog(self.root, self.catalog)
        self.assertEqual(result["total_stage_ids"], 2)
        self.assertEqual(result["verified_area_scripts"], 1)
        self.assertIsNone(result["stages"][1]["source"])

    def test_missing_script_and_invalid_digest_rejected(self):
        self.filename.unlink()
        with self.assertRaises(FileNotFoundError):
            validate_stage_catalog(self.root, self.catalog)
        self.stage["area_lua_sources"][0]["sha256"] = "not-a-digest"
        with self.assertRaisesRegex(ValueError, "SHA-256"):
            validate_stage_catalog(self.root, self.catalog)


if __name__ == "__main__":
    unittest.main()
