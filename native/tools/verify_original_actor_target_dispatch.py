#!/usr/bin/env python3
"""Verify the original AE3 USA target-vector dispatch against private retail data."""
import argparse,hashlib,json,struct
from pathlib import Path
ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
NAV_SHA='d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0'
CODE_SHA='2c5b50641477273a31fbaf9d4e70228b348a5fb416f10d1e8546d4004613ba80'
OPS={0x57c918:0x920f0030,0x57c91c:0x11e0001b,0x57c930:0x0c086de2,
0x57c934:0xc5ec0000,0x57c938:0x304200ff,0x57c93c:0x1040000c,
0x57c944:0xda010020,0x57c948:0x48220800,0x57c970:0x8e0f0004,
0x57c97c:0x0c15f29a,0x57c984:0x1000fff0,0x57c98c:0x0c086d16,
0x57c99c:0x0c086d16,0x57c9b0:0x0c086d16,0x57c9bc:0x0c0dbd35,
0x57c9e0:0x0c0dbd52,0x57ca60:0x1000ffb9,0x708a60:0x3dcccccd}
CALLS={0x57c930:0x21b788,0x57c97c:0x57ca68,
0x57c98c:0x21b458,0x57c99c:0x21b458,0x57c9b0:0x21b458,
0x57c9bc:0x36f4d4,0x57c9e0:0x36f548}
BRANCHES={0x57c91c:0x57c98c,0x57c93c:0x57c970,
0x57c984:0x57c948,0x57ca60:0x57c948}
def verify(elf,nav):
 b=Path(elf).read_bytes();p=Path(nav).read_bytes()
 if hashlib.sha256(b).hexdigest()!=ELF_SHA:raise ValueError('Owner-private SCUS_975.01 SHA mismatch')
 if hashlib.sha256(p).hexdigest()!=NAV_SHA:raise ValueError('Owner-private woods_c/navigation.pck SHA mismatch')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Expected retail MIPS ELF32 little-endian source')
 ph=struct.unpack_from('<I',b,28)[0];step,n=struct.unpack_from('<HH',b,42)
 seg=[]
 for i in range(n):
  typ,off,va,_,size,_,_,_=struct.unpack_from('<8I',b,ph+step*i)
  if typ==1 and off+size<=len(b):seg.append((va,va+size,off))
 def read(a,n):
  for lo,hi,off in seg:
   if lo<=a and a+n<=hi:return b[off+a-lo:off+a-lo+n]
  raise ValueError(f'Original source VA missing {a:#x}')
 def word(a):return struct.unpack('<I',read(a,4))[0]
 if hashlib.sha256(read(0x57c8ec,0x17c)).hexdigest()!=CODE_SHA:
  raise ValueError('Original 95-instruction route target dispatch changed')
 for a,v in OPS.items():
  if word(a)!=v:raise ValueError(f'Original opcode mismatch {a:#x}')
 for a,target in CALLS.items():
  ins=word(a)
  if ins>>26!=3 or (((a+4)&0xf0000000)|((ins&0x3ffffff)<<2))!=target:
   raise ValueError(f'Original call mismatch {a:#x}->{target:#x}')
 for a,target in BRANCHES.items():
  rel=struct.unpack('<h',struct.pack('<H',word(a)&65535))[0]
  if a+4+4*rel!=target:raise ValueError(f'Original branch mismatch {a:#x}')
 if p[:4]!=b'PCK\0' or len(p)!=3328:raise ValueError('Original PCK header or byte count changed')
 for i,(off,size) in enumerate(((96,2128),(2224,1104))):
  g=p[off:off+size]
  if struct.unpack_from('<I',g,0x24)[0]!=(39 if i==0 else 20):
   raise ValueError('Original navigation polygons changed')
 return {'original_USA_ELF_sha256':ELF_SHA,'original_nav_sha256':NAV_SHA,
  'source_instruction_words':95,'source_opcode_anchors':len(OPS),
  'source_JAL_edges':len(CALLS),'source_branch_destinations':len(BRANCHES),
  'source_F12_0x708A60_hex':hex(word(0x708a60)),
  'original_polygon_records':59,
  'fully_recovered_branch':'0x57C944 original controller+0x20 vector low64 passthrough',
  'other_pending_native_VU0_transforms':['0x57C98C','0x57CA68'],
  'complete_native_game_booted':False}
if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('original_elf',type=Path)
 parser.add_argument('original_navigation_pck',type=Path)
 a=parser.parse_args()
 print(json.dumps(verify(a.original_elf,a.original_navigation_pck),indent=2))
