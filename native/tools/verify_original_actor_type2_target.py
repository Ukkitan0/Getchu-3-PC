#!/usr/bin/env python3
"""Audit original Ape Escape 3 actor type-2 route handoff."""
import argparse,hashlib,json,struct
from pathlib import Path
ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
PCK_SHA='d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0'
REGIONS={
(0x57bd88,0x57bdfc):'64cede9fa2125151214a34fe27b21a1f5cb7a60219c622b5ec57fc66c619ba97',
(0x21b458,0x21b468):'721c9bfa1f3db18082b1b20b0b59e22f2b817816e04bb07020337c02f62376fb'}
ANCHORS={
0x57bd88:0x8e8f0004,0x57bd8c:0x8e840018,
0x57bd90:0x0c088441,0x57bd94:0x8df00030,
0x57bd9c:0x0c086d11,0x57bda4:0x38420002,
0x57bda8:0x2c420001,0x57bdb0:0x51e00049,
0x57bdb8:0x8e8f0018,0x57bdc0:0x8def0000,
0x57bdc4:0x8df00004,0x57bdc8:0xae900050,
0x57bdcc:0x0c086d16,0x57bddc:0x0c086d16,
0x57bde0:0x24050001,0x57bde8:0x24050002,
0x57bdf0:0x0c086d16,
0x21b458:0x00052900,0x21b45c:0x00a42821,
0x21b460:0x03e00008,0x21b464:0x78a20040}
CALLS={0x57bd90:0x221104,0x57bd9c:0x21b444,
0x57bdcc:0x21b458,0x57bddc:0x21b458,0x57bdf0:0x21b458}
def audit(elf,nav):
 b=Path(elf).read_bytes();p=Path(nav).read_bytes()
 if hashlib.sha256(b).hexdigest()!=ELF_SHA:raise ValueError('Private USA ELF hash mismatch')
 if hashlib.sha256(p).hexdigest()!=PCK_SHA:raise ValueError('Private woods_c PCK hash mismatch')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Expected original ELF32 MIPS')
 ph=struct.unpack_from('<I',b,28)[0];step,n=struct.unpack_from('<HH',b,42);seg=[]
 for i in range(n):
  kind,off,va,_,size,_,_,_=struct.unpack_from('<8I',b,ph+step*i)
  if kind==1 and off+size<=len(b):seg.append((va,va+size,off))
 def read(a,n):
  for lo,hi,off in seg:
   if lo<=a and a+n<=hi:return b[off+a-lo:off+a-lo+n]
  raise ValueError(f'Original ELF VA {a:#x} absent')
 def word(a):return struct.unpack('<I',read(a,4))[0]
 for (lo,hi),digest in REGIONS.items():
  if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=digest:
   raise ValueError(f'Original source region mismatch {lo:#x}')
 for a,w in ANCHORS.items():
  if word(a)!=w:raise ValueError(f'Original opcode mismatch {a:#x}')
 for a,t in CALLS.items():
  ins=word(a)
  if ins>>26!=3 or ((a+4)&0xf0000000)|((ins&0x3ffffff)<<2)!=t:
   raise ValueError(f'Original call edge mismatch {a:#x}')
 if len(p)!=3328 or p[:4]!=b'PCK\0':raise ValueError('Original woods_c PCK changed')
 counts=[];kinds=[];links=[]
 for i,(offset,size) in enumerate(((96,2128),(2224,1104))):
  raw=p[offset:offset+size];count=struct.unpack_from('<I',raw,0x24)[0]
  base=struct.unpack_from('<I',raw,0x3c)[0]
  if count!=(39 if i==0 else 20) or base+count*0x1e>size:
   raise ValueError('Original nav descriptors changed')
  counts.append(count);special=0;edges=0
  for j in range(count):
   at=base+0x1e*j;degree=struct.unpack_from('<H',raw,at+2)[0]
   if degree>3:raise ValueError('Unexpected source adjacency')
   for slot in range(degree):
    neighbor=struct.unpack_from('<H',raw,at+4+2*slot)[0]
    kind=struct.unpack_from('<H',raw,at+0xa+2*slot)[0]
    if neighbor>=count or kind>2:raise ValueError('Unexpected source neighbor/action')
    special+=(kind==2);edges+=1
  kinds.append(special);links.append(edges)
 return {'original_elf_sha256':ELF_SHA,'original_nav_sha256':PCK_SHA,
    'retail_instructions_audited':sum((hi-lo)//4 for lo,hi in REGIONS),
    'source_opcodes_pinned':len(ANCHORS),'source_calls_verified':len(CALLS),
    'source_polygons':counts,'original_edges':links,'original_type2_actions':kinds,
    'source_target':'route[1] -> controller+0x50',
    'source_vectors':'0x21B458 original slots 0,1,2',
    'unrecovered':'VU0 vector interpolation and actor movement after 0x57BDFC'}
if __name__=='__main__':
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument('original_elf',type=Path);ap.add_argument('woods_c_navigation_pck',type=Path)
 a=ap.parse_args();print(json.dumps(audit(a.original_elf,a.woods_c_navigation_pck),indent=2))
