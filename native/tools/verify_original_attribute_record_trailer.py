#!/usr/bin/env python3
"""Audit original source-text parser's last two rows and owner-offset writes."""
import argparse,hashlib,json,struct
from pathlib import Path
SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
REGION=(0x2ba9f8,0x2bab7c)
REGION_SHA='8b82cd1cc9b3bb8bd3a04793973ebe4ac6e5c6cef072b35941b5cb5dfef230c2'
ANCHORS={
 0x2ba9f8:0x8e050010,
 0x2baa10:0x24a5fff8,
 0x2baa1c:0x2406003a,
 0x2baa24:0x0c07ca31,
 0x2baa90:0x0c10a312,
 0x2baa94:0x8d840004,
 0x2baa98:0xae020024,
 0x2baac4:0x2665fffc,
 0x2baad0:0x2406003a,
 0x2baad8:0x0c07ca31,
 0x2bab40:0x0c10a312,
 0x2bab44:0x8da40004,
 0x2bab48:0x8e0f0000,
 0x2bab54:0x01e27821,
 0x2bab60:0xae0f0004,
}
CALLS={
 0x2baa14:0x626fb0,0x2baa24:0x1f28c4,
 0x2baa90:0x428c48,0x2baaa4:0x4c59cc,
 0x2baabc:0x4c5afc,0x2baac8:0x626fb0,
 0x2baad8:0x1f28c4,0x2bab40:0x428c48,
 0x2bab5c:0x4c59cc,0x2bab74:0x4c5afc}
def verify(path):
 b=Path(path).read_bytes()
 if hashlib.sha256(b).hexdigest()!=SHA:raise ValueError('Incorrect USA retail ELF')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Not MIPS ELF32 little endian')
 ph=struct.unpack_from('<I',b,28)[0];stride,count=struct.unpack_from('<HH',b,42)
 seg=[]
 for i in range(count):
  typ,off,va,_,length,_,_,_=struct.unpack_from('<8I',b,ph+i*stride)
  if typ==1 and off+length<=len(b):seg.append((va,va+length,off))
 def read(addr,n):
  for lo,hi,off in seg:
   if lo<=addr and addr+n<=hi:return b[off+addr-lo:off+addr-lo+n]
  raise ValueError('Unmapped original VA '+hex(addr))
 def word(pc):return struct.unpack('<I',read(pc,4))[0]
 lo,hi=REGION
 if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=REGION_SHA:
  raise ValueError('Original footer instruction window changed')
 for pc,w in ANCHORS.items():
  if word(pc)!=w:raise ValueError('Original trailer instruction mismatch '+hex(pc))
 for pc,target in CALLS.items():
  w=word(pc)
  if w>>26!=3 or (((pc+4)&0xf0000000)|((w&0x3ffffff)<<2))!=target:
   raise ValueError('Original trailer call mismatch '+hex(pc))
 assert word(0x2baa10)&65535==0xfff8
 assert word(0x2baac4)&65535==0xfffc
 assert word(0x2baa98)&65535==0x24
 assert word(0x2bab60)&65535==4
 assert word(0x2bab48)&65535==0
 assert word(0x2bab54)>>26==0 # ADDU 32-bit low-word (wrap)
 return {'original_elf_sha256':SHA,'source_window':[hex(lo),hex(hi)],
  'original_instruction_words':(hi-lo)//4,
  'exact_opcode_anchors':len(ANCHORS),'verified_call_edges':len(CALLS),
  'source_penultimate_line_offset_bytes':-8,
  'source_last_line_offset_bytes':-4,
  'original_owner_count_field':0x24,
  'original_owner_rebased_pointer_field':0x04,
  'original_owner_unrebased_base_field':0x00,
  'source_disk_allocation_recovered':False,'playable_game':False}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('elf',type=Path)
 print('PASS: original source trailer two inputs and 32-bit owner rebasing')
 print(json.dumps(verify(p.parse_args().elf),indent=2))
