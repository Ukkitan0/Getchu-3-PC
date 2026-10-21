#!/usr/bin/env python3
"""Verify owner-private retail AE3 USA top-level route modes and fallback."""
import argparse,hashlib,json,struct
from pathlib import Path
PIN='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
REGIONS={
(0x2207d0,0x220994):'0d080b45e4ca0e8f77eab6f804341d2dd1ebf63fbf60961e36c9af25f68b67bf',
(0x21f89c,0x21f8d0):'5e2821d9020e83168bcbbbe74c942a0ad847fa87c9495dde84a848daf1aa2bf6',
(0x4e3dec,0x4e3eb4):'e5874fbd0aeaea4a60f217f31dc2da1494a5839a5b002f7a9f0fca29d8d59c07',
(0x220a4c,0x220aa4):'1389f27874005b240d11d4f092d0ccac3ef259d418a44935da3d6e10663d0019',
}
OPCODES={
0x2207d8:0xac80000c,0x220820:0x0c106e2c,
0x220860:0x12000005,0x220868:0x0200202d,
0x220874:0x0002102b,0x220878:0x10400033,
0x220890:0x0c0882a9,0x2208a8:0x12c00022,
0x2208ac:0x2402ffff,0x2208b0:0x240f0001,
0x2208b4:0x12cf001b,0x220924:0x0c0883d8,
0x220934:0x70142ca9,0x220938:0x0c08840c,
0x220948:0x8e240008,0x22094c:0x0c087e2e,
0x220960:0x0000382d,0x220968:0x0000402d,
0x22096c:0x0c0882a9,0x220984:0x0c086d1a,
0x21f89c:0x27bdfff0,0x21f8a4:0x0c138f6b,
0x21f8b8:0x27bdfff0,0x21f8c0:0x8c840004,
0x21f8c8:0x08138f7b,
0x4e3df0:0x0000102d,0x4e3e28:0xc5f43f00,
0x4e3e6c:0x46140034,0x4e3e78:0x46000506,
0x220a58:0x11e00004,0x220a64:0x081068f6,
0x220a84:0x11e00004,0x220a90:0x081068f6,
0x6e3f00:0x7f7fffff}
CALLS={
0x220820:0x41b8b0,0x22086c:0x21f89c,
0x220890:0x220aa4,0x220898:0x220c00,0x2208a0:0x220d5c,
0x2208c0:0x220994,0x2208f4:0x41a3d8,0x220924:0x220f60,
0x220938:0x221030,0x22094c:0x21f8b8,
0x22096c:0x220aa4,0x220974:0x220c00,0x22097c:0x220d5c,
0x220984:0x21b468,0x21f8a4:0x4e3dac,0x21f8c8:0x4e3dec,
0x4e3e30:0x21b468,0x220a64:0x41a3d8,0x220a90:0x41a3d8}
def verify(path):
 data=path.read_bytes()
 if hashlib.sha256(data).hexdigest()!=PIN:
  raise ValueError('Expected exact USA SCUS_975.01')
 if data[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',data,18)[0]!=8:
  raise ValueError('Expected original MIPS ELF32')
 ph=struct.unpack_from('<I',data,28)[0]
 step,n=struct.unpack_from('<HH',data,42)
 seg=[]
 for i in range(n):
  kind,off,va,_,size,_,_,_=struct.unpack_from('<8I',data,ph+i*step)
  if kind==1 and off+size<=len(data):seg.append((va,va+size,off))
 def read(va,n):
  for start,end,off in seg:
   if start<=va and va+n<=end:return data[off+va-start:off+va-start+n]
  raise ValueError('Unmapped ELF virtual address '+hex(va))
 def word(va):return struct.unpack('<I',read(va,4))[0]
 for (lo,hi),expected in REGIONS.items():
  if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=expected:
   raise ValueError('Original complete compiled function differs '+hex(lo))
 for addr,expected in OPCODES.items():
  if word(addr)!=expected:raise ValueError('Original MIPS opcode differs '+hex(addr))
 for addr,expected in CALLS.items():
  op=word(addr)
  dest=((addr+4)&0xf0000000)|((op&0x3ffffff)<<2)
  if op>>26 not in (2,3) or dest!=expected:
   raise ValueError('Original direct J/JAL target differs '+hex(addr))
 return {'format':'ae3/original-top-route-dispatch/v1','source_sha256':PIN,
  'complete_code_regions':len(REGIONS),
  'original_instruction_words':sum((b-a)//4 for a,b in REGIONS),
  'verified_opcode_sites':len(OPCODES),'verified_J_JAL_targets':len(CALLS),
  'original_entries':['0x2207d0','0x21f89c','0x21f8b8','0x4e3dec',
                     '0x220a4c','0x220a78'],
  'mode_0':'farthest original 0x221030',
  'mode_1':'nearest original 0x220f60',
  'mode_other':'-1 passed to original 0x220994',
  'fallback':'0x21f8b8 -> 0x4e3dec -> 0x21b468 -> 0x220f60',
  'unresolved':'live source linked-list ownership and actual Actor/VU query producer; no playable Windows game'}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('original_elf',type=Path);p.add_argument('--out',type=Path)
 a=p.parse_args();r=verify(a.original_elf);s=json.dumps(r,indent=2)+'\n'
 if a.out:a.out.write_text(s,encoding='utf-8')
 else:print(s,end='')
 print('PASS',r['original_instruction_words'],'original instructions',r['verified_J_JAL_targets'],'call targets')
