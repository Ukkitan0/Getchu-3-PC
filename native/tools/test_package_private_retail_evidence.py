"""Private original Lua / executable handoff selection guards."""
from __future__ import annotations
import importlib.util
from pathlib import Path
import tempfile
import unittest
import zipfile

spec=importlib.util.spec_from_file_location(
    "owner_private_evidence",Path(__file__).with_name("package_private_retail_evidence.py"))
module=importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class PrivateOriginalEvidenceTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root=Path(self.tmp.name)
        self.source=self.root/"private"
        self.source.mkdir()
        (self.source/"SCUS_975.01").write_bytes(b"\x7fELF"+b"\x00"*100)
        (self.source/"original_iso_recovery_manifest.json").write_text('{"original":true}')
        (self.source/"original_lua_engine_api_audit.json").write_text('{"engine_calls":[]}')
        self.stage=self.source/"vfi"/"debug"/"us"/"stage"/"woods_c"
        self.stage.mkdir(parents=True)
        self.chunk=b"\x1bLua\x50"+b"\x00"*64
        (self.stage/"area.luc").write_bytes(self.chunk)
        (self.stage/"bg.pck").write_bytes(b"PCK\x00"+b"\x00"*40)
        (self.stage/"placeholder.plc").write_bytes(b"\x00"*192)

    def test_original_scripts_and_elf_without_entire_disc(self):
        dest=self.root/"handoff.zip"
        result=module.package(self.source,dest)
        with zipfile.ZipFile(dest) as archive:
            names=set(archive.namelist())
            self.assertIn("SCUS_975.01",names)
            self.assertIn("vfi/debug/us/stage/woods_c/area.luc",names)
            self.assertNotIn("vfi/debug/us/stage/woods_c/bg.pck",names)
            self.assertEqual(archive.read("vfi/debug/us/stage/woods_c/area.luc"),self.chunk)
        self.assertFalse(result["synthetic_game_resources"])
        self.assertEqual((self.stage/"area.luc").read_bytes(),self.chunk)

    def test_selected_original_stage_adds_real_pck_and_plc(self):
        dest=self.root/"woods_c_handoff.zip"
        module.package(self.source,dest,"woods_c")
        with zipfile.ZipFile(dest) as archive:
            names=set(archive.namelist())
            self.assertIn("vfi/debug/us/stage/woods_c/bg.pck",names)
            self.assertIn("vfi/debug/us/stage/woods_c/placeholder.plc",names)
        with self.assertRaises(FileExistsError):
            module.package(self.source,dest,"woods_c")

    def test_optional_executable_registration_report(self):
        (self.source/"original_elf_lua_registrations.json").write_text('{"names":[]}')
        dest=self.root/"with_registration.zip"
        module.package(self.source,dest)
        with zipfile.ZipFile(dest) as archive:
            self.assertIn("original_elf_lua_registrations.json",archive.namelist())


if __name__=="__main__":
    unittest.main()
