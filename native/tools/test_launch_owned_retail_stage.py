"""Original retail stage launch gate: never substitutes synthetic game resources."""
from __future__ import annotations
import importlib.util
from pathlib import Path
import tempfile
import unittest

spec=importlib.util.spec_from_file_location(
    "actual_retail_stage_launcher",Path(__file__).with_name("launch_owned_retail_stage.py"))
module=importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)

class OriginalRetailLaunchTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root=Path(self.tmp.name)
        self.iso=self.root/"SCUS_97501_original.iso"
        self.iso.write_bytes(b"private-owner-owned-original-placeholder")
        self.exe=self.root/"Getchu3Native.exe"
        self.exe.write_bytes(b"native-windows-image-placeholder")
        self.extracted=self.root/"extracted"
        self.glb=self.root/"glb"
        self.stage_src=self.extracted/"debug"/"us"/"stage"/"seaside_a"
        self.stage_models=self.glb/"debug"/"us"/"stage"/"seaside_a"
        self.stage_src.mkdir(parents=True)
        self.stage_models.mkdir(parents=True)
        (self.stage_src/"placeholder.plc").write_bytes(b"genuine-file-signature-not-evaluated-here")
        (self.stage_src/"collision.i3d").write_bytes(b"I3D_I3C\x00"+b"\x00"*28)
        (self.stage_models/"seaside_a_world.glb").write_bytes(b"glTF"+b"\x00"*32)
        self.calls=[]
    def run_real(self):
        class Proc:
            returncode=0
        def runner(args,check):
            self.calls.append(args)
            return Proc()
        return module.launch(iso=self.iso,stage="seaside_a",
                             extracted=self.extracted,glb=self.glb,exe=self.exe,
                             runner=runner)
    def test_real_stage_paths_are_passed_without_copy_or_substitution(self):
        r=self.run_real()
        self.assertEqual(len(self.calls),1)
        args=self.calls[0]
        self.assertEqual(args[1],"--retail")
        self.assertEqual(args[3],"seaside_a")
        self.assertTrue(args[4].endswith("stage"+str(Path("/"))+"seaside_a") or
                        Path(args[4]).name=="seaside_a")
        self.assertEqual(args[5],str((self.stage_src/"placeholder.plc").resolve()))
        self.assertEqual(Path(args[6]),self.stage_src.resolve())
        self.assertEqual(r["gameplay_verified"],False)
        self.assertEqual(r["missing_original_requirements"],[])
        self.assertEqual(r["native_glb_model_count"],1)
    def test_missing_original_collision_blocks_entire_game_launch(self):
        (self.stage_src/"collision.i3d").unlink()
        with self.assertRaisesRegex(RuntimeError,"No I3D_I3C"):
            self.run_real()
        self.assertEqual(self.calls,[])
    def test_fake_or_other_stage_collision_cannot_be_used(self):
        (self.stage_src/"collision.i3d").write_bytes(b"I3D_BIN\x00"+b"\x00"*28)
        foreign=self.extracted/"debug"/"us"/"stage"/"woods_c"
        foreign.mkdir(parents=True)
        (foreign/"collision.i3c").write_bytes(b"I3D_I3C\x00")
        with self.assertRaisesRegex(RuntimeError,"No I3D_I3C"):
            self.run_real()
        self.assertEqual(self.calls,[])
    def test_missing_original_plc_blocks_entire_game_launch(self):
        (self.stage_src/"placeholder.plc").unlink()
        with self.assertRaisesRegex(RuntimeError,"placeholder.plc"):
            self.run_real()
        self.assertEqual(self.calls,[])
    def test_other_stage_glb_not_misidentified_as_target(self):
        (self.stage_models/"seaside_a_world.glb").unlink()
        foreign=self.glb/"debug"/"us"/"stage"/"woods_c"
        foreign.mkdir(parents=True)
        (foreign/"woods_c_world.glb").write_bytes(b"glTF"+b"\x00"*40)
        with self.assertRaisesRegex(RuntimeError,"No converted original I3D GLB"):
            self.run_real()
        self.assertEqual(self.calls,[])
    def test_dry_run_must_not_launch_and_report_cannot_claim_playability(self):
        r=module.launch(iso=self.iso,stage="seaside_a",
                        extracted=self.extracted,glb=self.glb,exe=self.exe,
                        dry_run=True)
        self.assertEqual(r["status"],"original_sources_located_native_game_incomplete")
        self.assertFalse(r["gameplay_verified"])
        self.assertEqual(self.calls,[])
    def test_reports_blocker_before_attempting_any_launch(self):
        (self.stage_src/"collision.i3d").unlink()
        dest=self.root/"private_stage_report.json"
        with self.assertRaises(RuntimeError):
            module.launch(iso=self.iso,stage="seaside_a",
                          extracted=self.extracted,glb=self.glb,exe=self.exe,
                          report_path=dest)
        self.assertIn("No I3D_I3C",dest.read_text())

if __name__=="__main__":
    unittest.main()
