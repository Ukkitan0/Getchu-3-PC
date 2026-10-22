"""No-game-file tests for original I3C static collision import. Geometries synthetic."""
from __future__ import annotations
from pathlib import Path
import importlib.util
import sys
import subprocess
import tempfile
import unittest

FILE = Path(__file__).resolve().parents[1] / "Plugins/Getchu3Importer/Content/Python/getchu3_collision.py"
spec = importlib.util.spec_from_file_location("getchu3_collision", FILE)
collision = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = collision
spec.loader.exec_module(collision)


class CollisionTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.source = self.root / "source"
        self.source.mkdir()
        self.dest = self.root / "dest"
        self.ae3 = self.root / "ae3"
        self.ae3.touch()
        self.commands = []

    def source_file(self, name, data):
        path = self.source / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
        return path

    def fake_ae3(self, command, **kwargs):
        self.commands.append(command)
        self.assertEqual(command[1], "i3c")
        self.assertEqual(command[command.index("--format") + 1], "obj")
        out = Path(command[command.index("--out") + 1]) / (Path(command[2]).stem + ".obj")
        out.write_text("g ROOTNODE\nv 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n", encoding="utf-8")
        return subprocess.CompletedProcess(command, 0, "", "")

    def test_collisions_derived_from_original_i3c_not_models(self):
        self.source_file("stage/wood/a.i3d", b"I3D_I3C\x00" + b"\0" * 80)
        self.source_file("stage/wood/character.i3d", b"I3D_BIN\x00" + b"\0" * 80)
        result = collision.convert_collision(self.source, self.dest, str(self.ae3), runner=self.fake_ae3)
        self.assertEqual(result["original_i3c_files"], 1)
        self.assertEqual(result["collision_objects"][0]["vertices"], 3)
        self.assertEqual(result["collision_objects"][0]["faces"], 1)
        self.assertFalse(result["native_collision_behavior_verified"])
        self.assertTrue((self.dest / "stage/wood/a.obj").is_file())

    def test_same_basename_in_other_stage_kept_separate(self):
        for name in ["stage/a/wall.i3d", "stage/b/wall.i3d"]:
            self.source_file(name, b"I3D_I3C\x00" + b"\0" * 80)
        result = collision.convert_collision(self.source, self.dest, str(self.ae3), runner=self.fake_ae3)
        self.assertEqual({o["exported"] for o in result["collision_objects"]},
                         {"stage/a/wall.obj", "stage/b/wall.obj"})

    def test_missing_source_collision_is_rejected(self):
        self.source_file("some_model.i3d", b"I3D_BIN\x00" + b"\0" * 80)
        with self.assertRaisesRegex(RuntimeError, "no I3D_I3C"):
            collision.convert_collision(self.source, self.dest, str(self.ae3), runner=self.fake_ae3)

    def test_source_output_separation(self):
        self.source_file("stage/a.i3d", b"I3D_I3C\x00" + b"\0" * 80)
        with self.assertRaisesRegex(ValueError, "separate"):
            collision.convert_collision(self.source, self.source / "out", str(self.ae3),
                                        runner=self.fake_ae3)

    def test_blank_geometry_fails(self):
        self.source_file("stage/a.i3d", b"I3D_I3C\x00" + b"\0" * 80)
        def bad_output(command, **kwargs):
            out = Path(command[command.index("--out") + 1]) / (Path(command[2]).stem + ".obj")
            out.write_text("# no triangles\n", encoding="utf-8")
            return subprocess.CompletedProcess(command, 0, "", "")
        with self.assertRaisesRegex(RuntimeError, "no triangles"):
            collision.convert_collision(self.source, self.dest, str(self.ae3), runner=bad_output)

    def test_missing_converter_is_not_silent(self):
        self.source_file("stage/a.i3d", b"I3D_I3C\x00" + b"\0" * 80)
        with self.assertRaises(FileNotFoundError):
            collision.convert_collision(self.source, self.dest, "ae3-nonexistent",
                                        runner=self.fake_ae3)


if __name__ == "__main__":
    unittest.main()
