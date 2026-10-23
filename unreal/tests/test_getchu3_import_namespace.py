"""Synthetic checks that each private original-ISO run gets non-destructive assets."""
from __future__ import annotations

from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] /
                       "Plugins/Getchu3Importer/Content/Python"))
from getchu3_import_namespace import original_import_package_root


class NonDestructiveOriginalImportTests(unittest.TestCase):
    def setUp(self):
        self.digest = "5b4c6272c9daf190b3f72081f124edd7ea2af3cc0dafeaef206ded97a022fd09"

    def test_original_disc_run_has_owned_unreal_namespace(self):
        original = Path("/private/Getchu3Private/5b4c6272c9daf190-foo_1234")
        package = original_import_package_root(self.digest, original)
        self.assertEqual(package,
                         "/Game/ApeEscape3/Original/Imports/"
                         "Disc_5b4c6272c9daf190_Run_foo_1234")
        self.assertNotIn("Getchu3Private", package)

    def test_repeat_original_disc_run_cannot_replace_previous_assets(self):
        a = original_import_package_root(self.digest,
                 Path("/private/5b4c6272c9daf190-abc_1234"))
        b = original_import_package_root(self.digest,
                 Path("/private/5b4c6272c9daf190-def_5678"))
        self.assertNotEqual(a.casefold(), b.casefold())

    def test_reject_other_original_disc_workspace(self):
        with self.assertRaisesRegex(ValueError, "does not belong"):
            original_import_package_root(
                self.digest, Path("/private/deadbeefdeadbeef-foo_1234"))

    def test_reject_bad_disc_digest(self):
        for item in ("", "a" * 64, "A" * 64, "e" * 63):
            with self.subTest(digest=item):
                with self.assertRaises(ValueError):
                    original_import_package_root(item,
                        Path("/private/5b4c6272c9daf190-abc_1234"))

    def test_reject_invalid_workspace_suffix(self):
        for path in ("5b4c6272c9daf190-foo-evil",
                     "5b4c6272c9daf190-evil.name",
                     "5b4c6272c9daf190-abc",
                     "5b4c6272c9daf190-abc_DEF",
                     "5b4c6272c9daf190-abc@123"):
            with self.subTest(path=path):
                with self.assertRaises(ValueError):
                    original_import_package_root(
                        self.digest, Path("/private") / path)


if __name__ == "__main__":
    unittest.main()
