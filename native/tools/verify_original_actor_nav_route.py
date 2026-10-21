#!/usr/bin/env python3
"""Verify the original AE3 USA actor-side route consumers against owned ELF.

Reads private originals; outputs only code metadata, never retail bytes.
"""
import argparse, hashlib, json, struct
from pathlib import Path
ELF_SHA = '5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
NAV_SHA = 'd2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0'
REGIONS = {
 (0x221104,0x221180):'8c687b07222b7ab578ed09fc6c41baaf74de9894f1da4903b0c45fece3fac64b',
 (0x21b404,0x21b458):'2905980f023006a99a1815acf00b2f82a5e0b3838787378e0961bd1107b051f4',
 (0x575ec0,0x575f18):'4a234fd057264a52058cb522d5756e54582a42c0a78e2a6bf351130652dc4aff',
 (0x57bd88,0x57bdd4):'22d39430744064924f386c1a79c18834a15df607fdf6e59f0a8462bad2e70dd0',
 (0x317538,0x31759c):'6c9e46bfd0471d53b7c1b46fd5f368eb3c51f30d633859838fd1ab3d72571e5b',
 (0x31759c,0x317610):'21da46b01878420b03c6e2bd56c1dffe034190203acd63d5a3338d76ef2d9acd',
 (0x57ba98,0x57baf8):'3ea68c642ee77c6c1391094f98efd746c695c9d09d1998bdb0f840d3457a07c9',
 (0x57baf8,0x57bb60):'79aebf766d27e20af0f61640a3186d03c514f6329e260d1b5a1039fbb6616f0d',
}
ANCHORS = {
 0x221128:0x8c8e0000,0x221134:0x8cb20000,0x221138:0x8df10000,
 0x22114c:0x10510005,0x221150:0x26100001,0x221154:0x2a0f0003,
 0x221158:0x15e0fff9,0x221160:0x0000782d,
 0x21b410:0x8c8f0030,0x21b418:0x95ef0002,0x21b420:0x10a00005,
 0x21b428:0x95c50004,0x21b444:0x8c8f0030,0x21b454:0x95e2000a,
 0x575ed8:0x8c900000,0x575edc:0x10b00008,0x575ee4:0x52000006,
 0x575efc:0xae510000,0x57bda4:0x38420002,0x57bda8:0x2c420001,
 0x31755c:0x0c106e00,0x317574:0x0c15eea6,
 0x3175c8:0x0c106e00,0x3175e4:0x0c15eebe,
 0x57bad8:0x29ad0002,0x57bb30:0xa08f0030,
}
CALLS = {
 0x221140:0x21b404,0x57bd90:0x221104,0x57bd9c:0x21b444,
 0x575eec:0x220a78,0x575ef4:0x41a3b0,
 0x31755c:0x41b800,0x317574:0x57ba98,
 0x3175c8:0x41b800,0x3175e4:0x57baf8,
 0x570d80:0x2207d4,0x574e60:0x2207d4,
 0x575220:0x2207d4,0x5756f8:0x2207d4,
 0x575918:0x2207d4,0x575924:0x575ec0,
}
BRANCHES = {
 0x22114c:0x221164,0x221158:0x221140,
 0x575edc:0x575f00,0x575ee4:0x575f00,
}
def verify(elf_path,nav_path):
 data=Path(elf_path).read_bytes(); nav=Path(nav_path).read_bytes()
 if hashlib.sha256(data).hexdigest()!=ELF_SHA:raise ValueError('Original USA SCUS_975.01 SHA mismatch')
 if hashlib.sha256(nav).hexdigest()!=NAV_SHA:raise ValueError('Original woods_c/navigation.pck SHA mismatch')
 if data[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',data,18)[0]!=8:
  raise ValueError('Expected ELF32 little-endian MIPS retail source')
 ph=struct.unpack_from('<I',data,28)[0];step,n=struct.unpack_from('<HH',data,42)
 segments=[]
 for i in range(n):
  kind,offset,va,_,size,_,_,_=struct.unpack_from('<8I',data,ph+step*i)
  if kind==1 and offset+size<=len(data):segments.append((va,va+size,offset))
 def read(addr,n):
  for lo,hi,off in segments:
   if lo<=addr and addr+n<=hi:return data[off+addr-lo:off+addr-lo+n]
  raise ValueError('Unmapped source executable address '+hex(addr))
 def word(a):return struct.unpack('<I',read(a,4))[0]
 for (lo,hi),digest in REGIONS.items():
  if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=digest:raise ValueError('Source region differs '+hex(lo))
 for a,w in ANCHORS.items():
  if word(a)!=w:raise ValueError(f'Source opcode mismatch {a:#x}')
 for a,target in CALLS.items():
  inst=word(a)
  if inst>>26 !=3 or ((a+4)&0xf0000000)|((inst&0x3ffffff)<<2)!=target:
   raise ValueError(f'Source call mismatch {a:#x} -> {target:#x}')
 for a,target in BRANCHES.items():
  immediate=struct.unpack('<h',struct.pack('<H',word(a)&0xffff))[0]
  if a+4+4*immediate!=target:raise ValueError('Source branch mismatch '+hex(a))
 if nav[:4]!=b'PCK\x00' or struct.unpack_from('<II',nav,4)!=(64,2) or len(nav)!=3328:
  raise ValueError('Original woods_c PCK header/layout mismatch')
 expected=[39,20]; polygons=[]; links=[]; kinds=[]
 for i,(start,size) in enumerate(((96,2128),(2224,1104))):
  raw=nav[start:start+size]
  if len(raw)!=size:raise ValueError('Invalid woods_c package member size')
  count,base=struct.unpack_from('<II',raw,0x24)[0],struct.unpack_from('<I',raw,0x3c)[0]
  if count!=expected[i] or base+count*0x1e>size:raise ValueError('Original polygon descriptor mismatch')
  polygons.append(count); num=0; special=0
  for v in range(count):
   at=base+v*0x1e
   degree=struct.unpack_from('<H',raw,at+2)[0]
   if degree>3:raise ValueError('Unexpected polygon degree')
   for j in range(degree):
    dest=struct.unpack_from('<H',raw,at+4+j*2)[0]
    kind=struct.unpack_from('<H',raw,at+0x0a+j*2)[0]
    if dest>=count or kind>2:raise ValueError('Unexpected retail source edge')
    num+=1;special+=(kind==2)
  links.append(num);kinds.append(special)
 return {'original_elf_sha256':ELF_SHA,'original_nav_sha256':NAV_SHA,
    'complete_machine_regions':len(REGIONS),
    'audited_instruction_words':sum((hi-lo)//4 for lo,hi in REGIONS),
    'independent_opcode_anchors':len(ANCHORS),'direct_source_call_edges':len(CALLS),
    'source_branches':len(BRANCHES),'original_direct_query_callers':5,
    'retail_woods_c_polygons':polygons,'retail_woods_c_edges':links,
    'retail_woods_c_kind2_edges':kinds,'original_windows_game_booted':False}
if __name__=='__main__':
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument('original_elf',type=Path);ap.add_argument('original_woods_c_navigation_pck',type=Path)
 ns=ap.parse_args();print(json.dumps(verify(ns.original_elf,ns.original_woods_c_navigation_pck),indent=2))
