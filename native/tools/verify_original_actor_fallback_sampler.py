#!/usr/bin/env python3
"""Audit original Ape Escape 3 USA 0x57CA68 fallback six-sample selector.

No original executable or resource contents are emitted.
"""
import argparse,hashlib,json,struct
from pathlib import Path
ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
NAV_SHA='d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0'
BODY_SHA='6e33608fffc7f8f6980c8eefaee4f856925ba2b0d60fa7fd095dbc8f9c80c886'
CALLS={
 0x57caa8:0x21b458,0x57cab8:0x21b458,0x57cacc:0x21b458,
 0x57cadc:0x36f4d4,0x57cb00:0x36f548,0x57cbc4:0x4748ac,
 0x57cbd8:0x36f4d4,0x57cbfc:0x36f548,0x57ccbc:0x4748ac
}
ANCHORS={
 0x57ca68:0x27bdff70,0x57cbbc:0x24120004,
 0x57cbcc:0x46000586,0x57ccd0:0x46000586,
 0x57ccb4:0x2652ffff,0x57ccc4:0x4600b034,
 0x57ccc8:0x45000003,0x57ccd4:0x70108ca9,
 0x57ccd8:0x0641ffbe,
 0x708a70:0x3f800000,0x708a74:0x3f4ccccd,
 0x708a78:0x3d888888,
}
def verify(elf_file,nav_file):
 b=Path(elf_file).read_bytes();nav=Path(nav_file).read_bytes()
 if hashlib.sha256(b).hexdigest()!=ELF_SHA:raise ValueError('Original USA ELF SHA mismatch')
 if hashlib.sha256(nav).hexdigest()!=NAV_SHA:raise ValueError('Owner-original woods_c PCK SHA mismatch')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Expected source original little-endian MIPS ELF32')
 ph=struct.unpack_from('<I',b,28)[0];step,n=struct.unpack_from('<HH',b,42)
 segments=[]
 for i in range(n):
  typ,off,va,_,length,_,_,_=struct.unpack_from('<8I',b,ph+i*step)
  if typ==1 and off+length<=len(b):segments.append((va,va+length,off))
 def read(a,length):
  for lo,hi,off in segments:
   if lo<=a and a+length<=hi:return b[off+a-lo:off+a-lo+length]
  raise ValueError(f'Original ELF VA not mapped {a:#x}')
 def word(a):return struct.unpack('<I',read(a,4))[0]
 if hashlib.sha256(read(0x57ca68,0x57cd1c-0x57ca68)).hexdigest()!=BODY_SHA:
  raise ValueError('Original fallback random candidate code changed')
 for a,w in ANCHORS.items():
  if word(a)!=w:raise ValueError(f'Original source opcode differs at {a:#x}')
 for a,dest in CALLS.items():
  w=word(a)
  if w>>26!=3 or ((((a+4)&0xf0000000)|((w&0x3ffffff)<<2))!=dest):
   raise ValueError(f'Original direct source call mismatch {a:#x}')
 if (word(0x57ccd8)>>26)!=1 or (word(0x57ccd8)>>16)&31 != 1:
  raise ValueError('Original fallback bgez loop opcode changed')
 rel=struct.unpack('<h',struct.pack('<H',word(0x57ccd8)&65535))[0]
 if 0x57ccdc+4*rel!=0x57cbd4:
  raise ValueError('Original fallback branch does not repeat source trial')
 polygons=edges=kind2=0
 for graph,(start,size) in enumerate(((96,2128),(2224,1104))):
  g=nav[start:start+size]
  count=struct.unpack_from('<I',g,0x24)[0]
  desc=struct.unpack_from('<I',g,0x3c)[0]
  if count!=(39 if graph==0 else 20) or desc+count*30>len(g):
   raise ValueError('Original source polygons changed')
  polygons+=count
  for j in range(count):
   at=desc+j*30;degree=struct.unpack_from('<H',g,at+2)[0]
   if degree>3:raise ValueError('Original edge degree out of range')
   for edge in range(degree):
    neighbor=struct.unpack_from('<H',g,at+4+edge*2)[0]
    kind=struct.unpack_from('<H',g,at+10+edge*2)[0]
    if neighbor>=count or kind>2:raise ValueError('Original source link changed')
    edges+=1;kind2+=(kind==2)
 if (polygons,edges,kind2)!=(59,137,15):
  raise ValueError('Original owner source navigation differs')
 return {
  'original_ELF_SHA256':ELF_SHA,'original_navigation_SHA256':NAV_SHA,
  'original_machine_instructions_hashed':173,'original_direct_calls':len(CALLS),
  'original_opcode_anchors':len(ANCHORS),
  'initial_candidate_samples':1,'additional_loop_candidates':5,
  'source_rng_calls_per_candidate':2,'total_source_rng_calls':12,
  'source_selection':'strict C.OLT.S best < score; ties/NaN do not replace',
  'retail_polygons':polygons,'retail_links':edges,
  'source_VU0_candidate_transform_recovered':False,
  'native_windows_game_booted':False
 }
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('original_elf',type=Path)
 p.add_argument('original_woods_c_navigation_pck',type=Path)
 a=p.parse_args()
 print(json.dumps(verify(a.original_elf,a.original_woods_c_navigation_pck),indent=2))
