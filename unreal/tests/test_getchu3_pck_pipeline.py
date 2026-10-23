"""Cross-module regression: original PCK -> unchanged real format asset inputs.

Uses only structural test fixtures. The production import path does not
substitute fake game objects; this regression verifies source-data flow.
"""
from __future__ import annotations

import importlib.util
from pathlib import Path
import struct
import sys
import tempfile
import unittest

REPO = Path(__file__).resolve().parents[2]
TOOLS = REPO / "native" / "tools"
EDITOR_PYTHON = REPO / "unreal" / "Plugins" / "Getchu3Importer" / "Content" / "Python"

if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))
if str(EDITOR_PYTHON) not in sys.path:
    sys.path.insert(0, str(EDITOR_PYTHON))

from unpack_original_stage_pck import unpack
from getchu3_media import _candidates
from getchu3_collision import I3C_MAGIC


class OriginalPckPipelineTests(unittest.TestCase):
    def test_member_magic_remains_original_across_archive_import_boundary(self):
        with tempfile.TemporaryDirectory() as work:
            source = Path(work) / "original" / "vfi"
            stage = source / "debug" / "us" / "stage" / "woods_c"
            stage.mkdir(parents=True)
            names = [b"char", b"collision", b"face", b"animation"]
            originals = [
                b"I3D_BIN\x00" + b"m" * 48,
                b"I3D_I3C\x00" + b"c" * 48,
                b"TIM2" + b"t" * 48,
                b"I3D_I3M\x00" + b"a" * 48,
            ]
            pool = b""
            ids = []
            for name in names:
                ids.append(len(pool))
                pool += name + b"\x00"
            table = 16 + len(pool) + 8
            toc = bytearray(len(names) * 16)
            at = table + len(toc)
            for i, data in enumerate(originals):
                struct.pack_into("<IIII", toc, i * 16, ids[i], 0, at, len(data))
                at += len(data)
            header = bytearray(16)
            header[:4] = b"PCK\x00"
            struct.pack_into("<II", header, 4, table, len(names))
            (stage / "objects.pck").write_bytes(
                bytes(header + pool + b"\0" * 8 + toc + b"".join(originals)))

            extraction = unpack(source, include_collision=True)
            self.assertEqual(extraction["new_original_members_written"], 4)
            self.assertEqual(extraction["counts"]["collision_exported_for_unreal"], 1)
            members = stage / "pck_assets" / "objects"
            # The I3D_BIN model and I3D_I3C collision have the same file
            # extension, but source magic routes each to the right adapter.
            self.assertEqual((members / "0000_char.i3d").read_bytes(), originals[0])
            self.assertEqual((members / "0001_collision.i3d").read_bytes(), originals[1])
            self.assertEqual((members / "0003_animation.i3m").read_bytes(), originals[3])
            self.assertEqual(
                [f.relative_to(source).as_posix()
                 for f in _candidates(source, ".tm2", b"TIM2")],
                ["debug/us/stage/woods_c/pck_assets/objects/0002_face.tm2"])

            # This is a SOURCE-CONTRACT test. Actual ae3-sdk conversion and
            # Unreal static-mesh collision are independently verified later.
            i3ds = sorted(members.glob("*.i3d"))
            with i3ds[0].open("rb") as stream:
                first = stream.read(8)
            with i3ds[1].open("rb") as stream:
                second = stream.read(8)
            self.assertEqual({first, second}, {b"I3D_BIN\x00", I3C_MAGIC})

    def test_native_default_does_not_extract_collision(self):
        with tempfile.TemporaryDirectory() as work:
            source = Path(work) / "vfi"
            (source / "debug/us/stage/a").mkdir(parents=True)
            result = unpack(source)
            self.assertEqual(result["original_stage_pck_archives"], 0)
            self.assertFalse(result["owner_private_i3c_extraction_requested"])


if __name__ == "__main__":
    unittest.main()
