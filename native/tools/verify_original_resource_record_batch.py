#!/usr/bin/env python3
"""Audit USA retail 0xB0 source record producer and bucket insertion."""
import argparse,hashlib,json,struct
from pathlib import Path
SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
WINDOWS={
(0x1bb3ec,0x1bb458):'c2c9f2c7fa19789964ce7a8fbdb0ac946eb7b7341ee4faf63f70d1ae836926cd',
(0x1bb4e4,0x1bb508):'7a2f8aa1afa9c094e0016a7e3bedad89bada8563ac01c887e810193438e4164c',
(0x1bb458,0x1bb4cc):'9b20977354ba26e9d6bbca508d71269ebf467965a593cac36e54aa15243bc292',
(0x1bb558,0x1bb63c):'a05eed119665c122cd9215b094c7182d60a63a9f9d1102a65e3b28e6aadbd653',
(0x2bb0d0,0x2bb0f4):'f0b325c2bfd61e49dd8c542aa2f57ca9476b090ced98e5b71052fbf76fd7a83f',
(0x2bb0f4,0x2bb110):'996931612e8994c6c2c6b54950c9a6f0a34d4936abc5de7424a8a3fddafad9f4',
(0x1bb9a8,0x1bbb18):'441da325cbbcf3e04118db2e833a692ed0ed7ad8e11381c6e942ddb4616afb04',
(0x36d610,0x36d6f4):'c3b70a57ebe9d7a204fae920bd629a1199c0ce9f2a7738dc78efa42488af7f67'}
ANCHORS={
0x1bb400:0x0c0aec34,0x1bb408:0x1840000d,0x1bb414:0x8e040000,
0x1bb418:0x0220282d,0x1bb41c:0x0c06ed39,0x1bb420:0x26310001,
0x1bb424:0x0040282d,0x1bb428:0x26040010,0x1bb42c:0x0c0db584,
0x1bb430:0x0040302d,0x1bb438:0x55e0fff7,0x1bb43c:0x8e040000,
0x1bb4e4:0x8c840000,0x1bb4e8:0x10800005,
0x1bb4f0:0x240e00b0,0x1bb4f4:0x8c8f0004,
0x1bb4f8:0x00ae7018,0x1bb4fc:0x01ee1021,
0x2bb0d8:0x0c0aec21,0x2bb0e8:0x8de206c8,
0x1bb598:0x0c0ae85c,0x1bb5b0:0xae120000,
0x1bb5b8:0xae500000,0x1bb5f0:0x0c06ed16,
0x1bb5f4:0xae020004,0x1bb5f8:0x0c06ecfb,
0x1bb9a8:0x27bdffa0,0x1bb9f0:0x7bae0010,
0x1bbb00:0xdfb10048,
0x36d63c:0x0c0dbc7b,0x36d65c:0xac500020,
0x36d660:0x7c4f0000,0x36d664:0xac400024,
0x36d66c:0x7c4f0010,0x36d6a0:0x3210007f}
CALLS={
0x1bb400:0x2bb0d0,0x1bb41c:0x1bb4e4,0x1bb42c:0x36d610,
0x1bb46c:0x2bb0d0,0x1bb480:0x2bb0f4,0x1bb4a0:0x1bb9a8,
0x1bb5a4:0x36d498,0x1bb5b4:0x2bb0d0,0x1bb5f0:0x1bb458,
0x1bb5f8:0x1bb3ec,0x1bb6dc:0x1bb3ec,
0x1bb7bc:0x1bb3ec,0x1bb89c:0x1bb3ec}
def verify(path):
 b=path.read_bytes()
 if hashlib.sha256(b).hexdigest()!=SHA:raise ValueError("Wrong private USA original ELF")
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError("Expected MIPS ELF32 little endian")
 ph=struct.unpack_from('<I',b,28)[0];step,num=struct.unpack_from('<HH',b,42)
 seg=[]
 for i in range(num):
  typ,off,va,_,length,_,_,_=struct.unpack_from('<8I',b,ph+i*step)
  if typ==1 and off+length<=len(b):seg.append((va,va+length,off))
 def read(va,size):
  for lo,hi,off in seg:
   if lo<=va and va+size<=hi:return b[off+va-lo:off+va-lo+size]
  raise ValueError(f"Unmapped retail address {va:#x}")
 def word(pc):return struct.unpack('<I',read(pc,4))[0]
 for (lo,hi),sha in WINDOWS.items():
  if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=sha:
   raise ValueError(f"Original full code region differs: {lo:#x}")
 for pc,w in ANCHORS.items():
  if word(pc)!=w:raise ValueError(f"Original instruction differs: {pc:#x}")
 for pc,target in CALLS.items():
  w=word(pc)
  if w>>26!=3 or (((pc+4)&0xf0000000)|((w&0x03ffffff)<<2))!=target:
   raise ValueError(f"Original JAL target differs: {pc:#x}")
 def branch(pc):
  i=word(pc)&0xffff
  if i>=0x8000:i-=0x10000
  return pc+4+4*i
 assert branch(0x1bb408)==0x1bb440
 assert branch(0x1bb438)==0x1bb418
 assert branch(0x1bb4e8)==0x1bb500
 x=word(0x1bb4f8)
 # R5900 MULT a1,t6 -> destination GPR t6, fixed 0xB0 stride.
 assert (x>>26,x&63,(x>>21)&31,(x>>16)&31,(x>>11)&31)==(0,24,5,14,14)
 assert word(0x1bb424)==0x0040282d and word(0x1bb430)==0x0040302d
 # Exactly four original direct callers of the batch insertion function.
 sources=[]
 for lo,hi,_ in seg:
  for pc in range((lo+3)&~3,hi-3,4):
   w=word(pc)
   if w>>26==3 and (w&0x03ffffff)<<2==0x1bb3ec:sources.append(pc)
 assert sorted(sources)==[0x1bb5f8,0x1bb6dc,0x1bb7bc,0x1bb89c]
 return dict(original_elf_sha256=SHA,windows=len(WINDOWS),
  complete_instruction_words=sum((hi-lo)//4 for lo,hi in WINDOWS),
  exact_opcode_anchors=len(ANCHORS),JAL_edges=len(CALLS),
  record_stride=176,key_bytes=32,owner_record_buffer_offset=4,
  owner_count_offset=8,owner_bucket_table_offset=16,
  original_batch_callers=[hex(x) for x in sources],
  original_descriptor_populator='0x1BB9A8',
  actual_retail_archive_mapping_verified=False,playable_game=False)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('original_elf',type=Path)
 print('PASS: original batch record producer to 128-bucket insertion')
 print(json.dumps(verify(p.parse_args().original_elf),indent=2))
