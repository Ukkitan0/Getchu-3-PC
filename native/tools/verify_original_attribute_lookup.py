#!/usr/bin/env python3
"""Audit retail Ape Escape 3 USA attribute index and typed getters."""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
REGIONS={
 (0x5419ec,0x541aa4):'c463623742c0eb132a357a7c1cf962c2aeb407512ea79e3a0613ea3e5b24ec4d',
 (0x627e60,0x627eb0):'663aa53699355ffdb17cd9f61fde1b9882cad56c43a6d06b3ec3ad5bf6663e70',
 (0x2bb194,0x2bb1ac):'0c2d47e0cbb3f184954d4b731b6d753515c40d7e880e8ae40b645be2be2edf2d',
 (0x2bb1ac,0x2bb1d0):'4b39b6e72cdc4724eeecabbb154e36906136e920bbe3141beac9164896a03752',
 (0x2bb1d0,0x2bb2b0):'f95a9a0e9f1cf9e0b77b623bb72abdff2abff57cda3292cd75c5cbdf1b8d4884',
 (0x2bb2b0,0x2bb3bc):'14c19fea24d584dbc92d6ad3be975963701053bb1ff71f0c12ca2c44468c10ff',
 (0x2bb3bc,0x2bb4f8):'9cf658cb02b1466735a3f269e89b4f297322d064f099da3d77bfe2ee1ae111c2'}
ANCHORS={
 0x541a10:0x8ca50000,0x541a14:0x8ca40004,0x541a18:0x1080000e,
 0x541a28:0x26240010,0x541a2c:0x0c189f98,0x541a34:0x04420003,
 0x541a38:0x8e31000c,0x541a40:0x8e310008,0x541a44:0x1620fff9,
 0x541a64:0x24a50010,0x541a68:0x0c189f98,0x541a70:0x04410002,
 0x541a7c:0xae4f0000,
 0x627e6c:0x8c840000,0x627e70:0x8ca50000,
 0x627e74:0x8c8ffff4,0x627e78:0x8caefff4,
 0x627e8c:0x0c10ac36,0x627ea4:0x01cf1023,
 0x2bb194:0x8c820000,0x2bb198:0x3c0f7fff,
 0x2bb19c:0x35efffff,0x2bb1a0:0x004f1026,
 0x2bb1a8:0x2c420001,0x2bb1b8:0x0c10b113,
 0x2bb1e0:0x46006506,0x2bb208:0x0c189c4a,
 0x2bb21c:0x0c15067b,0x2bb240:0x0c150813,
 0x2bb250:0x028f8021,0x2bb254:0x0c0aec65,
 0x2bb264:0xc6140000,0x2bb288:0x4600a006,
 0x2bb2d8:0x00c0a82d,0x2bb2e8:0x0c189c4a,
 0x2bb2fc:0x0c15067b,0x2bb320:0x0c150813,
 0x2bb330:0x028f8021,0x2bb334:0x0c0aec65,
 0x2bb34c:0x8e100000,0x2bb3b0:0x02a0102d,
 0x2bb458:0x0c0d6ae2,0x2bb460:0x0c0aec6b}
CALLS={
 0x541a2c:0x627e60,0x541a68:0x627e60,0x627e8c:0x42b0d8,
 0x2bb1b8:0x42c44c,0x2bb208:0x627128,
 0x2bb21c:0x5419ec,0x2bb240:0x54204c,0x2bb254:0x2bb194,
 0x2bb2e8:0x627128,0x2bb2fc:0x5419ec,
 0x2bb320:0x54204c,0x2bb334:0x2bb194,
 0x2bb458:0x35ab88,0x2bb460:0x2bb1ac}
def verify(path):
 b=Path(path).read_bytes()
 if hashlib.sha256(b).hexdigest()!=ELF_SHA:
  raise ValueError('Wrong original owner-private USA SCUS_975.01')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Expected little-endian MIPS ELF32')
 ph=struct.unpack_from('<I',b,28)[0];step,count=struct.unpack_from('<HH',b,42)
 seg=[]
 for i in range(count):
  typ,off,va,_,length,_,_,_=struct.unpack_from('<8I',b,ph+i*step)
  if typ==1 and off+length<=len(b):seg.append((va,va+length,off))
 def read(addr,n):
  for lo,hi,off in seg:
   if lo<=addr and addr+n<=hi:return b[off+addr-lo:off+addr-lo+n]
  raise ValueError('Original VA not mapped: '+hex(addr))
 def word(addr):return struct.unpack('<I',read(addr,4))[0]
 for (lo,hi),sha in REGIONS.items():
  if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=sha:
   raise ValueError('Original region changed '+hex(lo))
 for addr,expected in ANCHORS.items():
  if word(addr)!=expected:raise ValueError('Original opcode differs at '+hex(addr))
 for addr,target in CALLS.items():
  ins=word(addr)
  if ins>>26!=3 or (((addr+4)&0xf0000000)|((ins&0x3ffffff)<<2))!=target:
   raise ValueError('Original JAL edge differs '+hex(addr))
 assert (word(0x541a38)&65535,word(0x541a40)&65535,
         word(0x541a28)&65535)==(12,8,16)
 assert (word(0x2bb198)&65535,word(0x2bb19c)&65535)==(0x7fff,0xffff)
 for addr in (0x2bb24c,0x2bb32c):assert word(addr)&65535==8
 assert word(0x2bb264)>>26==0x31 and word(0x2bb34c)>>26==0x23
 assert read(0x6f8e38,15)==b'EmptyStrings32\x00'
 return {
  'original_elf_sha256':ELF_SHA,
  'complete_original_code_regions':len(REGIONS),
  'original_instruction_words':sum((hi-lo)//4 for lo,hi in REGIONS),
  'exact_opcode_anchors':len(ANCHORS),
  'original_call_edges':len(CALLS),
  'original_key_tree_offsets':{'left':8,'right':12,'key':16},
  'original_resolved_value_offset_field':8,
  'numeric_missing_marker':'0x7fffffff',
  'original_string_placeholder':'EmptyStrings32',
  'original_live_attribute_table_decoded':False,
  'game_windows_playable':False}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('original_elf',type=Path)
 report=verify(p.parse_args().original_elf)
 print('PASS original attribute tree and numeric getters')
 print(json.dumps(report,indent=2))
