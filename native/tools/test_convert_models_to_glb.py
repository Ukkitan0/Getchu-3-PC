"""Game-free test: independently converted AE3 models from Saru -> MIT ae3-sdk."""
from __future__ import annotations
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0,str(Path(__file__).parent))
from convert_models_to_glb import export_models


class ConvertModels(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory()
        self.base=Path(self.tmp.name)
        self.extracted=self.base/"saru_output"
        self.extracted.mkdir()
        self.output=self.base/"glb_output"
        self.tool=self.base/"ae3.exe"
        self.tool.write_bytes(b"synthetic test executable only")
        for relative,bytes_ in (
           ("stage/A/hero.i3d",b"I3D_BIN\x00"+b"\x00"*32),
           ("stage/A/collision.i3d",b"I3D_I3C\x00"+b"\x00"*32),
           ("stage/B/npc.i3d",b"I3D_BIN\x00"+b"\x00"*32),
           ("stage/B/animation.i3m",b"I3D_I3M\x00"+b"\x00"*32),
           ("texture/sample.tm2",b"TIM2")
        ):
            p=self.extracted/relative
            p.parent.mkdir(parents=True,exist_ok=True)
            p.write_bytes(bytes_)
        self.calls=[]

    def tearDown(self):
        self.tmp.cleanup()

    def fake_run(self,cmd,**kwargs):
        self.assertEqual(cmd[0],str(self.tool))
        self.assertEqual(cmd[1],"gltf")
        self.assertEqual(cmd[3],"--out")
        self.assertEqual(cmd[5],"--textures")
        self.calls.append(cmd)
        path=Path(cmd[4])/(Path(cmd[2]).stem+".glb")
        path.write_bytes(b"glTF test fixture")
        return subprocess.CompletedProcess(cmd,0,b"",b"")

    def test_two_models_not_animation_or_collision(self):
        r=export_models(self.extracted,self.output,str(self.tool),runner=self.fake_run)
        self.assertEqual(r["original_i3d_bin_files"],2)
        self.assertEqual(r["all_i3d_extension_files"],3)
        self.assertEqual(r["results"]["converted"],2)
        self.assertEqual(len(self.calls),2)
        self.assertTrue((self.output/"stage/A/hero.glb").exists())
        self.assertTrue((self.output/"stage/B/npc.glb").exists())
        self.assertFalse((self.output/"stage/A/collision.glb").exists())
        self.assertFalse(r["ps2_hardware_emulation"])
        self.assertFalse(r["native_runtime_ae3_sdk_dependency"])

    def test_reuse_existing_glb_does_not_overwrite(self):
        export_models(self.extracted,self.output,str(self.tool),runner=self.fake_run)
        self.calls.clear()
        r=export_models(self.extracted,self.output,str(self.tool),runner=self.fake_run)
        self.assertEqual(r["results"]["already_converted"],2)
        self.assertEqual(self.calls,[])

    def test_failed_conversion_recorded_without_pretend_success(self):
        def failing(*args,**kwargs):
            return subprocess.CompletedProcess(args[0],1,b"",b"bad glb")
        r=export_models(self.extracted,self.output,str(self.tool),runner=failing)
        self.assertEqual(r["results"]["failed"],2)
        self.assertEqual(len(r["failed_sources"]),2)

    def test_separate_private_output_required(self):
        with self.assertRaisesRegex(ValueError,"separate"):
            export_models(self.extracted,self.extracted/"nested",str(self.tool))

    def test_missing_cli_reports_installation_requirement(self):
        with self.assertRaises(FileNotFoundError):
            export_models(self.extracted,self.output,str(self.base/"nonexistent.exe"))

    def test_named_original_i3m_animation_request_uses_ae3_sdk_gates(self):
        report=export_models(self.extracted,self.output,str(self.tool),
                             animations=True,runner=self.fake_run)
        self.assertTrue(report["requested_original_i3m_animation_pairing"])
        self.assertTrue(all(command[-1]=="--anims" for command in self.calls))
        self.assertEqual(len(self.calls),2)

    def test_animate_rebuilds_existing_static_glbs_in_place(self):
        # Critical for Windows users who previously generated static
        # GLBs: enabling -Animate must actually create skeletal channels.
        export_models(self.extracted,self.output,str(self.tool),runner=self.fake_run)
        self.calls.clear()
        report=export_models(self.extracted,self.output,str(self.tool),
                             animations=True,runner=self.fake_run)
        self.assertEqual(len(self.calls),2)
        self.assertEqual(report["results"]["upgraded_to_original_animation"],2)
        self.assertTrue(all(x[-1]=="--anims" for x in self.calls))

    def test_too_many_models_refused(self):
        with self.assertRaisesRegex(ValueError,"many"):
            export_models(self.extracted,self.output,str(self.tool),
                          max_models=1,runner=self.fake_run)


if __name__=="__main__":
    unittest.main()
