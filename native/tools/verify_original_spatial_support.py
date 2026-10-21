#!/usr/bin/env python3
"""Audit original AE3 USA spatial support without publishing proprietary bytes.
Full instruction-region SHA-256 checks plus exact opcode and branch anchors.
"""
import argparse,hashlib,json,struct
from pathlib import Path
PIN="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
REGIONS={
 (0x4e3c14,0x4e3d10):"10209a11c38481b3d27a9b68da992cfa79aa4d983b4853c1dac6d7f2c96d4031",
 (0x21b468,0x21b4e0):"ed9b248a9cdcfd4f8b30859b3c6bacb6e0d32e2313df293e017b975cbeb9fefd",
 (0x21b788,0x21b854):"c434176b035c929b0b12062267efb4b069e5fc796a3a715da1d9375a763c7d90",
 (0x21b384,0x21b3f8):"a4d99963163930421f7910b85e88be51dd6c9a5dd0727a0e296742e37e9367c1",
 (0x21b14c,0x21b178):"71ddd00c85d4b74aeacd98b5161fcf1c7319b261a253d957effd6abb82053391",
 (0x21afb0,0x21afb8):"6d64edf91449c1b17746c1ef18afa2eb25c70bdf1322ab3df5a2630993b7e2f1",
 (0x21b178,0x21b194):"f451cc60bb1ce68ab4813b06891ca5558e9f334dbdea41ff3b2d0ad21d1f353e",
 (0x21b194,0x21b240):"398a5def855a603e809038f0c8cf342b97134c37d63b86957385c3c49448ed1e"}
ANCHORS={
 0x4e3c14:0x8c8b0004,0x4e3c74:0x46030843,0x4e3c78:0x8c820008,
 0x4e3ca4:0x46040003,0x4e3cac:0x460008a4,0x4e3cc8:0x46000064,
 0x4e3cfc:0x95ef0050,0x21b478:0x95cf0016,0x21b4a4:0x95cf0016,
 0x21b4c8:0x46010083,0x21b794:0x956e0016,0x21b80c:0x460c0001,
 0x21b81c:0x46000834,0x21b820:0x45020001,0x21b82c:0x15e00007,
 0x21b848:0x24020001,0x21b3a8:0x8e300000,0x21b3c8:0x2652ffff,
 0x21b3cc:0x0643fff7,0x21b154:0x8c8f0004,0x21b158:0x11e00004,
 0x21afb0:0x03e00008,0x21b17c:0x8c820000,0x21b180:0x8c8f0004,
 0x21b1cc:0x14400008,0x21b234:0x46000004,0x21b23c:0x4600a500,
 0x6e3e2c:0x3f800000}
EDGES={0x21b3b8:0x21b14c,0x21b3c0:0x41a3b0,0x21b3d4:0x21afb0,
 0x21b164:0x41a3d8,0x21b1b8:0x21b178,0x21b1f0:0x21b180,
 0x21b200:0x21b180,0x21d8cc:0x4e3c14}
class ELF:
 def __init__(self,path):
  self.data=path.read_bytes()
  if hashlib.sha256(self.data).hexdigest()!=PIN:
   raise ValueError("Expected exact original USA SCUS_975.01")
  if self.data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",self.data,18)[0]!=8:
   raise ValueError("Expected MIPS little-endian ELF32")
  ph=struct.unpack_from("<I",self.data,28)[0]
  stride,count=struct.unpack_from("<HH",self.data,42)
  self.segments=[]
  for i in range(count):
   kind,off,va,_,n,_,_,_=struct.unpack_from("<8I",self.data,ph+i*stride)
   if kind==1 and off+n<=len(self.data):self.segments.append((va,va+n,off))
  if not self.segments:raise ValueError("No original ELF loadable sections")
 def read(self,va,n):
  for lo,hi,off in self.segments:
   if lo<=va and va+n<=hi:return self.data[off+va-lo:off+va-lo+n]
  raise ValueError("Unmapped original address "+hex(va))
 def word(self,va):return struct.unpack("<I",self.read(va,4))[0]
def verify(path):
 elf=ELF(path)
 for (start,end),digest in REGIONS.items():
  if hashlib.sha256(elf.read(start,end-start)).hexdigest()!=digest:
   raise ValueError("Original code window changed "+hex(start))
 for pc,word in ANCHORS.items():
  if elf.word(pc)!=word:raise ValueError("Original opcode differs "+hex(pc))
 for pc,dest in EDGES.items():
  word=elf.word(pc)
  target=((pc+4)&0xf0000000)|((word&0x3ffffff)<<2)
  if word>>26 not in (2,3) or target!=dest:
   raise ValueError("Original J/JAL target differs "+hex(pc))
 return {"format":"ae3/original-spatial-support-source/v1",
  "original_elf_sha256":PIN,"verified_code_windows":len(REGIONS),
  "verified_instruction_words":sum((end-start)//4 for start,end in REGIONS),
  "semantic_instruction_anchors":len(ANCHORS),"verified_direct_targets":len(EDGES),
  "original_methods":["0x4e3c14","0x21b788","0x21b468","0x21b178","0x21b180",
       "0x21b194","0x21b14c","0x21b384","0x21afb0"],
  "still_untranslated":["packed-coordinate source for grid lookup",
       "VU0 collision-plane projection","VU0 vector mean accumulation",
       "VU0 per-segment sqrt distance","original allocator implementation"],
  "not_a_playable_game":True}
if __name__=="__main__":
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument("original_elf",type=Path);p.add_argument("--out",type=Path)
 a=p.parse_args();report=verify(a.original_elf)
 body=json.dumps(report,indent=2)+"\n"
 if a.out:
  a.out.parent.mkdir(parents=True,exist_ok=True)
  a.out.write_text(body,encoding="utf-8")
 else:print(body,end="")
 print("PASS",report["verified_instruction_words"],"source instructions")
