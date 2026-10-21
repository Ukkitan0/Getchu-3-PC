#!/usr/bin/env python3
"""Original USA AE3 navigation PCK and executable dispatch audit; private inputs."""
import argparse,hashlib,struct
from pathlib import Path
ELF='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
PCK='d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0'
REGIONS={
(0x4f0a5c,0x4f0b2c):'514e055e8654013cccf4bd74e0a74c756de09e6dee3be5f55664b083ab3d4586',
(0x4f0fbc,0x4f1028):'41d3961d04fb8574db10c142f464054e1265ec7cbc4e68d517f2898908069ddc',
(0x26df38,0x26df50):'1f7d4a0f338b0a7d04366122d34d6c429ce01828b2cf08441b021fc4a4f2c400',
(0x26d24c,0x26d35c):'dcb06b243e56d8f481c5af26b6c6b6c843b76833b4599a660c00518f8c716a3e',
(0x21d46c,0x21d7d8):'e95e0a6ec99aa6b9841de0339736d8473083dd0ca41a3334d60e127826a37102'}
CALLS={0x4f0a74:0x26dd4c,0x4f0a7c:0x26d030,0x4f0a94:0x26df38,
0x4f0a9c:0x26d120,0x4f0abc:0x26d0d0,0x4f0ac4:0x26ddc8,
0x4f0ae0:0x26d214,0x4f0ae8:0x4f0fbc,0x4f0b08:0x26d214,
0x4f0b14:0x21d46c,0x4f0fd0:0x42c44c,0x4f0fec:0x42c44c,
0x4f1008:0x42c44c,0x26d2bc:0x42d148,0x26d310:0x26d138}
def audit(exe,pck):
 b=Path(exe).read_bytes();p=Path(pck).read_bytes()
 if hashlib.sha256(b).hexdigest()!=ELF or hashlib.sha256(p).hexdigest()!=PCK:
  raise ValueError('Incorrect original ELF or navigation.pck')
 # Original SHA-pinned executable LOAD maps text VA 0x100000 at file offset 4096.
 def read(a,n):return b[4096+a-0x100000:4096+a-0x100000+n]
 def word(a):return struct.unpack('<I',read(a,4))[0]
 for (start,end),sha in REGIONS.items():
  if hashlib.sha256(read(start,end-start)).hexdigest()!=sha:raise ValueError('Original code changed')
 for at,target in CALLS.items():
  w=word(at)
  if w>>26!=3 or ((w&0x3ffffff)<<2)!=target:raise ValueError('Original call changed')
 assert ((word(0x26df48)&0x3ffffff)<<2)==0x26d24c
 assert read(0x6e7430,4)==b'nav\0'
 for a,n in [(0x6e7470,b'nav_monkey.nav\0'),(0x6e7480,b'nav_enemy.nav\0')]:
  assert read(a,len(n))==n
 assert p[:4]==b'PCK\0' and struct.unpack_from('<II',p,4)==(64,2)
 total=0
 for i in range(2):
  name,type_,off,size=struct.unpack_from('<4I',p,64+16*i)
  assert p[name:p.index(0,name,64)].decode()==['nav_monkey.nav','nav_enemy.nav'][i]
  assert p[type_:p.index(0,type_,64)]==b'nav'
  assert (off,size)==[(96,2128),(2224,1104)][i]
  data=p[off:off+size];count=struct.unpack_from('<I',data,0x24)[0]
  offs=struct.unpack_from('<6I',data,0x38)
  assert count==[39,20][i]
  assert offs==[(96,208,1392,2064,2096,2128),(96,144,752,1072,1072,1104)][i]
  for j in range(count):
   at=offs[1]+30*j
   assert at+30<=size and struct.unpack_from('<H',data,at+22)[0]==3
   for v in struct.unpack_from('<3H',data,at+24):assert offs[2]+16*(v+1)<=size
  total+=count
 assert total==59
 return {'original_instructions':372,'verified_calls':len(CALLS),
         'retail_nav_polygons':total,'runtime_VAs_from_PCK':False}
if __name__=='__main__':
 q=argparse.ArgumentParser();q.add_argument('elf',type=Path);q.add_argument('pck',type=Path)
 a=q.parse_args();print('PASS',audit(a.elf,a.pck))
