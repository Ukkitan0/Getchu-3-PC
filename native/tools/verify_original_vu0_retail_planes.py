#!/usr/bin/env python3
"""Verify original VU0 convex polygon plane source, not an invented collision test."""
import argparse,hashlib,json,math,struct
from pathlib import Path
ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
PCK_SHA='d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0'
CODE_SHA='c434176b035c929b0b12062267efb4b069e5fc796a3a715da1d9375a763c7d90'
ANCHORS={0x21b7b4:0x4b60006c,0x21b7c0:0x4a800043,
 0x21b7c4:0x4be2196c,0x21b7d0:0x71cf7488,
 0x21b7d4:0x71ef7ba9,0x21b7d8:0x71ee7b89,
 0x21b7e0:0x4bc50afe,0x21b7e4:0x4bc1286e,
 0x21b7e8:0x4be208aa,0x21b7f4:0x4ba10841,
 0x21b7fc:0x4be10842,0x21b80c:0x460c0001,
 0x21b81c:0x46000834}
def verify(elf,pck):
 b=Path(elf).read_bytes();n=Path(pck).read_bytes()
 if hashlib.sha256(b).hexdigest()!=ELF_SHA:raise ValueError('Original ELF SHA mismatch')
 if hashlib.sha256(n).hexdigest()!=PCK_SHA:raise ValueError('Original woods_c PCK SHA mismatch')
 ph=struct.unpack_from('<I',b,28)[0];step,count=struct.unpack_from('<HH',b,42)
 seg=[]
 for i in range(count):
  typ,off,va,_,size,_,_,_=struct.unpack_from('<8I',b,ph+step*i)
  if typ==1 and off+size<=len(b):seg.append((va,va+size,off))
 def read(a,size):
  for lo,hi,off in seg:
   if lo<=a and a+size<=hi:return b[off+a-lo:off+a-lo+size]
  raise ValueError('Original address unmapped '+hex(a))
 def word(a):return struct.unpack('<I',read(a,4))[0]
 if hashlib.sha256(read(0x21b788,0xcc)).hexdigest()!=CODE_SHA:
  raise ValueError('Original retail VU0 eligibility body changed')
 for a,val in ANCHORS.items():
  if word(a)!=val:raise ValueError('Source instruction changed '+hex(a))
 inside=outside=previousInside=0
 for graph,(off,size) in enumerate(((96,2128),(2224,1104))):
  raw=n[off:off+size]
  count=struct.unpack_from('<I',raw,0x24)[0]
  descriptor=struct.unpack_from('<I',raw,0x3c)[0]
  vertices=struct.unpack_from('<I',raw,0x40)[0]
  if count!=(39 if graph==0 else 20):raise ValueError('Original polygon count mismatch')
  for k in range(count):
   at=descriptor+30*k
   nv=struct.unpack_from('<H',raw,at+0x16)[0]
   if nv!=3:raise ValueError('Original woods_c triangle vertex count changed')
   pts=[]
   for j in range(3):
    ix=struct.unpack_from('<H',raw,at+0x18+2*j)[0]
    pts.append(struct.unpack_from('<4f',raw,vertices+16*ix))
   q=[sum(p[j] for p in pts)/3 for j in range(4)]
   def accept(q,old=False):
    prev=pts[-1]
    for cur in pts:
     dx,dy,dz=[cur[i]-prev[i] for i in range(3)]
     nx,nz=(dy,-dz) if old else (dz,-dx)
     if nx*q[0]+nz*q[2] < nx*prev[0]+nz*prev[2]:
      return False
     prev=cur
    return True
   inside+=accept(q)
   previousInside+=accept(q,old=True)
   for j in range(3):
    prev,cur=pts[(j+2)%3],pts[j]
    dx,dz=cur[0]-prev[0],cur[2]-prev[2]
    l=math.hypot(dx,dz)
    if l<=0:raise ValueError('Original polygon edge zero length')
    query=[(prev[i]+cur[i])/2 for i in range(4)]
    query[0]-=(dz/l)*2;query[2]+=(dx/l)*2
    if accept(query):raise ValueError('Original plane fails to reject exterior')
    outside+=1
 if (inside,outside,previousInside)!=(59,177,0):
  raise ValueError('Retail geometry evidence does not match recovered normal')
 return {'original_ELF_SHA256':ELF_SHA,'original_PCK_SHA256':PCK_SHA,
  'complete_original_VU0_predicate_instructions':51,
  'original_machine_opcode_anchors':len(ANCHORS),
  'authentic_polygon_interior_pass':inside,'authentic_edge_exterior_rejected':outside,
  'previous_wrong_normal_polygon_interior_pass':previousInside,
  'actual_Windows_game_booted':False}
if __name__=='__main__':
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument('original_elf',type=Path);ap.add_argument('original_pck',type=Path)
 a=ap.parse_args()
 print(json.dumps(verify(a.original_elf,a.original_pck),indent=2))
