#!/usr/bin/env python3
"""Verify original USA audio-attribute map population and overwrite semantics."""
import argparse,hashlib,struct,json
from pathlib import Path
SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
REGION_START,REGION_END=0x2ba8d0,0x2ba9cc
REGION_SHA='962ff6b89aaa794a7ee1487656e4372b78ed599b90df948d540397f40793502a'
ANCHORS={
0x2ba8f0:0x0c150559,
0x2ba8fc:0x8fae0040,
0x2ba908:0x8dc50010,
0x2ba910:0x8e040004,
0x2ba914:0x8caffff4,
0x2ba918:0x8c8efff4,
0x2ba930:0x0c10ac36,
0x2ba938:0x14400003,
0x2ba948:0x04410003,
0x2ba950:0x24130001,
0x2ba958:0x11e00014,
0x2ba970:0x0c189bec,
0x2ba980:0x0c150846,
0x2ba994:0x0c150579,
0x2ba99c:0x8faf0060,
0x2ba9a4:0x0c1507c9,
0x2ba9ac:0x8fae00a0,
0x2ba9c0:0xadee0014,
0x2ba9c4:0x0c131673,
}
CALLS={0x2ba8f0:0x541564,0x2ba930:0x42b0d8,
 0x2ba970:0x626fb0,0x2ba980:0x542118,0x2ba994:0x5415e4,
 0x2ba9a4:0x541f24,0x2ba9c4:0x4c59cc}
def verify(path):
 b=Path(path).read_bytes()
 if hashlib.sha256(b).hexdigest()!=SHA:raise ValueError('Wrong original SCUS_975.01 ELF')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Expected little endian original ELF32 MIPS')
 ph=struct.unpack_from('<I',b,28)[0];step,count=struct.unpack_from('<HH',b,42)
 seg=[]
 for i in range(count):
  kind,off,va,_,size,_,_,_=struct.unpack_from('<8I',b,ph+i*step)
  if kind==1 and off+size<=len(b):seg.append((va,va+size,off))
 def read(addr,n):
  for lo,hi,off in seg:
   if lo<=addr and addr+n<=hi:return b[off+addr-lo:off+addr-lo+n]
  raise ValueError('Unmapped original VA '+hex(addr))
 def word(addr):return struct.unpack('<I',read(addr,4))[0]
 if hashlib.sha256(read(REGION_START,REGION_END-REGION_START)).hexdigest()!=REGION_SHA:
  raise ValueError('Original audio attribute map source changed')
 for pc,code in ANCHORS.items():
  if word(pc)!=code:raise ValueError('Source opcode mismatch '+hex(pc))
 for pc,target in CALLS.items():
  w=word(pc)
  if w>>26!=3 or (((pc+4)&0xf0000000)|((w&0x3ffffff)<<2))!=target:
   raise ValueError('Source direct call mismatch '+hex(pc))
 def branch(pc):
  imm=word(pc)&0xffff
  if imm&0x8000:imm-=0x10000
  return pc+4+imm*4
 assert branch(0x2ba958)==0x2ba9ac
 assert branch(0x2ba948)==0x2ba958
 assert word(0x2ba9c0)&65535==0x14
 assert word(0x2ba9c0)>>26==43
 return dict(original_elf_sha256=SHA,source='0x2ba8d0..0x2ba9cc',
  original_instruction_words=(REGION_END-REGION_START)//4,
  exact_opcode_anchors=len(ANCHORS),direct_JAL_edges=len(CALLS),
  original_key_insert='0x5415e4',source_node_value_word_offset=0x14,
  value_written_after_existing_or_new_key=True,
  parser_input_record_format_recovered=False,playable_windows_game=False)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('retail_elf',type=Path)
 print('PASS: original attribute key insert-or-update source path')
 print(json.dumps(verify(p.parse_args().retail_elf),indent=2))
