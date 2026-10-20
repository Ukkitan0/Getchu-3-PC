"""Original Ape Escape 3 PCK-to-native asset extraction contract tests."""
from __future__ import annotations
import importlib.util
from pathlib import Path
import struct
import tempfile
import unittest

spec=importlib.util.spec_from_file_location(
    "original_pck_unpacked",Path(__file__).with_name("unpack_original_stage_pck.py"))
module=importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class OriginalStagePckExtractionTests(unittest.TestCase):
    def setUp(self):
        self.temporary=tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root=Path(self.temporary.name)/"extracted"
        self.stage=self.root/"debug"/"us"/"stage"/"woods_c"
        self.stage.mkdir(parents=True)
        self.names=[b"mesh",b"collision",b"anim",b"texture"]
        self.data=[
            b"I3D_BIN\x00"+b"a"*32,
            b"I3D_I3C\x00"+b"b"*32,
            b"I3D_I3M\x00"+b"c"*32,
            b"TIM2"+b"d"*36,
        ]
        self.source=self.stage/"bg.pck"
        self.source.write_bytes(self.make_original_pck())

    def make_original_pck(self):
        pool=b""
        name_ids=[]
        for n in self.names:
            name_ids.append(len(pool))
            pool+=n+b"\x00"
        table=16+len(pool)+8
        entries=bytearray(4*16)
        offset=table+len(entries)
        data=bytearray()
        for i,blob in enumerate(self.data):
            struct.pack_into("<IIII",entries,i*16,name_ids[i],0,offset,len(blob))
            data+=blob
            offset+=len(blob)
        header=bytearray(16)
        header[:4]=b"PCK\x00"
        struct.pack_into("<II",header,4,table,4)
        return bytes(header+pool+b"\x00"*8+entries+data)

    def test_true_format_original_model_animation_and_texture_are_exposed(self):
        report=module.unpack(self.root,"woods_c")
        self.assertEqual(report["original_stage_pck_archives"],1)
        self.assertEqual(report["new_original_members_written"],3)
        target=self.stage/"pck_assets"/"bg"
        self.assertEqual((target/"0000_mesh.i3d").read_bytes(),self.data[0])
        self.assertEqual((target/"0002_anim.i3m").read_bytes(),self.data[2])
        self.assertEqual((target/"0003_texture.tm2").read_bytes(),self.data[3])
        self.assertFalse(list(target.glob("*.i3c")),
                         "Collision must be read once directly from the original PCK")
        self.assertEqual(report["counts"]["collision_remains_in_original_pck"],1)
        self.assertFalse(report["synthetic_assets_generated"])

    def test_reuse_only_identical_private_original_bytes(self):
        module.unpack(self.root)
        result=module.unpack(self.root)
        self.assertEqual(result["new_original_members_written"],0)
        self.assertEqual(result["counts"]["existing_exact_match"],3)
        output=self.stage/"pck_assets"/"bg"/"0000_mesh.i3d"
        output.write_bytes(b"owner data changed")
        with self.assertRaises(FileExistsError):
            module.unpack(self.root)

    def test_corrupt_original_pck_rejected_without_assets(self):
        data=bytearray(self.source.read_bytes())
        table=struct.unpack_from("<I",data,4)[0]
        struct.pack_into("<I",data,table+8,0xFFFFFFF0)
        self.source.write_bytes(data)
        with self.assertRaisesRegex(ValueError,"member bounds"):
            module.unpack(self.root)
        self.assertFalse((self.stage/"pck_assets").exists())

    def test_different_stage_is_not_unpacked(self):
        result=module.unpack(self.root,"seaside_a")
        self.assertEqual(result["original_stage_pck_archives"],0)
        self.assertFalse((self.stage/"pck_assets").exists())


if __name__=="__main__":
    unittest.main()
