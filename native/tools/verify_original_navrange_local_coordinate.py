#!/usr/bin/env python3
"""Audit original retail navrange centroid -> local-space argument path."""
import argparse,hashlib,struct
from pathlib import Path
SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
EXPECTED={
 0x21ef54:0x7004bca9, # PCPYLD s7,zero,a0 source header +0x20
 0x21ef64:0x70078ca9, # PCPYLD s1,zero,a3 original polygon centroid
 0x21f00c:0x48b71800, # QMTC2 s7,VF3
 0x21f014:0x48b10800, # QMTC2 s1,VF1
 0x21f020:0x4be3086c, # VSUB.xyzw VF1,VF1,VF3
 0x21f030:0xd8420000, # LQC2 VF2,0(V0)
 0x21f038:0x48310800, # QMFC2 s1,VF1
 0x21f048:0x48b11800, # QMTC2 s1,VF3
 0x21f04c:0xd8410010, # LQC2 VF1,0x10(V0)
 0x21f050:0x4be311bc, # VMULAx ACC,VF2,VF3.x
 0x21f054:0x4be308bd, # VMADDAy ACC,VF1,VF3.y
 0x21f060:0xd8410020, # LQC2 VF1,0x20(V0)
 0x21f068:0x4be308be, # VMADDAz ACC,VF1,VF3.z
 0x21f070:0xd8410030, # LQC2 VF1,0x30(V0)
 0x21f074:0x4be3088b, # VMADDw VF2,ACC,VF1,VF3.w
 0x21efa4:0x0c0d679d, # JAL quaternion matrix
 0x21efdc:0x0c0d67d1, # JAL original inverse
}
def verify(path):
 b=path.read_bytes()
 if hashlib.sha256(b).hexdigest()!=SHA:raise ValueError('Wrong original USA SCUS_975.01 ELF')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Expected ELF32 MIPS LE')
 ph=struct.unpack_from('<I',b,28)[0]; stride,count=struct.unpack_from('<HH',b,42)
 seg=[]
 for i in range(count):
  typ,off,va,_,sz,_,_,_=struct.unpack_from('<8I',b,ph+i*stride)
  if typ==1 and off+sz<=len(b):seg.append((va,va+sz,off))
 def w(va):
  for lo,hi,off in seg:
   if lo<=va and va+4<=hi:return struct.unpack_from('<I',b,off+va-lo)[0]
  raise ValueError('Unmapped original VA '+hex(va))
 for addr,expected in EXPECTED.items():
  if w(addr)!=expected:raise ValueError('Original opcode does not match at '+hex(addr))
 # The four columns are V0+0,16,32,48; operands VF2,VF1.
 for addr,reg,offset in ((0x21f030,2,0),(0x21f04c,1,16),
                         (0x21f060,1,32),(0x21f070,1,48)):
  x=w(addr)
  assert (x>>26,(x>>16)&31,(x>>21)&31,x&0xffff)==(0x36,reg,2,offset)
 for a,gpr,vf in ((0x21f00c,23,3),(0x21f014,17,1),(0x21f048,17,3)):
  x=w(a)
  assert (x>>26,(x>>21)&31,(x>>16)&31,(x>>11)&31)==(0x12,5,gpr,vf)
 for a,fun in ((0x21f020,44),(0x21f050,60),(0x21f054,61),
               (0x21f068,62),(0x21f074,11)):
  x=w(a)
  assert x>>26==0x12 and (x&63)==fun
 print('PASS:',len(EXPECTED),'original opcode anchors; centroid-minus-origin and 4x4 inverse projection')
 print('LIMIT: scalar distance and other original eligibility intermediates still unported')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('elf',type=Path)
 verify(p.parse_args().elf)
