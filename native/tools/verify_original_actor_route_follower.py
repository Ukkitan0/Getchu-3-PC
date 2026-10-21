#!/usr/bin/env python3
"""Audit retail Ape Escape 3 actor route traversal (no game data published)."""
import argparse,hashlib,json,struct
from pathlib import Path
ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
PCK_SHA='d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0'
REGIONS={
 (0x57c7cc,0x57c8ec):'d5941c30943e58525576393080619c3634bb4ff5dc3dcd1f8999fc08b19d3be5',
 (0x57c8ec,0x57ca68):'2c5b50641477273a31fbaf9d4e70228b348a5fb416f10d1e8546d4004613ba80',
 (0x4c1b3c,0x4c1bd8):'ff77828bd77a8683becdc06d797598149a084e5fa5858efaa00c19e92ab0022c',
}
ANCHORS={
 0x57c7f4:0x8c8f0004,0x57c7f8:0x908e001c,
 0x57c7fc:0x11c00036,0x57c800:0x8df30030,
 0x57c804:0x8c8f0050,0x57c808:0x11e00008,
 0x57c80c:0x03a0382d,0x57c814:0x0c1306cf,
 0x57c81c:0x8e0e0050,0x57c820:0x8faf0000,
 0x57c824:0x51ee0024,0x57c82c:0x8e0f0018,
 0x57c834:0x24140002,0x57c838:0x8def0000,
 0x57c83c:0x8de50004,0x57c840:0x0c15f23b,
 0x57c844:0xae050050,0x57c848:0x8e0f0018,
 0x57c850:0x8def0004,0x57c854:0x028f782a,
 0x57c858:0x11e00016,0x57c868:0x26940001,
 0x57c874:0x8dd10000,0x57c878:0x0c15f23b,
 0x57c87c:0x0220282d,0x57c890:0x0c1306cf,
 0x57c898:0x8faf0004,0x57c89c:0x562f0006,
 0x57c8a8:0xae110050,0x57c8ac:0x1000ffe8,
 0x57c8d8:0xac930050,0x57c8dc:0x0c15f23b,
 0x57c8e4:0x1000fff3,
}
CALLS={
 0x57c814:0x4c1b3c,0x57c840:0x57c8ec,
 0x57c878:0x57c8ec,0x57c890:0x4c1b3c,
 0x57c8dc:0x57c8ec,0x4c1ba8:0x21bd3c,
 0x57c930:0x21b788,0x57c9b0:0x21b458,
}
BRANCHES={
 0x57c7fc:0x57c8d8,0x57c808:0x57c82c,
 0x57c824:0x57c8b8,0x57c858:0x57c8b4,
 0x57c89c:0x57c8b8,0x57c8ac:0x57c850,
 0x57c8e4:0x57c8b4,
}
def verify(elf_path,nav_path):
 b=Path(elf_path).read_bytes();nav=Path(nav_path).read_bytes()
 if hashlib.sha256(b).hexdigest()!=ELF_SHA:raise ValueError('Private original USA ELF SHA mismatch')
 if hashlib.sha256(nav).hexdigest()!=PCK_SHA:raise ValueError('Private woods_c PCK SHA mismatch')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Expected original little-endian 32-bit MIPS ELF')
 ph=struct.unpack_from('<I',b,28)[0];step,n=struct.unpack_from('<HH',b,42)
 segments=[]
 for i in range(n):
  typ,off,va,_,size,_,_,_=struct.unpack_from('<8I',b,ph+step*i)
  if typ==1 and off+size<=len(b):segments.append((va,va+size,off))
 def read(va,n):
  for low,high,offset in segments:
   if low<=va and va+n<=high:return b[offset+va-low:offset+va-low+n]
  raise ValueError(f'Original unmapped source VA {va:#x}')
 def word(va):return struct.unpack('<I',read(va,4))[0]
 for (lo,hi),digest in REGIONS.items():
  if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=digest:
   raise ValueError(f'Original opcode region mismatch {lo:#x}')
 for va,w in ANCHORS.items():
  if word(va)!=w:raise ValueError(f'Original instruction mismatch {va:#x}')
 for va,target in CALLS.items():
  op=word(va)
  if op>>26!=3 or (((va+4)&0xf0000000)|((op&0x03ffffff)<<2))!=target:
   raise ValueError(f'Original JAL mismatch {va:#x}')
 for va,target in BRANCHES.items():
  displacement=struct.unpack('<h',struct.pack('<H',word(va)&0xffff))[0]
  if va+4+4*displacement!=target:
   raise ValueError(f'Original branch mismatch {va:#x}')
 if len(nav)!=3328 or nav[:4]!=b'PCK\0':raise ValueError('Original nav PCK altered')
 polygons=[];links=[];specials=[]
 for i,(off,length) in enumerate(((96,2128),(2224,1104))):
  data=nav[off:off+length]
  count=struct.unpack_from('<I',data,0x24)[0]
  start=struct.unpack_from('<I',data,0x3c)[0]
  if count!=(39 if i==0 else 20) or start+30*count>len(data):
   raise ValueError('Original descriptor count mismatch')
  edges=types2=0
  for s in range(count):
   at=start+30*s;degree=struct.unpack_from('<H',data,at+2)[0]
   if degree>3:raise ValueError('Original polygon degree exceeds 3')
   for slot in range(degree):
    dest=struct.unpack_from('<H',data,at+4+2*slot)[0]
    kind=struct.unpack_from('<H',data,at+10+2*slot)[0]
    if dest>=count or kind>2:raise ValueError('Original polygon edge invalid')
    edges+=1;types2+=(kind==2)
  polygons.append(count);links.append(edges);specials.append(types2)
 if links!=[95,42] or specials!=[15,0]:
  raise ValueError('Original woods_c navigation edges changed')
 return {'retail_ELF_SHA256':ELF_SHA,'retail_navigation_SHA256':PCK_SHA,
  'complete_source_regions':len(REGIONS),
  'instruction_words_audited':sum((hi-lo)//4 for lo,hi in REGIONS),
  'opcode_anchors':len(ANCHORS),'source_JAL_call_edges':len(CALLS),
  'source_branch_edges':len(BRANCHES),'retail_graph_polygons':polygons,
  'retail_graph_edges':links,'retail_type2_actions':specials,
  'native_route_follower':'0x57C7CC..0x57C8E8',
  'untranslated_original_dependencies':['0x4C1B3C VU projection','0x57C8EC VU target construction'],
  'playable_windows_game':False}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('original_elf',type=Path)
 p.add_argument('original_navigation_pck',type=Path)
 a=p.parse_args()
 print(json.dumps(verify(a.original_elf,a.original_navigation_pck),indent=2))
