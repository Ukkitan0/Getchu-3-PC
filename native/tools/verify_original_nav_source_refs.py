#!/usr/bin/env python3
"""Verify retail AE3 source action type-2 pointers and real woods_c records."""
import argparse,hashlib,json,struct
from pathlib import Path
ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
PCK_SHA='d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0'
REGIONS=[(0x21d640,0x21d7c0),(0x21d7c0,0x21d7d8),(0x21d4b4,0x21d520)]
WINDOW_SHA={'21d640-21d7c0':'8f08fd28f2150ffc3d446377fa286b9e38eaaf426bf8410b3c0fcfd7c306f136',
'21d7c0-21d7d8':'dd79230bd13e0d0af02d1aeb68b9bb2b12c08be2a36ebfda76ac172a21cc75c1',
'21d4b4-21d520':'c6cde8f0c939af8a4d980401a168e26899d84ae3b3be371723599a0ea8a53b70'}
ANCHORS={0x21d640:0x964f0002,0x21d650:0x001e7840,
0x21d658:0x95ee000a,0x21d65c:0x240f0001,0x21d670:0x240f0002,
0x21d674:0x11cf0019,0x21d6e8:0x96cf0000,
0x21d6f0:0x8dce0014,0x21d6f4:0x000f7940,
0x21d6fc:0x01cf7021,0x21d704:0x1000ffdd,
0x21d708:0xad8e0010,0x21d7c4:0x8e2f0004,
0x21d4dc:0x8e0f0044,0x21d4f4:0xac4f0014}
CALLS={0x21d724:0x41b800,0x21d734:0x21b0b0,0x21d7a0:0x21b240}
BRANCHES={0x21d674:0x21d6dc,0x21d704:0x21d67c,
0x21d7b0:0x21d764,0x21d7b8:0x21d680}

def audit(elf,pck):
 b=Path(elf).read_bytes();p=Path(pck).read_bytes()
 if hashlib.sha256(b).hexdigest()!=ELF_SHA or hashlib.sha256(p).hexdigest()!=PCK_SHA:
  raise ValueError('Wrong private retail sources')
 ph=struct.unpack_from('<I',b,28)[0];step,n=struct.unpack_from('<HH',b,42)
 segments=[]
 for i in range(n):
  kind,off,va,_,sz,_,_,_=struct.unpack_from('<8I',b,ph+i*step)
  if kind==1 and off+sz<=len(b):segments.append((va,va+sz,off))
 def read(a,n):
  for lo,hi,off in segments:
   if lo<=a and a+n<=hi:return b[off+a-lo:off+a-lo+n]
  raise ValueError('ELF VA unmapped '+hex(a))
 def word(a):return struct.unpack('<I',read(a,4))[0]
 regions={f'{lo:x}-{hi:x}':hashlib.sha256(read(lo,hi-lo)).hexdigest() for lo,hi in REGIONS}
 for label,expected_sha in WINDOW_SHA.items():
  if regions[label]!=expected_sha:raise ValueError('Original code region differs '+label)
 for pc,w in ANCHORS.items():
  if word(pc)!=w:raise ValueError(f'Original source opcode mismatch {pc:#x}, got {word(pc):08x}, expected {w:08x}')
 for pc,dest in CALLS.items():
  w=word(pc)
  if w>>26!=3 or (((pc+4)&0xf0000000)|((w&0x3ffffff)<<2))!=dest:
   raise ValueError('Original call mismatch '+hex(pc))
 for pc,dest in BRANCHES.items():
  off=word(pc)&0xffff
  if off>=0x8000:off-=0x10000
  if pc+4+4*off!=dest:raise ValueError('Original branch mismatch '+hex(pc))
 assert word(0x21d6f0)&65535==0x14
 assert word(0x21d6f4)&63==0 # 32-byte table indexing, SLL 5
 assert word(0x21d708)&65535==0x10
 assert word(0x21d4dc)&65535==0x44
 counts=[];resolved=[]
 for i in range(2):
  _,_,off,length=struct.unpack_from('<4I',p,64+16*i)
  original=p[off:off+length]
  num=struct.unpack_from('<I',original,0x24)[0]
  header=struct.unpack_from('<I',original,0x3c)[0]
  table=struct.unpack_from('<I',original,0x44)[0]
  hits=[]
  for record in range(num):
   at=header+record*0x1e
   child=struct.unpack_from('<H',original,at+2)[0]
   assert child<=3
   for k in range(child):
    kind=struct.unpack_from('<H',original,at+0xa+2*k)[0]
    arg=struct.unpack_from('<H',original,at+0x10+2*k)[0]
    if kind==2:
     assert table+32*(arg+1)<=len(original)
     hits.append([record,k,arg,table+32*arg])
  counts.append(num);resolved.append(hits)
 assert counts==[39,20]
 assert len(resolved[0])==15 and len(resolved[1])==0
 return {'source_function_region_sha256':regions,
   'original_instruction_words':sum((hi-lo)//4 for lo,hi in REGIONS),
   'source_opcode_anchors':len(ANCHORS),'JAL_edges':len(CALLS),'branch_edges':len(BRANCHES),
   'original_type2_actions':[len(x) for x in resolved],
   'original_type2_table_offsets':[2064,1072],
   'original_type2_records_verified':resolved[0],
   'type1_child_constructor_ported':False,'playable_windows_game':False}
if __name__=='__main__':
 q=argparse.ArgumentParser();q.add_argument('elf',type=Path);q.add_argument('pck',type=Path)
 a=q.parse_args();print(json.dumps(audit(a.elf,a.pck),indent=2))
