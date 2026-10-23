"""Offline contract regressions for genuine media conversion. All input bytes are synthetic."""
from __future__ import annotations

from pathlib import Path
import importlib.util
import sys
import subprocess
import tempfile
import unittest

SOURCE = Path(__file__).resolve().parents[1] / "Plugins/Getchu3Importer/Content/Python/getchu3_media.py"
SPEC = importlib.util.spec_from_file_location("getchu3_media", SOURCE)
media = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = media
SPEC.loader.exec_module(media)


class MediaTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.vfi = self.root / "retail"
        self.vfi.mkdir()
        self.out = self.root / "unreal_media"
        self.ae3 = self.root / "ae3"
        self.ae3.write_text("offline fake executable; never invoked", encoding="utf-8")
        self.calls = []

    def original(self, subpath: str, payload: bytes) -> Path:
        path = self.vfi / subpath
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(payload)
        return path

    def conversion(self, command, **kwargs):
        self.calls.append(command)
        if command[1] == "tm2":
            name = Path(command[2]).stem
            folder = Path(command[command.index("--out") + 1])
            folder.mkdir(parents=True, exist_ok=True)
            (folder / (name + ".png")).write_bytes(b"\x89PNG\r\n\x1a\n" + b"x" * 60)
        elif command[1] == "exst":
            dest = Path(command[command.index("-o") + 1])
            dest.parent.mkdir(parents=True, exist_ok=True)
            dest.write_bytes(b"RIFF" + b"\x00" * 44)
        return subprocess.CompletedProcess(command, 0, "", "")

    def test_identical_basename_from_two_stages_is_never_flattened(self):
        self.original("stage/a/a_face.tm2", b"TIM2" + b"\x00" * 32)
        self.original("stage/b/a_face.tm2", b"TIM2" + b"\x00" * 32)
        result = media.convert_textures(self.vfi, self.out, str(self.ae3),
                                        runner=self.conversion)
        self.assertEqual(result["source_files"], 2)
        self.assertEqual(result["converted_png_files"], 2)
        self.assertEqual(len(set(result["files"])), 2)
        self.assertTrue((self.out / "stage/a/a_face.png").is_file())
        self.assertTrue((self.out / "stage/b/a_face.png").is_file())

    def test_original_pck_texture_alias_is_pack_local_and_keeps_indexed_source(self):
        self.original("stage/woods_c/pck_assets/bg/0003_fur.tm2", b"TIM2" + b"a" * 32)
        self.original("stage/toyhouse_c/pck_assets/bg/0002_fur.tm2", b"TIM2" + b"b" * 32)
        alias_root = self.root / "material_aliases"
        result = media.convert_textures(self.vfi, self.out, str(self.ae3),
                                        material_alias_root=alias_root,
                                        runner=self.conversion)
        self.assertEqual(result["converted_png_files"], 2)
        self.assertEqual(len(result["pack_local_material_aliases"]), 2)
        self.assertTrue((self.out / "stage/woods_c/pck_assets/bg/0003_fur.png").is_file())
        self.assertTrue((self.out / "stage/toyhouse_c/pck_assets/bg/0002_fur.png").is_file())
        self.assertTrue((alias_root / "stage/woods_c/pck_assets/bg/fur.png").is_file())
        self.assertTrue((alias_root / "stage/toyhouse_c/pck_assets/bg/fur.png").is_file())
        self.assertFalse((alias_root / "fur.png").exists())

    def test_ambiguous_original_material_names_in_one_pack_are_blocked(self):
        self.original("stage/a/pck_assets/bg/0001_fur.tm2", b"TIM2" + b"1" * 32)
        self.original("stage/a/pck_assets/bg/0002_fur.tm2", b"TIM2" + b"2" * 32)
        def per_source_pixels(command, **kwargs):
            folder = Path(command[command.index("--out") + 1])
            folder.mkdir(parents=True, exist_ok=True)
            stem = Path(command[2]).stem
            (folder / (stem + ".png")).write_bytes(b"PNG" + stem.encode("ascii"))
            return subprocess.CompletedProcess(command, 0, "", "")
        with self.assertRaisesRegex(RuntimeError, "Ambiguous original texture material"):
            media.convert_textures(self.vfi, self.out, str(self.ae3),
                                   material_alias_root=self.root / "aliases",
                                   runner=per_source_pixels)

    def test_material_alias_destination_must_not_enter_original_or_png_source(self):
        self.original("stage/a/pck_assets/bg/0000_fur.tm2", b"TIM2" + b"f" * 32)
        for bad in [self.vfi / "material_names", self.out / "material_names"]:
            with self.subTest(destination=bad):
                with self.assertRaisesRegex(ValueError, "separate"):
                    media.convert_textures(self.vfi, self.out, str(self.ae3),
                                           material_alias_root=bad, runner=self.conversion)

    def test_invalid_retail_magic_rejected_before_conversion(self):
        self.original("stage/a/bad.tm2", b"XXXX123456")
        with self.assertRaisesRegex(ValueError, "magic"):
            media.convert_textures(self.vfi, self.out, str(self.ae3), runner=self.conversion)
        self.assertEqual(self.calls, [])

    def test_missing_converter_fails_instead_of_faking_texture(self):
        self.original("stage/a/good.tm2", b"TIM2" + b"\x00" * 30)
        with self.assertRaises(FileNotFoundError):
            media.convert_textures(self.vfi, self.out, str(self.root / "absent"),
                                   runner=self.conversion)

    def test_failed_cli_cannot_mark_conversion_complete(self):
        self.original("stage/a/good.tm2", b"TIM2" + b"\x00" * 30)
        def fails(command, **kwargs):
            return subprocess.CompletedProcess(command, 1, "", "decode error")
        with self.assertRaisesRegex(RuntimeError, "failed"):
            media.convert_textures(self.vfi, self.out, str(self.ae3), runner=fails)

    def test_original_exst_stream_decodes_without_claiming_sound_engine(self):
        self.original("debug/us/stage/a/music.x", b"EXST" + b"\x00" * 200)
        self.original("debug/us/stage/a/other.x", b"OTHER" + b"\x00" * 200)
        result = media.convert_stream_audio(self.vfi, self.out, str(self.ae3),
                                            runner=self.conversion)
        self.assertEqual(result["source_exst"], 1)
        self.assertEqual(result["wav_files"], ["debug/us/stage/a/music.wav"])
        self.assertEqual(result["other_x"], ["debug/us/stage/a/other.x"])
        self.assertFalse(result["gameplay_ready"])
        self.assertTrue((self.out / result["wav_files"][0]).is_file())

    def test_no_exst_is_valid_without_converter(self):
        result = media.convert_stream_audio(self.vfi, self.out, "not-installed",
                                            runner=self.conversion)
        self.assertEqual(result["wav_files"], [])
        self.assertEqual(self.calls, [])

    def test_output_within_original_vfi_rejected(self):
        self.original("a.tm2", b"TIM2" + b"\x00" * 100)
        with self.assertRaisesRegex(ValueError, "separate"):
            media.convert_textures(self.vfi, self.vfi / "png", str(self.ae3),
                                   runner=self.conversion)


if __name__ == "__main__":
    unittest.main()
