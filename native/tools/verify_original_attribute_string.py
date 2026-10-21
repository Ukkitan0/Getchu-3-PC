#!/usr/bin/env python3
"""Original Ape Escape 3 USA string getter 0x2BB3BC, retail ELF audit.

Checks only the owner's private executable. No game data is published.
"""
import argparse,hashlib,json,struct
from pathlib import Path
SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
REGIONS={
(0x2bb3bc,0x2bb4f8):'9cf658cb02b1466735a3f269e89b4f297322d064f099da3d77bfe2ee1ae111c2',
(0x2bb1ac,0x2bb1d0):'4b39b6e72cdc4724eeecabbb154e36906136e920bbe3141beac9164896a03752',
(0x35ab88,0x35ac14):'89fcd96294e67168372507ecc27a84193afac7f1d73ba43e37bbf69890210a4a',
(0x42c44c,0x42c590):'0d7de6655503cb4da5fb662fd3458aa2316f3b3c15bd4e5a692a1e54e5104d71',
(0x5419ec,0x541aa4):'c463623742c0eb132a357a7c1cf962c2aeb407512ea79e3a0613ea3e5b24ec4d',
(0x1bb9a8,0x1bbb18):'441da325cbbcf3e04118db2e833a692ed0ed7ad8e11381c6e942ddb4616afb04'}
ANCHORS={
0x2bb3c0:0x78ef0010,0x2bb3c8:0x78ee0000,
0x2bb3d0:0x00a0a82d,0x2bb3d8:0x00c0282d,0x2bb3e0:0x0080902d,
0x2bb3ec:0x7faf0010,0x2bb3f4:0x7fae0000,
0x2bb40c:0x0c189c4a,0x2bb41c:0x0c15067b,
0x2bb424:0x8faf0050,0x2bb42c:0x8e2e06dc,
0x2bb438:0x11ee0027,0x2bb440:0x0c150813,
0x2bb448:0x8c4f0000,0x2bb450:0x8df00008,
0x2bb454:0x02b08021,0x2bb458:0x0c0d6ae2,
0x2bb460:0x0c0aec6b,0x2bb468:0x10400017,
0x2bb474:0x7bae0010,0x2bb478:0x7baf0000,
0x2bb480:0x7e4f0000,0x2bb484:0x7e4e0010,
0x2bb488:0x0c189b52,0x2bb4c8:0x8fa40030,
0x2bb4cc:0x7bae0070,0x2bb4d0:0x1000ffea,
0x2bb4d4:0x7baf0060,0x2bb4d8:0x7baf0010,
0x2bb4e0:0x7bae0000,0x2bb4e8:0x7e4e0000,
0x2bb4ec:0x7e4f0010,0x2bb4f0:0x1000ffe5,
0x2bb1b8:0x0c10b113,0x2bb1c4:0x2c420001,
0x35ab94:0x10a00004,0x35ab9c:0x80af0000,
0x35abc4:0x90ad0000,0x35abfc:0x0c0d685f,
0x1bb9e8:0x0c0aecef,0x1bba20:0x0c0aecef,
0x1bba58:0x0c0aecef,0x1bba90:0x0c0aecef,
0x1bbac8:0x0c0aecef}
CALLS={
0x2bb40c:0x627128,0x2bb41c:0x5419ec,
0x2bb440:0x54204c,0x2bb458:0x35ab88,
0x2bb460:0x2bb1ac,0x2bb488:0x626d48,
0x2bb490:0x626c70,0x2bb498:0x626c68,
0x2bb1b8:0x42c44c,0x35abfc:0x35a17c,
0x1bb9e8:0x2bb3bc,0x1bba20:0x2bb3bc,
0x1bba58:0x2bb3bc,0x1bba90:0x2bb3bc,
0x1bbac8:0x2bb3bc}
def verify(path):
 b=Path(path).read_bytes()
 if hashlib.sha256(b).hexdigest()!=SHA:raise ValueError('Expected original USA SCUS_975.01')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Expected original little-endian MIPS ELF32')
 ph=struct.unpack_from('<I',b,28)[0];sz,count=struct.unpack_from('<HH',b,42)
 seg=[]
 for i in range(count):
  typ,off,va,_,n,_,_,_=struct.unpack_from('<8I',b,ph+i*sz)
  if typ==1 and off+n<=len(b):seg.append((va,va+n,off))
 def read(va,n):
  for lo,hi,off in seg:
   if lo<=va and va+n<=hi:return b[off+va-lo:off+va-lo+n]
  raise ValueError('Source address unmapped: '+hex(va))
 def word(a):return struct.unpack('<I',read(a,4))[0]
 for (lo,hi),digest in REGIONS.items():
  if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=digest:
   raise ValueError('Original function differs at '+hex(lo))
 for pc,op in ANCHORS.items():
  if word(pc)!=op:raise ValueError('Original opcode mismatch '+hex(pc))
 for pc,target in CALLS.items():
  op=word(pc)
  if op>>26!=3 or (((pc+4)&0xf0000000)|((op&0x3ffffff)<<2))!=target:
   raise ValueError('Original JAL differs at '+hex(pc))
 def branch(pc):
  n=word(pc)&65535
  if n>=32768:n-=65536
  return pc+4+n*4
 for pc,target in ((0x2bb438,0x2bb4d8),(0x2bb468,0x2bb4c8),
                   (0x2bb4d0,0x2bb47c),(0x2bb4f0,0x2bb488)):
  if branch(pc)!=target:raise ValueError('Branch target differs '+hex(pc))
 assert word(0x2bb468)>>26==4 # BEQZ marker==false takes source string
 assert word(0x2bb1c4)>>26==11 # SLTIU converts strcmp==0 to equality bool
 if read(0x6f8e38,15)!=b'EmptyStrings32\x00':
  raise ValueError('Missing retail string placeholder literal')
 for pc,rt,offset,base in ((0x2bb3c0,15,16,7),(0x2bb3c8,14,0,7),
                          (0x2bb4cc,14,0x70,29),(0x2bb4d4,15,0x60,29)):
  x=word(pc)
  assert (x>>26,(x>>21)&31,(x>>16)&31,x&65535)==(30,base,rt,offset)
 return {
  'original_elf_sha256':SHA,'complete_source_regions':len(REGIONS),
  'instruction_words':sum((hi-lo)//4 for lo,hi in REGIONS),
  'opcode_anchors':len(ANCHORS),'verified_JAL_edges':len(CALLS),
  'source_default_sentinel':'EmptyStrings32',
  'source_tree_deserialization_completed':False,'windows_game_playable':False}
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('original_elf',type=Path)
 report=verify(p.parse_args().original_elf)
 print('PASS: retail source string attribute lookup and fallback behavior')
 print(json.dumps(report,indent=2))
