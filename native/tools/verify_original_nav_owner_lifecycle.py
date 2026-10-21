#!/usr/bin/env python3
"""Verify retail USA nav graph owner allocation / teardown lifecycle."""
import argparse,hashlib,json,struct
from pathlib import Path
SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
PCK='d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0'
REGIONS=[(0x21af94,0x21afa8),(0x21d3d0,0x21d40c),(0x21d43c,0x21d46c),
(0x21d7d8,0x21d884),(0x21ed30,0x21ed94),(0x21ed94,0x21ee00),
(0x21b384,0x21b3f8),(0x22d8b0,0x22d8e0),(0x22e064,0x22e080)]
WINDOW_SHA={
 '21af94-21afa8':'edfb9e32fd4410bb123e4d4bd877d74991265edfda8d2e372b95e81fe69fb8b8',
 '21d3d0-21d40c':'b1fce307a9f5cc38edadd48059d7611b8cedb465449f3e1023bcc4b04487ca43',
 '21d43c-21d46c':'e41d92bc996ee048184031b3838ed5a0ecbe1a6fdc0bf03380bc917d50660095',
 '21d7d8-21d884':'07154907e37686d7e98c418420e28f483869d8f4e411eecaed1be6bdb059bbcf',
 '21ed30-21ed94':'753a36732cb2cc9fb4472feb395c6c05c74baf7a0946b3f4e325574a522285b0',
 '21ed94-21ee00':'bfb2e5cc32cf4820f8804ad34e3e38d0da617c4201d72a73e6ade1c4e65754ea',
 '21b384-21b3f8':'a4d99963163930421f7910b85e88be51dd6c9a5dd0727a0e296742e37e9367c1',
 '22d8b0-22d8e0':'c36ef58c51cafd5c8253d6917b545b0ef3e62eb8d8fbc1cbb34a732885b4aa2f',
 '22e064-22e080':'03645c7d167c8f3a062cd9b343d0568cab464b7e15426bca6a50d37d57ae6804'}
BRANCHES={0x21ed74:0x21ed48,0x21ede0:0x21edac,0x21d83c:0x21d820,
 0x21d7f0:0x21d808,0x21d808:0x21d854,0x21d854:0x21d874}
CALLS={0x21d3e0:0x21af94,0x21d448:0x21d7d8,0x21d454:0x21afb0,
0x21d7f8:0x41a3b0,0x21d82c:0x21b384,0x21d844:0x36ede8,0x21d85c:0x36ede8,
0x21ed5c:0x41b800,0x21ed68:0x21d3d0,0x21edcc:0x21d43c,
0x21edd4:0x41a3b0,0x21b3b8:0x21b14c,0x21b3c0:0x41a3b0,
0x21b3d4:0x21afb0,0x22d8b0:0x21ed30,0x22d8c8:0x4f0a5c,
0x22e06c:0x21ed94}
ANCHORS={
0x21af94:0xac80000c,0x21af98:0xac800000,0x21af9c:0xac800004,0x21afa4:0xac800008,
0x21d3e4:0x24840018,0x21d3e8:0x240fffff,0x21d3ec:0xae000010,
0x21d3f0:0xae0f000c,0x21d3f4:0xae000000,0x21d3f8:0xae000004,
0x21d7ec:0x8c840000,0x21d800:0xae000000,0x21d804:0x8e0f0004,
0x21d810:0x8e0f0014,0x21d81c:0x240f0070,0x21d844:0x0c0dbb7a,
0x21d84c:0xae000004,0x21d850:0x8e040010,0x21d864:0x240fffff,
0x21d868:0xae000010,0x21d86c:0xae0f000c,
0x21ed54:0x24040028,0x21ed70:0x2a4f0003,0x21ed78:0xae300000,
0x21edb0:0x00127080,0x21edb4:0x25ef7b80,0x21edbc:0x01cf8821,
0x21edc0:0x8e300000,0x21eddc:0x2a4f0003,0x21ede4:0xae200000,
0x21b394:0x24910004,0x21b39c:0x24120002,
0x22d8c8:0x0c13c297,0x22d8dc:0x8de47b80,
0x22e06c:0x0c087b65}

def verify(elf,pck):
 b=Path(elf).read_bytes();p=Path(pck).read_bytes()
 if hashlib.sha256(b).hexdigest()!=SHA or hashlib.sha256(p).hexdigest()!=PCK:
  raise ValueError('Wrong owner-private original ELF/navigation.pck')
 phoff=struct.unpack_from('<I',b,28)[0];step,n=struct.unpack_from('<HH',b,42)
 seg=[]
 for i in range(n):
  kind,off,va,_,length,_,_,_=struct.unpack_from('<8I',b,phoff+i*step)
  if kind==1 and off+length<=len(b):seg.append((va,va+length,off))
 def read(va,n):
  for lo,hi,off in seg:
   if lo<=va and va+n<=hi:return b[off+va-lo:off+va-lo+n]
  raise ValueError('Unmapped ELF address '+hex(va))
 def word(pc):return struct.unpack('<I',read(pc,4))[0]
 digests={f'{lo:x}-{hi:x}':hashlib.sha256(read(lo,hi-lo)).hexdigest() for lo,hi in REGIONS}
 for source,sha in WINDOW_SHA.items():
  if digests[source]!=sha:raise ValueError('Source function differs at '+source)
 for pc,opcode in ANCHORS.items():
  if word(pc)!=opcode:raise ValueError(f'Original opcode failed at {pc:#x}: {word(pc):08x} != {opcode:08x}')
 for pc,target in CALLS.items():
  op=word(pc)
  if op>>26!=3 or ((pc+4)&0xf0000000|((op&0x03ffffff)<<2))!=target:
   raise ValueError(f'Original call mismatch at {pc:#x}')
 for pc,target in BRANCHES.items():
  imm=word(pc)&0xffff
  if imm>=0x8000:imm-=0x10000
  if pc+4+4*imm!=target:raise ValueError('Original branch target differs '+hex(pc))
 # Three owner slots and 0x28 source allocation; all three loop iterations.
 assert word(0x21ed54)&65535==0x28
 assert word(0x21ed70)&65535==3
 assert word(0x21eddc)&65535==3
 assert word(0x21d81c)&65535==0x70
 assert word(0x21d7f0)>>26==0x14 # BEQL header absent
 assert word(0x21d808)>>26==0x14 # BEQL record array absent
 assert word(0x21d854)>>26==0x14 # BEQL grid absent
 assert read(0x647b80,12)==b'\x00'*12 # Original global owner array initial state
 # Both real asset headers have 39/20 original polygons in source+0x24.
 entries=[]
 for i in range(2):
  name,t,off,size=struct.unpack_from('<4I',p,64+16*i)
  count=struct.unpack_from('<I',p,off+0x24)[0]
  assert count==(39 if i==0 else 20)
  entries.append((off,size,count))
 return {'retail_function_windows':len(REGIONS),'original_instructions':sum((hi-lo)//4 for lo,hi in REGIONS),
  'exact_opcodes':len(ANCHORS),'call_edges':len(CALLS),'branch_edges':len(BRANCHES),
  'sha256_of_source_windows':digests,
  'original_nav_owner_slots':3,'original_allocated_owner_bytes':0x28,
  'original_record_bytes':0x70,'owned_original_nav_polygon_counts':[39,20],
  'source_graph_live_allocation_recovered':False,'native_game_playable':False}
if __name__=='__main__':
 a=argparse.ArgumentParser();a.add_argument('elf',type=Path);a.add_argument('pck',type=Path)
 args=a.parse_args();print(json.dumps(verify(args.elf,args.pck),indent=2))
