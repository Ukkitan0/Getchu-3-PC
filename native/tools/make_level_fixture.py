#!/usr/bin/env python3
"""Build a game-free two-object level fixture with the AE3 PLC record layout.

The files contain original-free synthetic GLBs and a .plc whose offsets
mirror the user-supplied Exporter research. For native Windows CI only.
"""
from __future__ import annotations
import argparse
from pathlib import Path
import struct
from make_scene_fixture import build_glb


def build(root: Path) -> Path:
    root.mkdir(parents=True, exist_ok=True)
    glb = build_glb()
    (root / "wod_b_bg01.glb").write_bytes(glb)
    (root / "a_wod_c_closet_door.glb").write_bytes(glb)
    name = b"closet_door"
    plc = bytearray(0xc0 + 0x80)
    struct.pack_into("<I", plc, 0, 1)
    plc[0xc0:0xc0+len(name)] = name
    plc[0xe0:0xe0+len(name)] = name
    struct.pack_into("<3f", plc, 0xc0+0x40, 40.0, 2.0, 15.0)
    struct.pack_into("<f", plc, 0xc0+0x4c, 1.0)
    struct.pack_into("<4f", plc, 0xc0+0x60, 1.0,1.0,1.0,1.0)
    (root / "placeholder.plc").write_bytes(plc)
    return root


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("out", type=Path)
    args = p.parse_args()
    result = build(args.out)
    print("Generated native game-free level with background + placed prop:", result)


if __name__ == "__main__":
    main()
