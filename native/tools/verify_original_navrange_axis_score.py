#!/usr/bin/env python3
"""Verify recovered navrange eligibility scalar calculations against a private USA retail ELF."""
import argparse, hashlib, struct
from pathlib import Path
ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
WINDOW_SHA={
 (0x21ef3c,0x21f5a4):'fe4c269c68019fbf5a42174e3d61e036f5f4dd80c0c9972e7e49b53a9e4a221a',
 (0x44bd5c,0x44bdcc):'e294969deeb71f3519e530f293b342a33986a76d58555392cad297018787ce51',
 (0x21f6d4,0x21f804):'8d118549d487c40d3e9b8dc8aa4dce44e12c7a71dc12cbeb7ad953122b9702a5'}
ANCHORS={
 0x21ef94:0x0c1107d0,0x21efa4:0x0c0d679d,
 0x21efd0:0x0c1107d0,0x21efdc:0x0c0d67d1,
 0x21f1ac:0x4492a000,0x21f208:0x46000b07,
 0x21f21c:0x460c0034,0x21f238:0x11e000bc,
 0x21f240:0x46010000,0x21f254:0x46000502,
 0x21f2c8:0x460c0034,0x21f2e0:0x11e00073,
 0x21f2e8:0x46010000,0x21f2fc:0x46000002,
 0x21f314:0x4600a500,0x21f374:0x460c0034,
 0x21f38c:0x11e00029,0x21f394:0x46010000,
 0x21f3a8:0x46000002,0x21f3c0:0x4600a500,
 0x21f3c8:0x13c0000c,0x21f3fc:0x4600a006,
 0x21f434:0x46000834,0x21f458:0x11e0ffdb,
 0x21f460:0x46010001,0x21f478:0x46000002,
 0x21f490:0x4600a500,0x21f4b0:0x46000834,
 0x21f4d4:0x11e0ff91,0x21f4dc:0x46010001,
 0x21f4f4:0x46000002,0x21f50c:0x4600a500,
 0x21f52c:0x46000834,0x21f550:0x11e0ff47,
 0x21f558:0x46010001,0x21f570:0x46000502,
 0x21f750:0x0c087bcf,0x21f75c:0x46010032,
 0x21f760:0x45000019,0x44bd5c:0x00052880,
 0x44bd6c:0x04b80000,0x44bd70:0x70042ee8,
 0x44bd80:0x00052880,0x44bd84:0x440e6000}
BRANCHES={
 0x21f238:0x21f52c,0x21f2e0:0x21f4b0,
 0x21f38c:0x21f434,0x21f550:0x21f270,
 0x21f4d4:0x21f31c,0x21f458:0x21f3c8,
 0x21f3c8:0x21f3fc}
CALLS={
 0x21ef94:0x441f40,0x21efa4:0x359e74,
 0x21efd0:0x441f40,0x21efdc:0x359f44,
 0x21f140:0x44bd80,0x21f1c4:0x44bd80,
 0x21f264:0x44bd80,0x21f310:0x44bd80,
 0x21f3bc:0x44bd80,
 0x21f48c:0x44bd5c,0x21f4a0:0x44bd80,
 0x21f508:0x44bd5c,0x21f51c:0x44bd80,
 0x21f580:0x44bd5c,0x21f594:0x44bd80,
 0x21f750:0x21ef3c}
def verify(path):
 b=path.read_bytes()
 if hashlib.sha256(b).hexdigest()!=ELF_SHA:raise ValueError('Wrong USA retail SCUS_975.01 ELF')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Expected MIPS little-endian ELF32')
 ph=struct.unpack_from('<I',b,28)[0];stride,num=struct.unpack_from('<HH',b,42)
 regions=[]
 for i in range(num):
  typ,off,va,_,sz,_,_,_=struct.unpack_from('<8I',b,ph+i*stride)
  if typ==1 and off+sz<=len(b):regions.append((va,va+sz,off))
 def read(start,end):
  for lo,hi,off in regions:
   if lo<=start and end<=hi:return b[off+start-lo:off+end-lo]
  raise ValueError('Unmapped original VA: '+hex(start))
 def w(addr):return struct.unpack('<I',read(addr,addr+4))[0]
 for (lo,hi),h in WINDOW_SHA.items():
  assert hashlib.sha256(read(lo,hi)).hexdigest()==h,hex(lo)
 for addr,expected in ANCHORS.items():assert w(addr)==expected,hex(addr)
 for addr,target in BRANCHES.items():
  immediate=w(addr)&65535
  if immediate&0x8000:immediate-=65536
  assert addr+4+immediate*4==target,hex(addr)
 for addr,target in CALLS.items():
  op=w(addr)
  assert op>>26==3 and (((addr+4)&0xf0000000)|((op&0x03ffffff)<<2))==target,hex(addr)
 print('PASS: 514 exact original words in 3 SHA-pinned instruction regions')
 print('PASS:',len(ANCHORS),'opcode anchors,',len(BRANCHES),'branches,',len(CALLS),'call edges')
 print('Still needed: actual MMI/VU0 register-vector setup and full production predicate')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('elf',type=Path);verify(p.parse_args().elf)
