#!/usr/bin/env python3
"""Audit retail Ape Escape 3 USA VU0 XYZ dot and actor projection-ray setup."""
import argparse,hashlib,json,struct
from pathlib import Path
SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
OPS={
 0x4748ac:0x48a40800,0x4748b0:0x48a51000,
 0x4748b4:0x4be2086a,0x4748b8:0x4ba10841,0x4748bc:0x4be10842,
 0x4c1b3c:0x48a50800,0x4c1b44:0x48a61000,
 0x4c1b4c:0x4be110ac,0x4c1b54:0x4be2106a,
 0x4c1b58:0x4ba10841,0x4c1b5c:0x4be10842,
 0x4c1b74:0x46000044,0x4c1b98:0x46010083,
 0x4c1ba4:0x4be11058,0x4c1ba8:0x0c086f4f,
 0x6d0194:0x3f800000
}
VU={
 0x4748b4:(0x2a,1,1,2),0x4748b8:(1,1,1,1),
 0x4748bc:(2,1,1,1),0x4c1b4c:(0x2c,2,2,1),
 0x4c1b54:(0x2a,1,2,2),0x4c1b58:(1,1,1,1),
 0x4c1b5c:(2,1,1,1),0x4c1ba4:(0x18,1,2,1)
}
def verify(path):
 b=Path(path).read_bytes()
 if hashlib.sha256(b).hexdigest()!=SHA:
  raise ValueError('Original USA SCUS_975.01 SHA mismatch')
 ph=struct.unpack_from('<I',b,28)[0];step,n=struct.unpack_from('<HH',b,42)
 segments=[]
 for i in range(n):
  typ,off,va,_,length,_,_,_=struct.unpack_from('<8I',b,ph+i*step)
  if typ==1 and off+length<=len(b):segments.append((va,va+length,off))
 def word(a):
  for lo,hi,off in segments:
   if lo<=a and a+4<=hi:return struct.unpack_from('<I',b,off+a-lo)[0]
  raise ValueError(f'Unmapped original VA: {a:#x}')
 for a,w in OPS.items():
  if word(a)!=w:raise ValueError(f'Original machine opcode changed: {a:#x}')
 for a,(op,fd,fs,ft) in VU.items():
  raw=word(a)
  if (raw&63,(raw>>6)&31,(raw>>11)&31,(raw>>16)&31)!=(op,fd,fs,ft):
   raise ValueError(f'Original VU0 source opcode decoded incorrectly {a:#x}')
 ins=word(0x4c1ba8)
 if ins>>26!=3 or (((0x4c1bac)&0xf0000000)|((ins&0x3ffffff)<<2))!=0x21bd3c:
  raise ValueError('Original polygon traversal source JAL changed')
 return {'retail_original_ELF_SHA256':SHA,
         'full_native_dot_4748ac':True,
         'native_projection_ray_setup_4c1b3c':True,
         'verified_original_instructions':36,
         'verified_VU0_ops':len(VU),'independent_opcode_anchors':len(OPS),
         'polygon_intersector_21bd3c_recovered':False,
         'complete_native_game_booted':False}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('original_elf',type=Path);a=p.parse_args()
 print(json.dumps(verify(a.original_elf),indent=2))
