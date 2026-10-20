"""Offline regression for ORIGINAL Ape Escape 3 ISO extraction logic.

This verifies archive bounds, no asset substitutions, original byte identity,
SZ decoding, original ELF extraction and full VFI asset selection. Fixture
data is for parser testing ONLY; no standalone synthetic game is produced.
"""
from __future__ import annotations
import importlib.util
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch
import zlib

spec=importlib.util.spec_from_file_location(
    "actual_iso_extractor",Path(__file__).with_name("extract_retail_iso.py"))
module=importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class RetailIsoExtractionTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root=Path(self.tmp.name)
        self.image=self.root/"Original_AE3_USA.iso"
        self.elf=b"\x7fELF"+bytes(300)
        self.lua=b"\x1bLua\x50"+bytes(range(64))
        compressor=zlib.compressobj(level=9,wbits=-15)
        self.sz=struct.pack("<I",len(self.lua))+compressor.compress(self.lua)+compressor.flush()
        self.pck=b"PCK\x00"+bytes(128)
        self.binary=bytearray(32000)
        self.binary[1024:1024+len(self.elf)]=self.elf
        self.binary[4096:4096+len(self.lua)]=self.lua
        self.binary[8192:8192+len(self.sz)]=self.sz
        self.binary[16384:16384+len(self.pck)]=self.pck
        self.image.write_bytes(self.binary)
        self.vfi={
            "debug/us/stage/woods_c/area.luc":
                ("debug/us/stage/woods_c/area.luc",4096,len(self.lua)),
            "debug/us/stage/woods_c/controller.luc.sz":
                ("debug/us/stage/woods_c/controller.luc.sz",8192,len(self.sz)),
            "debug/us/stage/woods_c/bg.pck":
                ("debug/us/stage/woods_c/bg.pck",16384,len(self.pck)),
        }

    def run_extraction(self,out,all_resources=False,vfi=None):
        with (patch.object(module,"find_databin",return_value=(0,32000)),
              patch.object(module,"vfi_entries",return_value=self.vfi if vfi is None else vfi),
              patch.object(module,"root_entries",return_value={"SCUS_975.01":(1024,len(self.elf))})):
            return module.extract(self.image,out,all_resources=all_resources)

    def test_recovers_all_original_lua_even_sz_no_replacement(self):
        out=self.root/"private_lua"
        report=self.run_extraction(out)
        self.assertEqual(report["original_lua_scripts"],2)
        self.assertEqual(report["original_vfi_files_extracted"],2)
        self.assertEqual(report["original_sz_members_decompressed"],1)
        self.assertEqual((out/"vfi/debug/us/stage/woods_c/area.luc").read_bytes(),self.lua)
        self.assertEqual((out/"vfi/debug/us/stage/woods_c/controller.luc").read_bytes(),self.lua)
        self.assertFalse((out/"vfi/debug/us/stage/woods_c/bg.pck").exists())
        self.assertEqual((out/"SCUS_975.01").read_bytes(),self.elf)
        self.assertTrue((out/"original_iso_recovery_manifest.json").exists())
        self.assertTrue(report["not_playable"])

    def test_complete_mode_also_keeps_original_game_pck(self):
        out=self.root/"entire_vfi_private"
        report=self.run_extraction(out,all_resources=True)
        self.assertEqual(report["original_vfi_files_extracted"],3)
        self.assertEqual((out/"vfi/debug/us/stage/woods_c/bg.pck").read_bytes(),self.pck)
        self.assertEqual(report["resource_counts"][".pck"],1)

    def test_original_file_collision_and_unsafe_paths_fail_closed(self):
        out=self.root/"unsafe"
        bad=dict(self.vfi)
        bad["debug/us/stage/woods_c/controller.luc"]=[
            "debug/us/stage/woods_c/controller.luc",8192,len(self.sz)]
        with self.assertRaisesRegex(ValueError,"Collision"):
            self.run_extraction(out,vfi=bad)
        self.assertFalse(out.exists())
        bad={"../lost.luc":("../lost.luc",4096,len(self.lua))}
        with self.assertRaisesRegex(ValueError,"Unsafe"):
            self.run_extraction(out,vfi=bad)
        self.assertFalse(out.exists())

    def test_bad_sz_size_does_not_masquerade_as_retail_script(self):
        wrong=self.root/"corrupt.iso"
        data=bytearray(self.binary)
        data[8192:8196]=struct.pack("<I",len(self.lua)+1)
        wrong.write_bytes(data)
        with (patch.object(module,"find_databin",return_value=(0,32000)),
              patch.object(module,"vfi_entries",return_value=self.vfi),
              patch.object(module,"root_entries",return_value={"SCUS_975.01":(1024,len(self.elf))})):
            with self.assertRaisesRegex(ValueError,"Invalid original SZ"):
                module.extract(wrong,self.root/"bad_sz_out")

    def test_occupied_private_destination_never_overwritten(self):
        out=self.root/"owner_output"
        out.mkdir()
        (out/"existing.luc").write_bytes(b"original data")
        with self.assertRaises(FileExistsError):
            self.run_extraction(out)
        self.assertEqual((out/"existing.luc").read_bytes(),b"original data")


if __name__=="__main__":
    unittest.main()
