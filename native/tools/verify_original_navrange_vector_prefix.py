#!/usr/bin/env python3
"""Verify exact original vector prefix and the inert retail delay loop."""
import argparse
from pathlib import Path
from decode_original_navrange_vu import elf_words
ANCHORS={
0x359e98:0x48a50800,0x359ea0:0x48ae1000,0x359ea4:0x4be10868,
0x359ea8:0x4b4200ac,0x359eac:0x48ad1800,0x359eb0:0x4be00183,
0x359eb4:0x4ac300ec,0x359eb8:0x4be111bd,0x359ebc:0x48af2000,
0x359ec0:0x4b0030bf,0x359ec4:0x4be1194e,0x359ec8:0x4b84012c,
0x441f40:0x240e0003,0x441f44:0x25ceffff,0x441f48:0x240fffff,
0x441f4c:0,0x441f50:0,0x441f54:0,0x441f58:0,0x441f5c:0,
0x441f60:0x55cffffa,0x441f64:0x25ceffff,
0x441f68:0x03e00008,0x441f6c:0,
0x21ef94:0x0c1107d0,0x21efd0:0x0c1107d0}
def verify(path):
 read=elf_words(path)
 for va,word in ANCHORS.items():
  if read(va)!=word:raise ValueError("Unexpected retail word at "+hex(va))
 imm=read(0x441f60)&0xffff
 if imm&0x8000:imm-=65536
 assert 0x441f60+4+4*imm==0x441f4c
 for source in (0x21ef94,0x21efd0):
  w=read(source)
  assert w>>26==3 and (((source+4)&0xf0000000)|((w&0x03ffffff)<<2))==0x441f40
 print("PASS:",len(ANCHORS),"original opcode anchors, branch target, two JAL edges")
 print("Remaining: source MMI packing and complete native 0x21EF3C")
if __name__=="__main__":
 p=argparse.ArgumentParser();p.add_argument("elf",type=Path);verify(p.parse_args().elf)
