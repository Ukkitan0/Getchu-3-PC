#!/usr/bin/env python3
"""Verify original AE3 USA attachment constructors and vtables from owned ELF.

Reads the authentic executable locally. Only address/verification metadata is
produced, never original executable/game bytes or emulated hardware.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct

SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
VTABLES={
 0x6b1240:[0x56aeac,0x56aebc,0x56af0c,0x56af1c,0x56af14,0x56af24],
 0x6b1220:[0x56aedc,0x56aeec,0x56af2c,0x56af3c,0x56af34,0x56af44],
 0x6b1500:[0x572524,0x572578,0x572800,0x572810,0x572808,0x572840],
 0x6b1520:[0x572478,0x5724cc,0x5725d0,0x5725e0,0x5725d8,0x5727d8]
}
# Exact MIPS R5900 instructions, including 16-byte LQ/SQ copy operations.
WORDS={
 0x312780:0x0c106e00,0x312784:0x24040060,
 0x312788:0x7a0f0000,0x312790:0x7c4f0010,
 0x3127b0:0x7a0f0010,0x3127b8:0x7c4f0020,
 0x3127bc:0x7a0f0020,0x3127c0:0x7c4f0030,
 0x3127c4:0x7a0f0030,0x3127cc:0x7c4f0040,
 0x312794:0xac520050,0x31279c:0xac400054,0x3127a8:0xac4f0000,
 0x3127f0:0x0c106e00,0x3127f4:0x24040048,
 0x31280c:0x8daf0004,0x312808:0xac510010,
 0x312810:0xac400044,0x312818:0xac400014,
 0x312828:0xa040003c,0x312830:0xa040003d,0x312838:0xac400040,
 0x31286c:0x0c106e00,0x312870:0x24040040,
 0x312874:0xac500004,0x31287c:0xac510008,
 0x312884:0xac4f0000,0x312888:0xac400034,0x31288c:0xac400030,
 0x571e48:0xac850004,0x571e6c:0xac8f0000,
 0x571e70:0xac860018,0x571e74:0xac800020,
 0x571e78:0xac800010,0x571e7c:0xac800014,
 0x56af10:0x0000102d,0x56af18:0x0000102d,
 0x56af20:0x00000000,0x56af28:0x0000102d,
 0x56af30:0x0000102d,0x56af38:0x0000102d,
 0x56af40:0x00000000,
 0x5725d4:0x24020003,0x5725dc:0x00c0102d,
 0x572804:0x24020003,0x57280c:0x00c0102d,
 0x5725ec:0x11e00005,0x572818:0x8c8f0010,
 0x315c28:0xa1cf00a1,0x315c60:0xa1cf00a1,
 0x3161bc:0x30c600ff,0x3161c4:0x34af0001,
 0x3161cc:0x01e6280b,0x3161d4:0x34af0002,
 0x3161d8:0x30e700ff,0x3161e4:0x01e7280b,
 0x3161e8:0xae020004,0x316210:0xae000000
}
EDGES={0x312780:0x41b800,0x3127f0:0x41b800,
 0x31286c:0x41b800,0x315c2c:0x41a3b0,
 0x315c64:0x41a3b0,0x3161e0:0x316174}

class Elf:
 def __init__(self,path):
  self.data=path.read_bytes()
  if hashlib.sha256(self.data).hexdigest()!=SHA:
   raise ValueError("Expected original USA SCUS_975.01 SHA256")
  if self.data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",self.data,18)[0]!=8:
   raise ValueError("Not MIPS ELF32")
  phoff=struct.unpack_from("<I",self.data,28)[0]
  stride,count=struct.unpack_from("<HH",self.data,42)
  self.segments=[]
  for i in range(count):
   typ,off,va,_,length,_,flags,_=struct.unpack_from("<IIIIIIII",self.data,phoff+i*stride)
   if typ==1 and off+length<=len(self.data):
    self.segments.append((va,va+length,off))
  if not self.segments:raise ValueError("No original ELF LOAD segments")
 def word(self,va):
  for lo,hi,off in self.segments:
   if lo<=va<=hi-4:return struct.unpack_from("<I",self.data,off+va-lo)[0]
  raise ValueError("Unmapped original executable address "+hex(va))

def verify(path):
 elf=Elf(path)
 for va,items in VTABLES.items():
  if [elf.word(va+4*i) for i in range(len(items))]!=items:
   raise ValueError("Original source vtable mismatch "+hex(va))
 for va,w in WORDS.items():
  if elf.word(va)!=w:
   raise ValueError("Original R5900 instruction mismatch "+hex(va))
 for pc,dest in EDGES.items():
  word=elf.word(pc)
  original=((pc+4)&0xf0000000)|((word&0x3ffffff)<<2)
  if word>>26 not in (2,3) or original!=dest:
   raise ValueError("Original call/jump target mismatch "+hex(pc))
 return dict(format="ae3/original-monkey-attachment-subsystem/v1",
  elf_sha256=SHA,
  verified_vtables={hex(k):[hex(n) for n in v] for k,v in VTABLES.items()},
  verified_instruction_sites=len(WORDS),
  verified_branch_sites=len(EDGES),
  original_allocation_sizes={"0x312760":96,"0x3127d8":72,"0x312854":64},
  original_small_child_virtual5="0x56AF24 returns zero",
  still_unrecovered="0x56AF44 parameterized child virtual5, actor update/AI and full scene")

if __name__=="__main__":
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument("original_elf",type=Path)
 p.add_argument("--out",type=Path)
 a=p.parse_args()
 report=verify(a.original_elf)
 content=json.dumps(report,indent=2)+"\n"
 if a.out:a.out.write_text(content,encoding="utf-8")
 else:print(content,end="")
 print("PASS: authenticated original attachment vtables and instructions")
