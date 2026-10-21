#!/usr/bin/env python3
"""Audit original Ape Escape 3 USA query endpoint selection modes."""
import argparse,hashlib,json,struct
from pathlib import Path
ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
NAV_SHA='d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0'
WINDOWS={(0x220924,0x220994):'ae89128db77bbec8a18ff7b581e65b46e47e8cd880d14483766548dff8e9e389',
 (0x220f60,0x221030):'ce1ecf130e36912ca0b8318dd39477c90f406a1988c86adf4a6c1839b9f7a63a',
 (0x221030,0x221104):'026285e6343cbeec99f543d4bdf61800b9a9ff6c806b337569c933780fef675e'}
ANCHORS={
0x220924:0x0c0883d8,0x220938:0x0c08840c,
0x220f7c:0x8c8f000c,0x220f80:0x19e00023,
0x220f90:0xc5f447ec,0x220fa0:0x240f0038,
0x220fb0:0x0c086d1a,0x220fe8:0x46140034,
0x220fec:0x45000003,0x220ff4:0x46000506,
0x220ff8:0x0200902d,0x221004:0x15e0ffe7,
0x22104c:0x8c8f000c,0x221050:0x19e00024,
0x221060:0xc5e047f0,0x22106c:0x46000507,
0x221074:0x240f0038,0x221084:0x0c086d1a,
0x2210bc:0x4600a034,0x2210c0:0x45000003,
0x2210c8:0x46000506,0x2210cc:0x0200902d,
0x2210d8:0x15e0ffe7,
}
CALLS={0x220924:0x220f60,0x220938:0x221030,
0x220fb0:0x21b468,0x221084:0x21b468}
BRANCHES={0x220f80:0x221010,0x220fec:0x220ffc,
0x221004:0x220fa4,0x221050:0x2210e4,
0x2210c0:0x2210d0,0x2210d8:0x221078}
def audit(elf,nav):
 b=Path(elf).read_bytes();p=Path(nav).read_bytes()
 if hashlib.sha256(b).hexdigest()!=ELF_SHA or hashlib.sha256(p).hexdigest()!=NAV_SHA:
  raise ValueError('Wrong original user-owned USA retail ELF or PCK')
 ph=struct.unpack_from('<I',b,28)[0];stride,n=struct.unpack_from('<HH',b,42)
 segments=[]
 for i in range(n):
  typ,off,va,_,size,_,_,_=struct.unpack_from('<8I',b,ph+i*stride)
  if typ==1 and off+size<=len(b):segments.append((va,va+size,off))
 def read(va,n):
  for lo,hi,off in segments:
   if lo<=va and va+n<=hi:return b[off+va-lo:off+va-lo+n]
  raise ValueError('Unmapped retail MIPS VA '+hex(va))
 def word(a):return struct.unpack('<I',read(a,4))[0]
 for (lo,hi),sha in WINDOWS.items():
  if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=sha:raise ValueError('Original retail code changed '+hex(lo))
 for a,w in ANCHORS.items():
  if word(a)!=w:raise ValueError(f'Original instruction mismatch {a:#x}: {word(a):08x} != {w:08x}')
 for a,target in CALLS.items():
  w=word(a)
  if w>>26!=3 or (((a+4)&0xf0000000)|((w&0x3ffffff)<<2))!=target:
   raise ValueError('Original call differs '+hex(a))
 for a,target in BRANCHES.items():
  d=word(a)&65535
  if d>=32768:d-=65536
  if a+4+4*d!=target:raise ValueError('Source branch differs '+hex(a))
 assert word(0x220fe8)==0x46140034 # C.OLT.S distance < best
 assert word(0x2210bc)==0x4600a034 # C.OLT.S best < distance
 assert word(0x22106c)==0x46000507 # NEG.S initialize -FLT_MAX
 assert struct.unpack('<I',read(0x6e47ec,4))[0]==0x7f7fffff
 assert struct.unpack('<I',read(0x6e47f0,4))[0]==0x7f7fffff
 assert p[:4]==b'PCK\0' and struct.unpack_from('<II',p,4)==(64,2)
 count=[struct.unpack_from('<I',p,struct.unpack_from('<4I',p,64+16*i)[2]+0x24)[0] for i in range(2)]
 assert count==[39,20]
 return {'source_regions':len(WINDOWS),
  'source_instruction_words':sum((hi-lo)//4 for lo,hi in WINDOWS),
  'opcodes':len(ANCHORS),'JAL_edges':len(CALLS),'branches':len(BRANCHES),
  'source_nearest':'0x220F60 first minimum XYZ distance',
  'source_farthest':'0x221030 first maximum XYZ distance',
  'mode_0':'farthest','mode_1':'nearest','other_mode':'provided search index',
  'retail_nav_polygons':count,'playable_windows_game':False}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('original_elf',type=Path);p.add_argument('private_navigation_pck',type=Path)
 args=p.parse_args();print('PASS original NavGraph endpoint mode selection')
 print(json.dumps(audit(args.original_elf,args.private_navigation_pck),indent=2))
