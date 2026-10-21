#!/usr/bin/env python3
"""Verify SHA-pinned authentic Ape Escape 3 USA tree insert/rebalance.

No original copyrighted executable or instruction arrays are exported.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
REGIONS={
 (0x541ca0,0x541dac):'d3764fd4515fb84a5802161fa958bfa88e36dd4c964c870a9d6bb25a3bc3085a',
 (0x541dac,0x541e9c):'b2e44051638a86a6b710807800b294ac1451af50cc1add79d3b623e3f5126073',
 (0x49c26c,0x49c398):'d50f1061d0209c4f0053d4ef98e56baba8cef82783df86309972a84a7c9345cb',
 (0x49c3b0,0x49c44c):'76ed01ca325ad08b746be9909abf42b7ea4f34c0721cb77b33772f14cdd085be',
 (0x541fc4,0x541ffc):'de5f707589012906bdbf3ee71e6903714bec49572c3bbc83400fcb9fcc7987b5',
 (0x5419ec,0x541aa4):'c463623742c0eb132a357a7c1cf962c2aeb407512ea79e3a0613ea3e5b24ec4d',
 (0x541564,0x541660):'c02c704146cbdee47e51ecfa04226e5cbd5979225c9f53d09de70acd26f3b042'
}
ANCHORS={
 0x541c5c:0x0c1506a9, 0x541cec:0x0c113908, 0x541cf0:0x24040018,
 0x541d00:0x0c1507f1, 0x541d08:0xae330008, 0x541d18:0xae330004,
 0x541d20:0xadf3000c, 0x541d24:0xae110004, 0x541d2c:0xae000008,
 0x541d30:0xae00000c,0x541d38:0x0c12709b,0x541d48:0xae900000,
 0x541d54:0xae4f0004,0x541d74:0x8dcf0008,0x541d7c:0xadd30008,
 0x541d88:0x0c1508eb,0x541e58:0x0c150728,0x541e64:0xa24f0004,
 0x541fd8:0x0c189bec,0x541fe0:0x8e0f0004,0x541fe8:0xae2f0004,
 0x49c270:0xac800000,0x49c288:0x8e2f0000,0x49c2b4:0x8daf0000,
 0x49c2c0:0x8dac0004,0x49c2d0:0x8d8e000c,0x49c300:0x0c1270ec,
 0x49c318:0x8de40004,0x49c320:0xac800000,0x49c36c:0x0c1270ff,
 0x49c388:0x0c1270ec,0x49c3b0:0x8c8d000c,0x49c3b4:0x8daf0008,
 0x49c3c0:0xade40004,0x49c3fc:0x8c8d0008,0x49c400:0x8daf000c,
 0x541a2c:0x0c189f98,0x541a38:0x8e31000c,0x541a40:0x8e310008
}
CALLS={
 0x541c5c:0x541aa4,0x541cec:0x44e420,
 0x541d00:0x541fc4,0x541d38:0x49c26c,
 0x541d88:0x5423ac,0x541e58:0x541ca0,
 0x541fd8:0x626fb0,0x49c300:0x49c3b0,
 0x49c36c:0x49c3fc,0x49c388:0x49c3b0,
 0x541a2c:0x627e60
}
def verify(file):
 b=Path(file).read_bytes()
 if hashlib.sha256(b).hexdigest()!=ELF_SHA:
  raise ValueError('Wrong original owner-private USA SCUS_975.01 ELF')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Expected original little-endian MIPS ELF32')
 ph=struct.unpack_from('<I',b,28)[0]
 step,count=struct.unpack_from('<HH',b,42)
 segments=[]
 for i in range(count):
  kind,off,va,_,sz,_,_,_=struct.unpack_from('<8I',b,ph+i*step)
  if kind==1 and off+sz<=len(b):segments.append((va,va+sz,off))
 def read(va,n):
  for lo,hi,off in segments:
   if lo<=va and va+n<=hi:return b[off+va-lo:off+va-lo+n]
  raise ValueError(f'Original unmapped address: {va:#x}')
 def word(va):return struct.unpack('<I',read(va,4))[0]
 for (start,end),digest in REGIONS.items():
  if hashlib.sha256(read(start,end-start)).hexdigest()!=digest:
   raise ValueError(f'Original complete function differs: {start:#x}')
 for va,opcode in ANCHORS.items():
  if word(va)!=opcode:raise ValueError(f'Original opcode differs: {va:#x}')
 for va,target in CALLS.items():
  ins=word(va)
  dest=((va+4)&0xf0000000)|((ins&0x03ffffff)<<2)
  if ins>>26!=3 or dest!=target:raise ValueError(f'Original JAL differs: {va:#x}')
 assert word(0x541cf0)&65535==0x18
 assert (word(0x541fe0)&65535,word(0x541fe8)&65535)==(4,4)
 assert word(0x541d08)&65535==8 and word(0x541d2c)&65535==8
 assert word(0x541d30)&65535==12
 assert word(0x49c3b0)&65535==12 and word(0x49c3fc)&65535==8
 assert (word(0x541d38)&0x03ffffff)<<2==0x49c26c
 assert word(0x49c270)==0xac800000
 return {
  'original_elf_sha256':ELF_SHA,
  'complete_function_windows':len(REGIONS),
  'whole_original_instruction_words':sum((end-start)//4 for start,end in REGIONS),
  'independent_exact_instruction_anchors':len(ANCHORS),
  'original_direct_call_edges':len(CALLS),
  'original_record_allocation_size':0x18,
  'original_node_fields':{'color':0,'parent':4,'left':8,'right':12,'key':16,'source_pair_word':20},
  'original_rebalance':'0x49c26c',
  'original_rotate_right':'0x49c3b0',
  'original_rotate_left':'0x49c3fc',
  'original_unique_insert':'0x541dac',
  'original_key_lookup':'0x5419ec',
  'serialized_attribute_tree_population_completed':False,
  'playable_windows_game':False}
if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('original_elf',type=Path)
 x=verify(parser.parse_args().original_elf)
 print('PASS original attribute node insertion, RB rebalance, and key lookup')
 print(json.dumps(x,indent=2))
