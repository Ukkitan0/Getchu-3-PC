#!/usr/bin/env python3
"""Original AE3 USA actor projection operand provenance + genuine stage graph audit.

Reads user-owned private retail source files. Never publishes binary bytes.
"""
import argparse,hashlib,json,struct
from pathlib import Path
ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
NAV_SHA='d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0'
REGIONS={
 (0x57c7cc,0x57c8ec):'d5941c30943e58525576393080619c3634bb4ff5dc3dcd1f8999fc08b19d3be5',
 (0x4c1b3c,0x4c1bd8):'ff77828bd77a8683becdc06d797598149a084e5fa5858efaa00c19e92ab0022c',
 (0x21bd3c,0x21bea8):'62dc9cc5c1bab489c842abff8513fae3249924fac8502529b781e959454b2c0f'
}
SOURCE_INSTRUCTIONS={
 0x57c7e4:0x7005aca9, # source incoming A1 saved as original S5
 0x57c7f4:0x8c8f0004, # actor/controller owner
 0x57c7fc:0x11c00036, # multi-polygon route mode
 0x57c810:0x78860040, # original packed controller+0x40 -> A2
 0x57c814:0x0c1306cf, # raycast 0x4C1B3C
 0x57c838:0x8def0000, # original route item 1
 0x57c840:0x0c15f23b, # target-vector 0x57C8EC
 0x57c878:0x0c15f23b, # next target-vector
 0x57c88c:0x70152ca9, # restored original incoming $a1
 0x57c890:0x0c1306cf, # next raycast 0x4C1B3C
 0x57c894:0x700234a9, # target packed $v0 -> original ray $a2
 0x57c8dc:0x0c15f23b, # single source polygon target
}
CALLS={
 0x57c814:0x4c1b3c,0x57c890:0x4c1b3c,
 0x57c840:0x57c8ec,0x57c878:0x57c8ec,0x57c8dc:0x57c8ec,
 0x4c1ba8:0x21bd3c
}
def verify(elf_path,nav_path):
 b=Path(elf_path).read_bytes();nav=Path(nav_path).read_bytes()
 if hashlib.sha256(b).hexdigest()!=ELF_SHA:raise ValueError('Original US ELF SHA-256 mismatch')
 if hashlib.sha256(nav).hexdigest()!=NAV_SHA:raise ValueError('Original woods_c PCK SHA-256 mismatch')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Source requires original little-endian MIPS ELF32')
 ph=struct.unpack_from('<I',b,28)[0];step,n=struct.unpack_from('<HH',b,42)
 segments=[]
 for i in range(n):
  typ,off,va,_,length,_,_,_=struct.unpack_from('<8I',b,ph+i*step)
  if typ==1 and off+length<=len(b):segments.append((va,va+length,off))
 def read(a,length):
  for lo,hi,off in segments:
   if lo<=a and a+length<=hi:return b[off+a-lo:off+a-lo+length]
  raise ValueError(f'Original source unmapped VA {a:#x}')
 def word(a):return struct.unpack('<I',read(a,4))[0]
 for (lo,hi),digest in REGIONS.items():
  if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=digest:
   raise ValueError(f'Original complete source routine differs {lo:#x}')
 for a,w in SOURCE_INSTRUCTIONS.items():
  if word(a)!=w:raise ValueError(f'Source operand instruction changed {a:#x}')
 for a,target in CALLS.items():
  instr=word(a)
  if instr>>26!=3 or ((a+4)&0xf0000000)|((instr&0x3ffffff)<<2)!=target:
   raise ValueError(f'Original JAL edge mismatch {a:#x}')
 if len(nav)!=3328 or nav[:4]!=b'PCK\x00':
  raise ValueError('Original authentic woods_c PCK format differs')
 count=0;edges=0;specials=0
 for i,(start,size) in enumerate(((96,2128),(2224,1104))):
  source=nav[start:start+size]
  n=struct.unpack_from('<I',source,0x24)[0]
  base=struct.unpack_from('<I',source,0x3c)[0]
  if n!=(39 if i==0 else 20) or base+n*30>len(source):
   raise ValueError('Original polygon descriptor header changed')
  for j in range(n):
   at=base+30*j;deg=struct.unpack_from('<H',source,at+2)[0]
   if deg>3:raise ValueError('Original polygon degree > 3')
   for e in range(deg):
    dst=struct.unpack_from('<H',source,at+4+2*e)[0]
    kind=struct.unpack_from('<H',source,at+10+2*e)[0]
    if dst>=n or kind>2:raise ValueError('Original polygon edge outside source graph')
    edges+=1;specials+=(kind==2)
  count+=n
 if (count,edges,specials)!=(59,137,15):raise ValueError('Retail source graph changed')
 return {
   'original_USA_ELF_SHA256':ELF_SHA,
   'original_woods_c_PCK_SHA256':NAV_SHA,
   'complete_original_source_instructions_hashed':sum((hi-lo)//4 for lo,hi in REGIONS),
   'instruction_opcode_anchors':len(SOURCE_INSTRUCTIONS),
   'direct_source_JAL_edges':len(CALLS),
   'original_polygon_count':count,'original_neighbor_links':edges,
   'original_type2_edges':specials,
   'first_raycast_end':'original controller packed vector at +0x40',
   'later_raycast_end':'packed source original 0x57C8EC return value',
   'raycast_start':'original input $a1, not controller+0x20',
   'native_actor_tick_loop_complete':False,'Windows_original_game_booted':False}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('original_elf',type=Path)
 p.add_argument('original_woods_c_navigation_pck',type=Path)
 args=p.parse_args()
 print(json.dumps(verify(args.original_elf,args.original_woods_c_navigation_pck),indent=2))
