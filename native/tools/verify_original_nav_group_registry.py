#!/usr/bin/env python3
"""Verify authentic AE3 USA navrange owner dispatch. Does not publish game bytes."""
import argparse,hashlib,json,struct
from pathlib import Path
SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
WINDOWS={
(0x4e4060,0x4e4154):'338f03f5b36f39989cb74109603ca289f8370d15c473ed44bb05c5fe7536d1f3',
(0x36d6f4,0x36d7c8):'b21c85e4deb8ebdb99e6db3dff416eddb65f5f886cb24651c19bbde045963e8e',
(0x36d610,0x36d6f4):'c3b70a57ebe9d7a204fae920bd629a1199c0ce9f2a7738dc78efa42488af7f67',
(0x21f5a4,0x21f604):'a87c4da53f294bd235c7148b967fb62c86638dc08aa7c1a59c6b958577ec466a',
(0x4e3fec,0x4e4030):'bb4a56b8715b6815a4d5742c385bb3499e589373ec044904bf1e8e8061f32615',
(0x21f6d4,0x21f804):'8d118549d487c40d3e9b8dc8aa4dce44e12c7a71dc12cbeb7ad953122b9702a5',
}
ANCHORS={
0x4e4064:0x3c0f006e,0x4e4080:0x79ef3f10,0x4e4084:0x78ad0000,
0x4e4088:0x71e072a8,0x4e408c:0x71c072a8,0x4e4090:0x71ae6c89,
0x4e4094:0x71af78a8,0x4e4098:0x700f7dc8,0x4e409c:0x01e07827,
0x4e40a0:0x2def0001,0x4e40a4:0x11e00024,
0x4e40ac:0x8caf0054,0x4e40b0:0x19e00021,0x4e40c0:0x8e45005c,
0x4e40c4:0x00138940,0x4e40cc:0x26730001,
0x4e40d0:0x0c0db5bd,0x4e40d4:0x00b12821,
0x4e40e4:0x0c106e00,0x4e40f4:0x0c087d81,
0x4e4108:0x0c0db584,0x4e411c:0x0c087db5,
0x4e4124:0x8e4f0054,0x4e4128:0x026f782a,
0x21f5a8:0xac850000,0x21f5bc:0x0c106e00,
0x21f5c8:0x0c040561,0x21f5e0:0xae300004,
0x4e3ffc:0x10c00008,0x4e4004:0x0c087da4,
0x36d728:0x0c0db3f4,0x36d734:0x0c0db3f4,
}
CALLS={
0x4e40d0:0x36d6f4,0x4e40e4:0x41b800,0x4e40f4:0x21f604,
0x4e4108:0x36d610,0x4e411c:0x21f6d4,
0x36d728:0x36cfd0,0x36d734:0x36cfd0,
0x36d63c:0x36f1ec,0x36d688:0x36cfd0,0x36d694:0x36cfd0,
0x21f5bc:0x41b800,0x21f5c8:0x101584,0x21f5d8:0x44e280,
0x4e4004:0x21f690,0x4e4018:0x41a3b0,
0x21f728:0x21d884,0x21f738:0x21b468,
0x21f750:0x21ef3c,0x21f770:0x4e3dac,0x21f784:0x44e420,
}
def verify(path):
 b=path.read_bytes()
 if hashlib.sha256(b).hexdigest()!=SHA:raise ValueError('Wrong USA original ELF')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Expected original MIPS ELF32')
 ph=struct.unpack_from('<I',b,28)[0];stride,num=struct.unpack_from('<HH',b,42)
 seg=[]
 for i in range(num):
  typ,off,va,_,size,_,_,_=struct.unpack_from('<8I',b,ph+i*stride)
  if typ==1 and off+size<=len(b):seg.append((va,va+size,off))
 def read(addr,size):
  for lo,hi,offset in seg:
   if lo<=addr and addr+size<=hi:return b[offset+addr-lo:offset+addr-lo+size]
  raise ValueError('Unmapped original ELF address '+hex(addr))
 def word(addr):return struct.unpack('<I',read(addr,4))[0]
 for (start,end),digest in WINDOWS.items():
  if hashlib.sha256(read(start,end-start)).hexdigest()!=digest:
   raise ValueError('Original function region changed '+hex(start))
 for pc,w in ANCHORS.items():
  if word(pc)!=w:raise ValueError('Original MIPS opcode mismatch '+hex(pc))
 for pc,target in CALLS.items():
  instr=word(pc)
  dest=((pc+4)&0xf0000000)|((instr&0x3ffffff)<<2)
  if instr>>26 not in (2,3) or dest!=target:
   raise ValueError('Original J/JAL target differs '+hex(pc))
 if read(0x6e3f10,16)!=b'navrange\x00\x00\x00\x00\x00\x00\x00\x00':
  raise ValueError('Original 128-bit navrange literal differs')
 return {
  'format':'ae3/retail-navrange-group-ownership/v1',
  'private_original_elf_sha256':SHA,
  'original_entry_va':'0x4e4060',
  'source_magic':'navrange',
  'source_count_offset':0x54,'source_pointer_offset':0x5c,
  'source_group_key_stride_bytes':32,
  'complete_function_windows':len(WINDOWS),
  'original_instruction_words':sum((e-s)//4 for s,e in WINDOWS),
  'verified_opcode_anchors':len(ANCHORS),
  'verified_direct_J_JAL_targets':len(CALLS),
  'unrecovered':['0x21ef3c VU source eligibility','original live resource-group source allocations'],
  'playable_windows_game':False}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('original_elf',type=Path);p.add_argument('--out',type=Path)
 a=p.parse_args();result=verify(a.original_elf)
 content=json.dumps(result,indent=2)+'\n'
 if a.out:a.out.write_text(content,encoding='utf-8')
 else:print(content,end='')
 print('PASS original navrange:',result['original_instruction_words'],'instructions')
