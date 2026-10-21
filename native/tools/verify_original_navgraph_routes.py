#!/usr/bin/env python3
"""Audit authentic Ape Escape 3 USA NavGraph dispatch against owner's ELF.

Only original function metadata/hashes: no executable bytes, navigation files,
or game assets are committed or reproduced.
"""
import argparse,hashlib,json,struct
from pathlib import Path
SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
WINDOWS={
 (0x4f0a5c,0x4f0b2c):'514e055e8654013cccf4bd74e0a74c756de09e6dee3be5f55664b083ab3d4586',
 (0x4f0fbc,0x4f1028):'41d3961d04fb8574db10c142f464054e1265ec7cbc4e68d517f2898908069ddc',
}
SITES={
 0x4f0a5c:0x27bdffc0,
 0x4f0ae0:0x0c09b485,
 0x4f0ae8:0x0c13c3ef,
 0x4f0af8:0x25ef7b80,
 0x4f0b00:0x004f1021,
 0x4f0b04:0x0220202d,
 0x4f0b08:0x0c09b485,
 0x4f0b10:0x8c450008,
 0x4f0b14:0x0c08751b,
 0x4f0b18:0x0200202d,
 0x4f0fbc:0x27bdfff0,
 0x4f0fd0:0x0c10b113,
 0x4f0fe4:0x1040000b,
 0x4f0fec:0x0c10b113,
 0x4f1008:0x0c10b113,
 0x4f1010:0x240f0002,
}
CALLS={
 0x4f0ae8:0x4f0fbc,
 0x4f0b14:0x21d46c,
 0x4f0fd0:0x42c44c,
 0x4f0fec:0x42c44c,
 0x4f1008:0x42c44c,
}
STRINGS={
 0x6e7470:b'nav_monkey.nav\x00',
 0x6e7480:b'nav_enemy.nav\x00',
 0x6e7490:b'nav_extra0.nav\x00',
 0x6e3e40:b'void NavGraph::load(const void*)\x00',
 0x6e3e68:b'NavGraph::naviList\x00',
}
def verify(path:Path):
 data=path.read_bytes()
 if hashlib.sha256(data).hexdigest()!=SHA:
  raise ValueError('Wrong original USA SCUS_975.01 hash')
 if data[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',data,18)[0]!=8:
  raise ValueError('Expected MIPS little-endian ELF32')
 ph=struct.unpack_from('<I',data,28)[0]
 step,n=struct.unpack_from('<HH',data,42)
 seg=[]
 for i in range(n):
  kind,off,va,_,size,_,_,_=struct.unpack_from('<8I',data,ph+i*step)
  if kind==1 and off+size<=len(data):seg.append((va,va+size,off))
 def read(va,size):
  for lo,hi,off in seg:
   if lo<=va and va+size<=hi:return data[off+va-lo:off+va-lo+size]
  raise ValueError('Unmapped original ELF address '+hex(va))
 def word(va):return struct.unpack('<I',read(va,4))[0]
 for (start,end),digest in WINDOWS.items():
  if hashlib.sha256(read(start,end-start)).hexdigest()!=digest:
   raise ValueError('Original function instruction region changed: '+hex(start))
 for site,expected in SITES.items():
  if word(site)!=expected:raise ValueError('Original opcode changed: '+hex(site))
 for pc,target in CALLS.items():
  w=word(pc);decoded=((pc+4)&0xf0000000)|((w&0x3ffffff)<<2)
  if w>>26 not in (2,3) or decoded!=target:
   raise ValueError('Original direct function call changed at '+hex(pc))
 for va,text in STRINGS.items():
  if read(va,len(text))!=text:
   raise ValueError('Original NavGraph name/string differs at '+hex(va))
 return {
  'format':'ae3/original-NavGraph-loader-routes/v1',
  'original_elf_sha256':SHA,
  'verified_original_code_regions':len(WINDOWS),
  'original_instructions_in_regions':sum((b-a)//4 for a,b in WINDOWS),
  'independently_checked_instruction_words':len(SITES),
  'verified_direct_original_calls':len(CALLS),
  'original_resource_names':[v[:-1].decode() for k,v in STRINGS.items() if k in (0x6e7470,0x6e7480,0x6e7490)],
  'original_slot_table_global':'0x647b80 (+0,+4,+8)',
  'original_dispatch':'0x4f0fbc',
  'original_load':'0x4f0a5c',
  'original_source_loader':'0x21d46c',
  'limitation':'Authentic file/route recovery, original update/render/gameplay still incomplete'}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('original_elf',type=Path);p.add_argument('--out',type=Path)
 a=p.parse_args();v=verify(a.original_elf)
 result=json.dumps(v,indent=2)+'\n'
 if a.out:a.out.write_text(result,encoding='utf-8')
 else:print(result,end='')
 print('PASS:',v['original_instructions_in_regions'],'original NavGraph instructions checked')
