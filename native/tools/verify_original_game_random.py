#!/usr/bin/env python3
"""Audit original USA SCUS_975.01 game RNG against private executable."""
import argparse,hashlib,json,struct
from pathlib import Path
ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
REGIONS={
 (0x36f2dc,0x36f368):'dc0ac996d3183029d3515a70f448f474b961b7e7b3209411f8bbbc5fe04c53dd',
 (0x36f3c0,0x36f444):'459692193799826a13a6eb0ced367a60b2715c49bcd0d845947371b473862a5f',
 (0x36f444,0x36f584):'de327aaf6c7ef30bc65806e7d8b4bc75a455fee662b1800e24b8c8537b50f33d'}
ANCHORS={
0x36f2dc:0x8c8c09c0,0x36f2e4:0x25890001,0x36f2ec:0x2d8f00e3,
0x36f2f8:0x8dae0004,0x36f2fc:0x8daa0634,0x36f300:0x8daf0000,
0x36f304:0x3c0d9908,0x36f308:0xac8909c0,0x36f310:0x35adb0df,
0x36f338:0x014f7826,0x36f340:0xad8f0000,
0x36f3c4:0xac850000,0x36f3dc:0x240c0001,
0x36f3e4:0x3c0d6c07,0x36f3e8:0x35ad8965,
0x36f408:0x15e0fff5,0x36f410:0xac8009c0,
0x36f414:0x0080882d,0x36f418:0x2410026f,
0x36f420:0x0c0dbcb7,0x36f444:0x27bdfff0,
0x36f450:0x8c8f09c0,0x36f468:0xdfbf0008,
0x36f470:0x3c0e9d2c,0x36f474:0x35ce5680,
0x36f484:0x3c0eefc6,0x36f494:0x00107c82,
0x36f4b4:0x0c0dbd11,0x36f4bc:0x3c0f7fff,
0x36f4c4:0x35efffff,0x36f4c8:0x004f1024,
0x36f4dc:0x0c0dbd2b,0x36f4f4:0x46800020,
0x36f4f8:0xc5e10000,0x36f4fc:0x46010002,
0x36f524:0x02118023,0x36f528:0x0050001a,
0x36f530:0x00001010,0x36f538:0x00511021,
0x36f560:0x0c0dbd35,0x36f564:0x4615a501,
0x36f56c:0x46140002,0x36f570:0x46150000}
CALLS={0x36f420:0x36f2dc,0x36f45c:0x36f2dc,
       0x36f4b4:0x36f444,0x36f4dc:0x36f4ac,
       0x36f51c:0x36f4ac,0x36f560:0x36f4d4}
def verify(path):
 b=Path(path).read_bytes()
 if hashlib.sha256(b).hexdigest()!=ELF_SHA:raise ValueError('Retail ELF SHA mismatch')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Expected owner-original MIPS ELF32')
 ph=struct.unpack_from('<I',b,28)[0]
 entry_size,count=struct.unpack_from('<HH',b,42)
 segments=[]
 for i in range(count):
  typ,offset,va,_,size,_,_,_=struct.unpack_from('<8I',b,ph+entry_size*i)
  if typ==1 and offset+size<=len(b):segments.append((va,va+size,offset))
 def word(a):
  for low,high,offset in segments:
   if low<=a and a+4<=high:
    return struct.unpack_from('<I',b,offset+a-low)[0]
  raise ValueError(f'Original address {a:#x} unmapped')
 for (lo,hi),digest in REGIONS.items():
  data=b''
  for a in range(lo,hi,4):data+=struct.pack('<I',word(a))
  if hashlib.sha256(data).hexdigest()!=digest:
   raise ValueError(f'Original code region differs {lo:#x}')
 for a,w in ANCHORS.items():
  if word(a)!=w:raise ValueError(f'Original RNG opcode mismatch {a:#x}')
 for a,dest in CALLS.items():
  ins=word(a)
  if ins>>26!=3 or (((a+4)&0xf0000000)|((ins&0x3ffffff)<<2))!=dest:
   raise ValueError(f'Original RNG JAL differs {a:#x}')
 return {'owner_elf_sha256':ELF_SHA,
  'whole_original_regions_checked':len(REGIONS),
  'original_instruction_words_audited':sum((hi-lo)//4 for lo,hi in REGIONS),
  'individual_opcode_anchors':len(ANCHORS),'call_edges':len(CALLS),
  'MT_state_bytes':2496,'warmup_steps':623,
  'retail_default_seed':5489,'float_scale_bits':'0x30000000'}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('original_elf',type=Path)
 a=p.parse_args()
 print(json.dumps(verify(a.original_elf),indent=2))
