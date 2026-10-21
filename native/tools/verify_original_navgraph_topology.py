#!/usr/bin/env python3
"""Static audit of original AE3 USA NavGraph adjacency and traversal.

Original executable required privately; this script publishes hashes and source
addresses only. Does not emulate PS2 or include any copyrighted bytes.
"""
import argparse,hashlib,json,struct
from pathlib import Path
PIN='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
RANGES=[(0x21b3f8,0x21b458),(0x21e648,0x21e70c),(0x4f0fbc,0x4f1028)]
ANCHORS={
0x21b3f8:0x8c8f0030,0x21b400:0x95e20002,
0x21b410:0x8c8f0030,0x21b418:0x95ef0002,
0x21b41c:0x00af282a,0x21b428:0x95c50004,
0x21b42c:0x8c840000,0x21b438:0x0000102d,
0x21e670:0x0c087626,0x21e678:0x54400007,
0x21e684:0x0c08768b,0x21e698:0x95cf0002,
0x21e69c:0x25ef0001,0x21e6ac:0x0000802d,
0x21e6bc:0x0c0876d9,0x21e6c8:0x0c086d01,
0x21e6d0:0x26100001,0x21e6d4:0x968f0002,
0x21e6e4:0x15e0fff4,0x4f0fec:0x0c10b113,
0x4f1008:0x0c10b113,0x4f1010:0x240f0002,
0x4f0af8:0x25ef7b80,0x22d8c8:0x0c13c297}
CALLS={0x21b430:0x21d884,0x21e670:0x21d898,
       0x21e684:0x21da2c,0x21e6bc:0x21db64,
       0x21e6c8:0x21b404,0x22d8c8:0x4f0a5c,
       0x4f0fec:0x42c44c}
STRINGS={0x6e7398:b'navigation\0',0x6e7470:b'nav_monkey.nav\0',
         0x6e7480:b'nav_enemy.nav\0',0x6e7490:b'nav_extra0.nav\0'}
def audit(path:Path):
 data=path.read_bytes()
 if hashlib.sha256(data).hexdigest()!=PIN:
  raise ValueError('Expected original owner-private SCUS_975.01 USA ELF')
 if data[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',data,18)[0]!=8:
  raise ValueError('Expected little-endian MIPS ELF32')
 ph=struct.unpack_from('<I',data,28)[0];step,n=struct.unpack_from('<HH',data,42);segments=[]
 for i in range(n):
  typ,off,va,_,size,_,_,_=struct.unpack_from('<8I',data,ph+step*i)
  if typ==1 and off+size<=len(data):segments.append((va,va+size,off))
 def get(va,size):
  for start,end,off in segments:
   if start<=va and va+size<=end:return data[off+va-start:off+va-start+size]
  raise ValueError('Unmapped source ELF address '+hex(va))
 def word(va):return struct.unpack('<I',get(va,4))[0]
 for va,expected in ANCHORS.items():
  if word(va)!=expected:raise ValueError('Retail MIPS instruction differs '+hex(va))
 for va,target in CALLS.items():
  w=word(va);dest=((va+4)&0xf0000000)|((w&0x3ffffff)<<2)
  if w>>26 not in (2,3) or dest!=target:
   raise ValueError('Retail J/JAL branch differs '+hex(va))
 for va,string in STRINGS.items():
  if get(va,len(string))!=string:raise ValueError('Original embedded NavGraph name mismatch '+hex(va))
 return {'source_elf_sha256':PIN,'original_source_regions':[
  {'start':hex(a),'stop':hex(b),'sha256':hashlib.sha256(get(a,b-a)).hexdigest(),
   'instruction_words':(b-a)//4} for a,b in RANGES],
   'original_MIPS_opcode_anchors':len(ANCHORS),
   'original_direct_J_JAL_targets':len(CALLS),
   'methods':['0x21b3f8','0x21b404','0x21e648','0x4f0fbc'],
   'original_nav_source_name':'navigation',
   'untranslated_original_routine':'0x21db64',
   'gameplay_complete':False}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('original_elf',type=Path);p.add_argument('--out',type=Path)
 args=p.parse_args();report=audit(args.original_elf)
 result=json.dumps(report,indent=2)+'\n'
 if args.out:args.out.write_text(result)
 else:print(result,end='')
 print('verified',sum(r['instruction_words'] for r in report['original_source_regions']),
       'retail original instructions')
