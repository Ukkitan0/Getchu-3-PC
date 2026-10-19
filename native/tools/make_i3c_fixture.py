#!/usr/bin/env python3
"""Generate a synthetic but structurally genuine AE3 I3D_I3C cube collision file.

For testing the native C++ original-format collision loader WITHOUT
distributing the game. This is not a recovered Sony collision asset.
"""
from __future__ import annotations
import argparse
from pathlib import Path
import struct

VERTICES=[
    (-.5,-.5,-.5),(.5,-.5,-.5),(.5,-.5,.5),(-.5,-.5,.5),
    (-.5,.5,-.5),(.5,.5,-.5),(.5,.5,.5),(-.5,.5,.5)
]
TRIS=[
    (0,2,1),(0,3,2),(4,5,6),(4,6,7),
    (0,1,5),(0,5,4),(1,2,6),(1,6,5),
    (2,3,7),(2,7,6),(3,0,4),(3,4,7)
]

def make_i3c():
    b=bytearray(0x300)
    b[:8]=b"I3D_I3C\x00"
    struct.pack_into("<II",b,8,0x00030000,len(b))
    b[0x10]=1
    struct.pack_into("<HHII",b,0x14,1,1,0x40,0x50)
    struct.pack_into("<IHHI",b,0x40,0x60,1,0,0x80)
    struct.pack_into("<I",b,0x50,0x70)
    b[0x60:0x60+9]=b"ROOTNODE\x00"
    b[0x70:0x70+10]=b"coll_wall\x00"
    struct.pack_into("<4f",b,0x80,0,0,0,1)
    struct.pack_into("<4f",b,0x90,.5,.5,.5,0)
    struct.pack_into("<II",b,0x80+0x28,0x180,0x100)
    for i,v in enumerate(VERTICES):
        struct.pack_into("<4f",b,0x100+i*16,*v,0)
    struct.pack_into("<H",b,0x180,len(TRIS))
    struct.pack_into("<I",b,0x188,0x190)
    for i,indices in enumerate(TRIS):
        struct.pack_into("<HI",b,0x190+i*12,0x8001,0)
        struct.pack_into("<I",b,0x190+i*12+8,0x220+i*8)
        struct.pack_into("<4H",b,0x220+i*8,*indices,i)
    return bytes(b)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("destination",type=Path)
    args=p.parse_args()
    args.destination.parent.mkdir(parents=True,exist_ok=True)
    data=make_i3c()
    args.destination.write_bytes(data)
    print("Generated game-free original-format I3C collision cube:",
          args.destination,len(data),"bytes, 12 original-format triangles")

if __name__=="__main__":main()
