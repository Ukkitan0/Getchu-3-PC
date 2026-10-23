"""Validate that original I3D models use ONLY source-pack-local TIM2 aliases.

This is an argument-routing/source identity test, not an animation or Unreal
rendering test. No game assets are used.
"""
from __future__ import annotations

import importlib.util
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

REPO = Path(__file__).resolve().parents[2]
FILE = REPO / "native" / "tools" / "convert_models_to_glb.py"
spec = importlib.util.spec_from_file_location("getchu3_original_model_converter", FILE)
converter = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = converter
spec.loader.exec_module(converter)


class PackMaterialRoutingTests(unittest.TestCase):
    def setUp(self):
        folder = tempfile.TemporaryDirectory()
        self.addCleanup(folder.cleanup)
        self.root = Path(folder.name)
        self.vfi = self.root / "source"
        self.vfi.mkdir()
        self.models = self.root / "converted" / "models"
        self.original_png = self.root / "converted" / "textures"
        self.aliases = self.root / "converted" / "material_aliases"
        self.ae3 = self.root / "ae3"
        self.ae3.write_bytes(b"fixture executable path")
        self.calls = []

    def original_model(self, subpath, *, magic=b"I3D_BIN\x00"):
        f = self.vfi / subpath
        f.parent.mkdir(parents=True, exist_ok=True)
        f.write_bytes(magic + b"\0" * 48)
        return f

    def make_original_alias(self, subpath):
        f = self.aliases / subpath
        f.parent.mkdir(parents=True, exist_ok=True)
        f.write_bytes(b"fixture PNG")
        return f

    def fake_sdk(self, command, **kwargs):
        self.calls.append(command)
        expected = Path(command[command.index("--out") + 1]) / (
            Path(command[2]).stem + ".glb")
        expected.parent.mkdir(parents=True, exist_ok=True)
        expected.write_bytes(b"fixture GLB")
        return subprocess.CompletedProcess(command, 0, b"", b"")

    def test_different_pcks_never_share_same_material_name(self):
        a = self.original_model("debug/us/stage/a/pck_assets/bg/0000_ape.i3d")
        b = self.original_model("debug/us/stage/b/pck_assets/bg/0000_ape.i3d")
        self.make_original_alias("debug/us/stage/a/pck_assets/bg/fur.png")
        self.make_original_alias("debug/us/stage/b/pck_assets/bg/fur.png")
        result = converter.export_models(
            self.vfi, self.models, str(self.ae3), textures=self.original_png,
            pack_texture_aliases=self.aliases, runner=self.fake_sdk)
        self.assertEqual(result["original_i3d_bin_files"], 2)
        self.assertEqual(result["results"]["converted"], 2)
        selected = {Path(c[2]): Path(c[c.index("--textures") + 1])
                    for c in self.calls}
        self.assertEqual(selected[a], self.aliases / a.relative_to(self.vfi).parent)
        self.assertEqual(selected[b], self.aliases / b.relative_to(self.vfi).parent)

    def test_collisions_are_not_imported_as_skeletal_models(self):
        self.original_model("debug/us/stage/a/pck_assets/bg/0000_collision.i3d",
                            magic=b"I3D_I3C\x00")
        model = self.original_model("debug/us/stage/a/pck_assets/bg/0001_ape.i3d")
        result = converter.export_models(
            self.vfi, self.models, str(self.ae3),
            pack_texture_aliases=self.aliases, runner=self.fake_sdk)
        self.assertEqual(result["original_i3d_bin_files"], 1)
        self.assertEqual(len(self.calls), 1)
        self.assertEqual(Path(self.calls[0][2]), model)
        self.assertEqual(result["results"]["models_without_verified_local_materials"], 1)

    def test_direct_vfi_model_uses_original_texture_root(self):
        model = self.original_model("debug/us/shared/ape.i3d")
        result = converter.export_models(
            self.vfi, self.models, str(self.ae3), textures=self.original_png,
            pack_texture_aliases=self.aliases, runner=self.fake_sdk)
        self.assertEqual(result["results"]["converted"], 1)
        self.assertEqual(
            self.calls[0][self.calls[0].index("--textures") + 1],
            str(self.original_png))
        self.assertEqual(Path(self.calls[0][2]), model)


if __name__ == "__main__":
    unittest.main()
