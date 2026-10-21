#!/usr/bin/env python3
"""Audit original AE3 USA actor movement-controller constructors and dispatch.

Reads the owner's SHA-pinned private retail ELF. Outputs only metadata.
"""
import argparse, hashlib, json, struct
from pathlib import Path

ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
REGIONS={
(0x57ba98,0x57bb60):'05af2724bfe2f19ebb7fd52bd7e5c21c9a2caf348367aeda64e545474d27b6e2',
(0x57bb60,0x57bcb8):'fda78f6b9961e87ee9f85cbd366d89da5ca256298634c64b7bdb6eda510a8702',
(0x57bcb8,0x57bd3c):'ca0b2213c64510b3491ec4799aebc5e6a0bb2b375cd43ea0c9f9094e75070f38',
(0x317538,0x317610):'cd57e7145b12481f31dbf0e598949454d07befa823d07256aa44ae27732929c9'}
OPCODES={
0x57ba9c:0xac800010,0x57baac:0xac850004,0x57bab4:0xa087001d,
0x57babc:0xa0800030,0x57bac4:0xac800014,0x57bac8:0x8ccd0004,
0x57bad8:0x29ad0002,0x57bae0:0x39ad0001,0x57bae8:0xa08d001c,
0x57bafc:0x7c870020,0x57bb0c:0xac850004,0x57bb14:0xa088001d,
0x57bb1c:0xac800010,0x57bb28:0x240f0001,0x57bb30:0xa08f0030,
0x57bb34:0x29ad0002,0x57bb38:0xac860018,0x57bb40:0x39ad0001,
0x57bb48:0xa08d001c,0x57bb50:0xac8f0000,0x57bb7c:0x8c8e0010,
0x57bbb4:0x27bdfff0,0x57bc14:0x8c8f0004,0x57bc18:0x8def0040,
0x57bc1c:0x15e00008,0x57bc24:0x8c8f0010,0x57bc34:0x8de20008,
0x57bc60:0x8c8e0010,0x57bc70:0x8de20010,0x57bc90:0x8c8f0010,
0x57bca0:0x8def000c,0x57bcb8:0x27bdff80,0x57bcdc:0x908f0030,
0x57bce0:0x11e00012,0x57bd24:0x45010002,
0x31755c:0x0c106e00,0x317574:0x0c15eea6,
0x3175c8:0x0c106e00,0x3175e4:0x0c15eebe}
CALLS={0x57bae4:0x57bcb8,0x57bb4c:0x57bcb8,
0x31755c:0x41b800,0x317574:0x57ba98,
0x3175c8:0x41b800,0x3175e4:0x57baf8}
JUMPS={0x57bc04:0x41a3b0}
BRANCHES={0x57bc1c:0x57bc40,0x57bce0:0x57bd2c}
def verify(path):
 b=Path(path).read_bytes()
 if hashlib.sha256(b).hexdigest()!=ELF_SHA:raise ValueError('Original USA SCUS_975.01 SHA mismatch')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Expected original little-endian ELF32 MIPS')
 ph=struct.unpack_from('<I',b,28)[0];step,count=struct.unpack_from('<HH',b,42)
 seg=[]
 for i in range(count):
  kind,off,va,_,size,_,_,_=struct.unpack_from('<8I',b,ph+step*i)
  if kind==1 and off+size<=len(b):seg.append((va,va+size,off))
 def read(va,size):
  for lo,hi,off in seg:
   if lo<=va and va+size<=hi:return b[off+va-lo:off+va-lo+size]
  raise ValueError(f'Original executable address unmapped {va:#x}')
 def word(va):return struct.unpack('<I',read(va,4))[0]
 for (lo,hi),digest in REGIONS.items():
  if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=digest:
   raise ValueError(f'Original code region mismatch {lo:#x}')
 for va,w in OPCODES.items():
  if word(va)!=w:raise ValueError(f'Original opcode mismatch {va:#x}')
 for va,target in CALLS.items():
  op=word(va);dest=((va+4)&0xf0000000)|((op&0x3ffffff)<<2)
  if op>>26!=3 or dest!=target:raise ValueError(f'Original JAL mismatch {va:#x}')
 for va,target in JUMPS.items():
  op=word(va);dest=((va+4)&0xf0000000)|((op&0x3ffffff)<<2)
  if op>>26!=2 or dest!=target:raise ValueError(f'Original jump mismatch {va:#x}')
 for va,target in BRANCHES.items():
  imm=struct.unpack('<h',struct.pack('<H',word(va)&65535))[0]
  if va+4+4*imm!=target:raise ValueError(f'Original branch mismatch {va:#x}')
 return {'elf_sha256':ELF_SHA,'verified_machine_regions':len(REGIONS),
   'original_instruction_words':sum((hi-lo)//4 for lo,hi in REGIONS),
   'independent_opcode_anchors':len(OPCODES),'source_JAL_calls':len(CALLS),
   'source_J_tailcalls':len(JUMPS),'source_branches':len(BRANCHES),
   'constructors':['0x57BA98','0x57BAF8'],
   'child_methods':['0x57BC0C','0x57BC54','0x57BC88'],
   'destructors':['0x57BB60','0x57BBB4'],
   'unresolved':'0x57BCB8 vector actor movement/controller update',
   'windows_game_booted':False}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('original_elf',type=Path);args=p.parse_args()
 print(json.dumps(verify(args.original_elf),indent=2))
