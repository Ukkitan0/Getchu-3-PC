#!/usr/bin/env python3
"""Audit USA Ape Escape 3 original 128-bucket resource dispatch from owner-private ELF."""
import argparse,hashlib,struct,json
from pathlib import Path
ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
WINDOWS={
(0x27242c,0x272474):'676bfd5e15e41305974f7782e3f11a1bd8c13f0a9236a23e3778a9e299b15432',
(0x36d844,0x36d8c8):'65b245dea211fe48df41374ad3188052d3610e2077a25bc238203c2ea30c3a20',
(0x518e24,0x518e4c):'cbd2d83c95364fa9735da64f07cdf9499cf740fff422dac4a00c5b8f0b7684e6',
(0x518e4c,0x518e70):'b0913729bfffdad673279c8821fdbfeae9173fe834f531c780ab1c29d5465422',
(0x4e3d9c,0x4e3dac):'47fe39e0b371f0818dabaad89a73f8b0573eae83043cb1a92498b65242e3a509'}
ANCHORS={
0x272438:0x25eff610,0x27243c:0xafa50004,0x272440:0x8c840000,
0x272444:0xafaf0000,0x27244c:0x0c0db611,0x272450:0x24840004,
0x27245c:0x25eff628,0x272460:0x0c138f67,0x272464:0xafaf0000,
0x36d858:0x2412007f,0x36d864:0x8c900000,0x36d868:0x8e050000,
0x36d86c:0x10a0000c,0x36d870:0x01e0982d,0x36d878:0x8e6f0000,
0x36d880:0x8e260020,0x36d884:0x8def0000,0x36d888:0x01e0f809,
0x36d88c:0x0260202d,0x36d890:0x8e310024,0x36d894:0x5620fff9,
0x36d898:0x8e6f0000,0x36d8a0:0x2652ffff,0x36d8a4:0x0641fff0,
0x36d8a8:0x26100004,0x518e2c:0x8c840004,0x518e30:0x8c8f0000,
0x518e34:0x8def0000,0x518e38:0x01e0f809,0x518e3c:0x00c0282d,
0x518e50:0x8c8f0000,0x518e58:0x8def000c,0x518e5c:0x01e0f809,
0x4e3d9c:0x3c0f006b,0x4e3da0:0x25efe290,0x4e3da8:0xac8f0000}
METHODS={
0x6af610:(0x518e4c,0x518e14,0x518df4,0x518e24),
0x6af628:(0x518e4c,0x518dc8,0x518da8,0x41b8e8)}
def verify(path):
 b=Path(path).read_bytes()
 if hashlib.sha256(b).hexdigest()!=ELF_SHA:raise ValueError('Wrong USA SCUS_975.01 ELF')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Expected MIPS ELF32 LE')
 ph=struct.unpack_from('<I',b,28)[0];stride,n=struct.unpack_from('<HH',b,42)
 seg=[]
 for i in range(n):
  typ,off,va,_,sz,_,_,_=struct.unpack_from('<8I',b,ph+i*stride)
  if typ==1 and off+sz<=len(b):seg.append((va,va+sz,off))
 def read(va,n):
  for lo,hi,off in seg:
   if lo<=va and va+n<=hi:return b[off+va-lo:off+va-lo+n]
  raise ValueError(f'Unmapped retail address {va:#x}')
 def word(a):return struct.unpack('<I',read(a,4))[0]
 for (lo,hi),digest in WINDOWS.items():
  if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=digest:raise ValueError(f'Original code changed at {lo:#x}')
 for va,expected in ANCHORS.items():
  if word(va)!=expected:raise ValueError(f'Instruction differs at {va:#x}')
 for va,expected in METHODS.items():
  if struct.unpack('<4I',read(va,16))!=expected:raise ValueError(f'Vtable differs at {va:#x}')
 def target(va):
  imm=word(va)&65535
  if imm&32768:imm-=65536
  return va+4+4*imm
 assert target(0x36d86c)==0x36d8a0
 assert target(0x36d894)==0x36d87c
 assert target(0x36d8a4)==0x36d868
 assert (word(0x36d858)&65535)==127
 assert (word(0x36d880)&65535,word(0x36d890)&65535)==(0x20,0x24)
 assert word(0x518e58)&65535==12
 for pc,expected in ((0x27244c,0x36d844),(0x272460,0x4e3d9c)):
  op=word(pc)
  assert op>>26==3 and ((op&0x3ffffff)<<2)==expected
 assert 0x6b0000+(word(0x4e3da0)&65535)-65536==0x6ae290
 return {'original_elf_sha256':ELF_SHA,'complete_instruction_regions':len(WINDOWS),
  'original_instruction_words':sum((hi-lo)//4 for lo,hi in WINDOWS),
  'opcode_anchors':len(ANCHORS),'original_buckets':128,
  'original_payload_offset':0x20,'original_next_offset':0x24,
  'event_descriptor_active':'0x6af610','event_descriptor_transition':'0x6af628',
  'event_descriptor_base':'0x6ae290',
  'source_callback_chain':'0x36D844 -> 0x518E4C -> 0x518E24 -> receiver slot 0',
  'commercial_binary_in_repository':False}
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('original_elf',type=Path)
 a=p.parse_args();r=verify(a.original_elf)
 print('PASS original AE3 native resource broadcaster')
 print(json.dumps(r,indent=2))
