#!/usr/bin/env python3
"""SHA-verify original USA navrange callback registration and owner lifetime."""
import argparse,hashlib,json,struct
from pathlib import Path
SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
TABLES={
 0x6ae248:(0x4e4060,0x4e4050,0x4e4030,0,0,0),
 0x6ae260:(0x4e41bc,0x4e3fdc,0x4e3fbc,0x4e3fec,0,0),
 0x6ae278:(0x4e41bc,0x4e3fac,0x4e3f8c,0x41b8e8,0,0)}
REGIONS={
 (0x21f8d0,0x21f940):'a724baf2b2e68803b0f935c1f0dfd4f2dc895c56247f0b89dddbb9c0071f3c02',
 (0x21f940,0x21f9b4):'547fae0011f7a28cdd0d3491073c112b51a311cbf69194b8ec96225a921929dd',
 (0x27242c,0x272474):'676bfd5e15e41305974f7782e3f11a1bd8c13f0a9236a23e3778a9e299b15432'}
ANCHORS={
 0x21f8ec:0x0c106e00,0x21f8f8:0x0c0db526,
 0x21f900:0x3c0f0064,0x21f908:0xadf07b90,
 0x21f918:0x3c0f006b,0x21f91c:0x25efe248,
 0x21f920:0x0c09c90b,0x21f924:0xafaf0000,
 0x21f944:0x3c0f006b,0x21f94c:0x25efe260,
 0x21f960:0x8e247b90,0x21f964:0x0c0db611,
 0x21f968:0xafaf0000,0x21f96c:0x8e307b90,
 0x21f978:0x0c0db56a,0x21f980:0x0c1068ec,
 0x21f988:0x3c0f006b,0x21f98c:0xae207b90,
 0x21f990:0x25efe278,0x21f998:0x0c138f67,
 0x272440:0x8c840000,0x27244c:0x0c0db611,
 0x272460:0x0c138f67,0x22d8d8:0x0c087e34,
 0x22e064:0x0c087e50,0x21f9c8:0x8de47b90}
CALLS={
 0x21f8ec:0x41b800,0x21f8f8:0x36d498,
 0x21f920:0x27242c,0x21f964:0x36d844,
 0x21f978:0x36d5a8,0x21f980:0x41a3b0,
 0x21f998:0x4e3d9c,0x27244c:0x36d844,
 0x272460:0x4e3d9c,0x22d8d8:0x21f8d0,
 0x22e064:0x21f940}
def verify(path):
 b=Path(path).read_bytes()
 if hashlib.sha256(b).hexdigest()!=SHA:raise ValueError('Wrong original retail USA ELF')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Expected little-endian MIPS ELF32')
 ph=struct.unpack_from('<I',b,28)[0];stride,num=struct.unpack_from('<HH',b,42)
 segments=[]
 for i in range(num):
  typ,off,va,_,size,_,_,_=struct.unpack_from('<8I',b,ph+i*stride)
  if typ==1 and off+size<=len(b):segments.append((va,va+size,off))
 def read(va,size):
  for lo,hi,off in segments:
   if lo<=va and va+size<=hi:return b[off+va-lo:off+va-lo+size]
  raise ValueError('Original VA not mapped: '+hex(va))
 def word(va):return struct.unpack('<I',read(va,4))[0]
 for (start,stop),digest in REGIONS.items():
  if hashlib.sha256(read(start,stop-start)).hexdigest()!=digest:
   raise ValueError('Original lifecycle region differs '+hex(start))
 for va,expected in TABLES.items():
  if struct.unpack('<6I',read(va,24))!=expected:
   raise ValueError('Original descriptor differs '+hex(va))
 for va,expected in ANCHORS.items():
  if word(va)!=expected:raise ValueError('Original opcode differs '+hex(va))
 for va,target in CALLS.items():
  w=word(va)
  if (w>>26)!=3 or (((va+4)&0xf0000000)|((w&0x03ffffff)<<2))!=target:
   raise ValueError('Original JAL edge differs '+hex(va))
 assert word(0x21f918)>>26==15 and (word(0x21f918)&0xffff)==0x6b
 for addr,table in ((0x21f91c,0x6ae248),(0x21f94c,0x6ae260)):
  imm=struct.unpack('<h',read(addr,4)[:2])[0]
  assert 0x6b0000+imm==table
 assert (word(0x21f924)>>26)==43 and (word(0x21f968)>>26)==43
 direct=[]
 for lo,hi,_ in segments:
  for va in range((lo+3)&~3,hi-3,4):
   insn=word(va)
   if insn>>26==3 and ((insn&0x03ffffff)<<2)==0x4e4060:direct.append(va)
 if direct:raise ValueError('Unexpected direct call to navrange callback')
 return {
  'source_elf_sha256':SHA,
  'registration_descriptor_va':'0x6ae248',
  'registration_descriptor_methods':[hex(x) for x in TABLES[0x6ae248]],
  'teardown_descriptor_va':'0x6ae260',
  'teardown_descriptor_methods':[hex(x) for x in TABLES[0x6ae260]],
  'final_cleanup_descriptor_va':'0x6ae278',
  'original_owner_global_va':'0x647b90',
  'stage_init_jal':'0x22d8d8 -> 0x21f8d0',
  'stage_teardown_jal':'0x22e064 -> 0x21f940',
  'original_dispatch_installer':'0x27242c',
  'direct_jal_to_navrange_callback':len(direct),
  'verified_instruction_anchors':len(ANCHORS),
  'verified_jal_edges':len(CALLS),
  'sha_pinned_instruction_words':sum((end-start)//4 for start,end in REGIONS),
  'original_binary_bytes_committed':False}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('original_elf',type=Path)
 p.add_argument('--json',action='store_true')
 a=p.parse_args()
 result=verify(a.original_elf)
 if a.json:print(json.dumps(result,indent=2))
 else:
  print('PASS: retail navrange indirect dispatcher, init/teardown and map lifetime')
  print(json.dumps(result,indent=2))
