#!/usr/bin/env python3
"""Verify recovered original Ape Escape 3 USA route-search instructions.

Requires owner's exact SCUS_975.01 ELF. Only structural metadata and
instruction hashes are produced; commercial code and data remain private.
"""
import argparse,hashlib,json,struct
from pathlib import Path
PIN='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
WINDOWS={
 (0x220994,0x220a4c):'417764b1fd072cd69df9fc4242f721521fc731498f766d53542cb45dcc392483',
 (0x220c00,0x220d14):'a20d4eafc3ab9dabb26fa25897f66238cfb651e6d0090e5b0af8aaac34ebfd64',
 (0x220d14,0x220d5c):'bab94040c18987ab0db96285a376580b18c4febfa76d55f3bfe19f1794ed9cb0',
 (0x220d5c,0x220f60):'5a7b8d205645ff8f55d9d42748959eb023a9186048becffd5bbd5c15f7025b7b',
 (0x220f60,0x221030):'ce1ecf130e36912ca0b8318dd39477c90f406a1988c86adf4a6c1839b9f7a63a',
 (0x221030,0x221100):'78b55fd574903116a966b8def43534ae75183ea88762aa83dcdbf67f57e33d77',
}
ANCHORS={
 0x2209b4:0x50af000c,0x2209c0:0x25ef0001,
 0x2209e8:0x0c106e2c,0x220a28:0x8de50030,
 0x220c24:0x8c8f000c,0x220c48:0x0c086cfe,
 0x220c98:0x0c086d01,0x220ca4:0x0c088345,
 0x220cdc:0xade2000c,0x220ce0:0x0c086d11,
 0x220cec:0xae020010,0x220cfc:0xaded0008,
 0x220d08:0xadf40024,0x220d14:0x8c8c000c,
 0x220d18:0x2402ffff,0x220d34:0x8def0004,
 0x220d38:0x55e50003,0x220d88:0x8c8e0010,
 0x220d90:0xadc00034,0x220dc4:0x91cf0000,
 0x220dd0:0xc5c10034,0x220de0:0x46000d06,
 0x220e28:0x8e040004,0x220e30:0xa20f0000,
 0x220e5c:0x922f0000,0x220e6c:0x8dee0018,
 0x220e74:0x11cf0028,0x220e84:0x11c00010,
 0x220e94:0x4600a500,0x220e98:0x4601a034,
 0x220ea4:0xe6340034,0x220ea8:0xae350030,
 0x220ec8:0x0c086d1a,0x220f08:0x55cfffe1,
 0x220f20:0x0c086d38,0x220f28:0x0c086c65,
 0x220fe8:0x46140034,0x2210bc:0x4600a034,
 0x6e47e8:0x7f7fffff,0x6e47ec:0x7f7fffff,
 0x6e47f0:0x7f7fffff,
}
CALLS={
 0x220c48:0x21b3f8,0x220c98:0x21b404,
 0x220ca4:0x220d14,0x220ce0:0x21b444,
 0x220e2c:0x21b468,0x220ec8:0x21b468,
 0x220f20:0x21b4e0,0x220f28:0x21b194,
 0x220fb0:0x21b468,0x221084:0x21b468,
 0x2209e8:0x41b8b0,
}
def verify(path):
 data=path.read_bytes()
 if hashlib.sha256(data).hexdigest()!=PIN:
  raise ValueError('Wrong owner-private original USA SCUS_975.01 ELF')
 if data[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',data,18)[0]!=8:
  raise ValueError('Expected MIPS little-endian ELF32')
 ph=struct.unpack_from('<I',data,28)[0]
 step,count=struct.unpack_from('<HH',data,42)
 segments=[]
 for i in range(count):
  typ,off,va,_,size,_,_,_=struct.unpack_from('<8I',data,ph+i*step)
  if typ==1 and off+size<=len(data):segments.append((va,va+size,off))
 def read(va,size):
  for start,end,offset in segments:
   if start<=va and va+size<=end:
    return data[offset+va-start:offset+va-start+size]
  raise ValueError('Unmapped original address '+hex(va))
 def word(va):return struct.unpack('<I',read(va,4))[0]
 for (start,end),digest in WINDOWS.items():
  if hashlib.sha256(read(start,end-start)).hexdigest()!=digest:
   raise ValueError('Original route-search opcode region differs '+hex(start))
 for va,w in ANCHORS.items():
  if word(va)!=w:raise ValueError('Source opcode mismatch '+hex(va))
 for pc,target in CALLS.items():
  w=word(pc)
  decoded=((pc+4)&0xf0000000)|((w&0x03ffffff)<<2)
  if w>>26 not in (2,3) or decoded!=target:
   raise ValueError('Original source call differs '+hex(pc))
 return {
  'format':'ae3/verified-original-navigation-route-search/v1',
  'original_elf_sha256':PIN,
  'original_code_regions':len(WINDOWS),
  'source_instruction_words':sum((b-a)//4 for a,b in WINDOWS),
  'exact_independent_opcode_sites':len(ANCHORS),
  'original_direct_J_JAL_sites':len(CALLS),
  'translated_methods':['0x220994','0x220c00','0x220d14',
                        '0x220d5c','0x220f60','0x221030'],
  'source_search_record_stride':56,
  'source_record_fields':{
   'visited':0,'polygon_pointer':4,'outgoing_count':8,
   'destination_indices':12,'edge_types':24,'source_edge_slots':36,
   'predecessor':48,'accumulated_cost':52},
  'unresolved':['0x220aa4 original graph admission/filtering',
       'type-1 source spline cost at 0x21b4e0/0x21b194',
       'live actor controller route execution'],
  'correction':'0x21db64 submits debug navigation geometry, not this shortest path relaxation routine',
  'full_game_playable':False}
if __name__=='__main__':
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument('original_elf',type=Path)
 ap.add_argument('--out',type=Path)
 args=ap.parse_args()
 report=verify(args.original_elf)
 result=json.dumps(report,indent=2)+'\n'
 if args.out:args.out.write_text(result,encoding='utf-8')
 else:print(result,end='')
 print('PASS original navigation route search:',report['source_instruction_words'],'instructions')
