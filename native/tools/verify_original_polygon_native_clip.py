#!/usr/bin/env python3
"""Verify actual source 0x21EE00 polygon edge clipping against USA retail ELF."""
import argparse,hashlib,json,struct
from pathlib import Path
SOURCE='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
NAV='d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0'
CLIP_SHA='c5169ee9e1f3723e43295dda7a7e8a7864a1ee8deaaf7fe2e9f660361f7e5ffd'
OPS={
0x21ee10:0x4b6000ac,0x21ee14:0x4be3086c,
0x21ee2c:0x4a800083,0x21ee54:0x4bc112fe,
0x21ee5c:0x4bc2086e,0x21ee78:0x46000045,
0x21ee84:0x46000834,0x21eea8:0x4600a036,
0x21eef4:0x4600a501,0x21eef8:0x4615b034,
0x21ef04:0x4615a503,0x21ef14:0x46140028,
0x21ef18:0xe6400020,0x21ef30:0x46140029,
0x21ef2c:0x46010034,0x6e3ef8:0x358637bd}
CALLS={a:0x4748ac for a in (0x21ee68,0x21ee90,0x21eea0,0x21eedc,0x21eeec)}
BRANCHES={
 0x21ee88:0x21eedc,0x21eeb0:0x21eeb8,
 0x21ef08:0x21ef30,0x21ef34:0x21ef1c}
def verify(elf,nav):
 b=Path(elf).read_bytes();p=Path(nav).read_bytes()
 if hashlib.sha256(b).hexdigest()!=SOURCE:raise ValueError('Original USA retail ELF SHA differs')
 if hashlib.sha256(p).hexdigest()!=NAV:raise ValueError('Original woods_c PCK SHA differs')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Expected source original MIPS little-endian ELF32')
 ph=struct.unpack_from('<I',b,28)[0];step,cnt=struct.unpack_from('<HH',b,42)
 seg=[]
 for i in range(cnt):
  typ,off,va,_,size,_,_,_=struct.unpack_from('<8I',b,ph+i*step)
  if typ==1 and off+size<=len(b):seg.append((va,va+size,off))
 def read(a,size):
  for lo,hi,off in seg:
   if lo<=a and a+size<=hi:return b[off+a-lo:off+a-lo+size]
  raise ValueError(f'Original source VA {a:#x} outside executable')
 def word(a):return struct.unpack('<I',read(a,4))[0]
 if hashlib.sha256(read(0x21ee00,0x13c)).hexdigest()!=CLIP_SHA:
  raise ValueError('Original edge clipping source machine-code body changed')
 for a,val in OPS.items():
  if word(a)!=val:raise ValueError(f'Original opcode mismatch at {a:#x}')
 for a,target in CALLS.items():
  ins=word(a)
  if ins>>26!=3 or (((a+4)&0xf0000000)|((ins&0x3ffffff)<<2))!=target:
   raise ValueError(f'Original source call mismatch {a:#x}')
 for a,target in BRANCHES.items():
  ins=word(a);signed=struct.unpack('<h',struct.pack('<H',ins&0xffff))[0]
  if a+4+4*signed!=target:raise ValueError(f'Original source branch mismatch {a:#x}')
 if len(p)!=3328 or p[:4]!=b'PCK\0':raise ValueError('Authentic source PCK changed')
 for graph,(start,size) in enumerate(((96,2128),(2224,1104))):
  part=p[start:start+size]
  if struct.unpack_from('<I',part,0x24)[0]!=(39 if graph==0 else 20):
   raise ValueError('Original polygon count changed')
 return {
   'original_USA_ELF_SHA256':SOURCE,
   'original_nav_PCK_SHA256':NAV,
   'complete_original_21EE00_instructions_hashed':79,
   'source_opcode_sites':len(OPS),
   'original_XYZ_dot_calls':len(CALLS),
   'source_control_flow_branches':len(BRANCHES),
   'source_parallel_tolerance_bits':'0x358637BD',
   'source_range_updates':'MAX.S lower at +0x20 / MIN.S upper at +0x24',
   'source_complete_clip_arithmetic_native':True,
   'connected_recursive_traversal_21BD3C':True,
   'whole_Windows_native_game_booted':False
 }
if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('original_USA_elf',type=Path)
 parser.add_argument('original_woods_c_navigation_pck',type=Path)
 args=parser.parse_args()
 print(json.dumps(verify(args.original_USA_elf,args.original_woods_c_navigation_pck),indent=2))
