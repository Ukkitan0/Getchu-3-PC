#!/usr/bin/env python3
"""Audit original USA 0x21BD3C polygon traversal against private retail inputs."""
import argparse,hashlib,json,struct
from pathlib import Path
ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
NAV_SHA='d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0'
REGION_SHA='62dc9cc5c1bab489c842abff8513fae3249924fac8502529b781e959454b2c0f'
CALLS={0x21bd8c:0x21ee00,0x21bda0:0x21ee00,0x21bdb4:0x21ee00,
       0x21be30:0x21b3f8,0x21be48:0x21b444,
       0x21be60:0x21b404,0x21be78:0x21bd3c}
BRANCHES={0x21bd94:0x21bdc4,0x21bda8:0x21bdc4,
          0x21bdbc:0x21bdc4,0x21bdc8:0x21bea0,
          0x21bdec:0x21bdf8,0x21be00:0x21be30,
          0x21be40:0x21be08,0x21be58:0x21be30,
          0x21be90:0x21be10,0x21be98:0x21be30,
          0x21bea0:0x21be0c}
ANCHORS={0x21bd7c:0xafa00020,0x21bd90:0xe7ac0024,
         0x21bdf4:0xae900000,0x21bdfc:0x46140034,
         0x21be3c:0x0242102a,0x21be58:0x1440fff5,
         0x21be5c:0x26520001,0x21be7c:0x0280382d}
def verify(elf,nav):
 b=Path(elf).read_bytes();n=Path(nav).read_bytes()
 if hashlib.sha256(b).hexdigest()!=ELF_SHA:raise ValueError('Retail USA ELF SHA mismatch')
 if hashlib.sha256(n).hexdigest()!=NAV_SHA:raise ValueError('Retail woods_c PCK SHA mismatch')
 ph=struct.unpack_from('<I',b,28)[0];step,cnt=struct.unpack_from('<HH',b,42)
 seg=[]
 for i in range(cnt):
  typ,off,va,_,size,_,_,_=struct.unpack_from('<8I',b,ph+i*step)
  if typ==1 and off+size<=len(b):seg.append((va,va+size,off))
 def read(a,l):
  for lo,hi,off in seg:
   if lo<=a and a+l<=hi:return b[off+a-lo:off+a-lo+l]
  raise ValueError(f'Original ELF unmapped VA {a:#x}')
 def word(a):return struct.unpack('<I',read(a,4))[0]
 if hashlib.sha256(read(0x21bd3c,0x16c)).hexdigest()!=REGION_SHA:
  raise ValueError('Original recursive polygon traversal changed')
 for a,t in CALLS.items():
  opcode=word(a)
  if opcode>>26!=3 or (((a+4)&0xf0000000)|((opcode&0x3ffffff)<<2))!=t:
   raise ValueError(f'Original source JAL mismatch {a:#x}')
 for a,t in BRANCHES.items():
  imm=struct.unpack('<h',struct.pack('<H',word(a)&0xffff))[0]
  if a+4+4*imm!=t:raise ValueError(f'Original source branch mismatch {a:#x}')
 for a,t in ANCHORS.items():
  if word(a)!=t:raise ValueError(f'Original source opcode mismatch {a:#x}')
 if word(0x6e3ef8)!=0x358637bd:raise ValueError('Original 0x21EE00 epsilon changed')
 graphs=[];totalEdges=totalKind2=0
 for graph,(off,size) in enumerate(((96,2128),(2224,1104))):
  raw=n[off:off+size];count=struct.unpack_from('<I',raw,0x24)[0]
  desc=struct.unpack_from('<I',raw,0x3c)[0]
  if count!=(39 if graph==0 else 20) or desc+count*30>len(raw):
   raise ValueError('Original navigation descriptors mismatch')
  edges=kinds=0
  for i in range(count):
   at=desc+30*i;degree=struct.unpack_from('<H',raw,at+2)[0]
   if degree>3:raise ValueError('Original degree exceeds 3')
   for j in range(degree):
    dst=struct.unpack_from('<H',raw,at+4+2*j)[0]
    typ=struct.unpack_from('<H',raw,at+10+2*j)[0]
    if dst>=count or typ>2:raise ValueError('Original edge kind/target invalid')
    edges+=1;kinds+=(typ==2)
  totalEdges+=edges;totalKind2+=kinds
  graphs.append({'source_polygons':count,'original_edges':edges,'source_type2_edges':kinds})
 if (totalEdges,totalKind2)!=(137,15):raise ValueError('Original retail graph differs')
 return {'retail_USA_ELF_SHA256':ELF_SHA,'retail_navigation_SHA256':NAV_SHA,
   'source_machine_instructions_hashed':91,'direct_original_calls':len(CALLS),
   'source_branch_destinations':len(BRANCHES),'independent_opcodes':len(ANCHORS),
   'retail_graphs':graphs,'original_21EE00_intersection_translated':False,
   'playable_native_windows_game':False}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('retail_elf',type=Path);p.add_argument('retail_woods_c_navigation_pck',type=Path)
 args=p.parse_args()
 print(json.dumps(verify(args.retail_elf,args.retail_woods_c_navigation_pck),indent=2))
