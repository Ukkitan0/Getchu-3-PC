"""Synthetic-game regression for Saru3VfiTool native offline asset bridge.

No commercial assets and no PS2 runtime are used. Mock external binaries only;
tests validate exact CLI syntax, output safety, folder discovery and statuses.
"""
from __future__ import annotations

from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).parent))
from prepare_owned_assets import prepare, saru_command


class SaruBridgeTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.game = self.root / "Owned Test Game.iso"
        self.game.write_bytes(b"fake fixture (not copyrighted disc)")
        self.converter = self.root / "Saru3VfiTool.exe"
        self.converter.write_bytes(b"mock executable")
        self.probe = self.root / "Getchu3AssetProbe.exe"
        self.probe.write_bytes(b"mock executable")
        self.workspace = self.root / "private owned assets"
        self.commands = []

    def tearDown(self):
        self.temp.cleanup()

    def fake_run(self, command, **kwargs):
        self.assertTrue(kwargs["check"])
        self.commands.append(command)
        if "--copy-databin" in command:
            self.assertEqual(command[1], str(self.game.resolve()))
            Path(command[-1]).write_bytes(b"VFI\0game-free")
        elif "extract" in command:
            self.assertEqual(command[0], str(self.converter.resolve()))
            self.assertIn("--tim2", command)
            extracted = Path(command[-1])
            extracted.mkdir(parents=True, exist_ok=True)
            (extracted / "databin.manifest.json").write_text("{}")
            for rel, data in [
                ("STAGE/PACK_A.PCK.d/area.luc", b"\x1bLua\x50"),
                ("STAGE/WORLD_X.i3d", b"\x00"),
                ("GRAPHICS/obj.png", b"PNG"),
                ("STAGE/level.i3c", b"\x00"),
                ("PARAM/level.exdb.json", b"{}"),
            ]:
                target = extracted / rel
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(data)
        else:
            raise AssertionError(f"unexpected subprocess {command}")

    def test_full_native_iso_to_recursive_asset_inventory(self):
        result = prepare(self.game, self.workspace, self.converter, self.probe,
                         runner=self.fake_run)
        self.assertEqual(len(self.commands), 2)
        self.assertEqual(self.commands[0][2], "--copy-databin")
        self.assertEqual(result["asset_types"][".luc"], 1)
        self.assertEqual(result["asset_types"][".i3d"], 1)
        self.assertEqual(result["asset_types"][".png"], 1)
        self.assertEqual(result["asset_types"][".json"], 2)
        self.assertEqual(result["lua_bytecode_bytes"], 5)
        self.assertFalse(result["native_runtime_depends_on_saru"])
        self.assertFalse(result["requires_ps2_hardware_emulation"])
        self.assertTrue((self.workspace / "native_asset_inventory.json").exists())

    def test_reuse_extraction_does_not_run_external_tools(self):
        prepare(self.game, self.workspace, self.converter, self.probe,
                runner=self.fake_run)
        self.commands.clear()
        second = prepare(self.game, self.workspace, self.converter, self.probe,
                         runner=self.fake_run, reuse=True)
        self.assertEqual(self.commands, [])
        self.assertEqual(second["asset_types"][".luc"], 1)

    def test_nonempty_output_refuses_overwrite_without_reuse(self):
        self.workspace.mkdir(parents=True)
        (self.workspace / "keep.txt").write_text("do not overwrite")
        with self.assertRaisesRegex(FileExistsError, "already contains"):
            prepare(self.game, self.workspace, self.converter, self.probe,
                    runner=self.fake_run)
        self.assertEqual(self.commands, [])
        self.assertEqual((self.workspace / "keep.txt").read_text(), "do not overwrite")

    def test_data_bin_input_skips_unnecessary_iso_export(self):
        game = self.root / "DATA.BIN"
        game.write_bytes(b"VFI\0")
        def runner(command, **kwargs):
            self.assertNotIn("--copy-databin", command)
            self.assertEqual(command[-2], str(game.resolve()))
            output = Path(command[-1])
            output.mkdir(parents=True, exist_ok=True)
            (output / "databin.manifest.json").write_text("{}")
        result = prepare(game, self.workspace, self.converter, self.probe, runner=runner)
        self.assertEqual(result["asset_types"][".json"], 1)
        self.assertEqual(result["raw_databin"], str(game.resolve()))

    def test_missing_manifest_after_converter_is_failure(self):
        def runner(command, **kwargs):
            if "--copy-databin" in command:
                Path(command[-1]).write_bytes(b"VFI\0")
        with self.assertRaisesRegex(RuntimeError, "did not produce"):
            prepare(self.game, self.workspace, self.converter, self.probe,
                    runner=runner)

    def test_external_gpl_utility_is_never_runtime_dependency(self):
        library = self.root / "Saru3VfiTool.dll"
        library.write_bytes(b"synthetic .NET assembly")
        self.assertEqual(saru_command(library), ["dotnet", str(library)])
        badTool = self.root / "DATA.BIN"
        badTool.write_bytes(b"synthetic, not an executable")
        with self.assertRaisesRegex(ValueError, "exe or .dll"):
            saru_command(badTool)

    def test_keeps_tex_options_explicit(self):
        def runner(command, **kwargs):
            if "--copy-databin" in command:
                Path(command[-1]).write_bytes(b"VFI\0")
            else:
                self.assertNotIn("--tim2", command)
                out = Path(command[-1])
                out.mkdir(parents=True, exist_ok=True)
                (out / "databin.manifest.json").write_text("{}")
        prepare(self.game, self.workspace, self.converter, self.probe,
                tim2_png=False, runner=runner)


if __name__ == "__main__":
    unittest.main(verbosity=2)
