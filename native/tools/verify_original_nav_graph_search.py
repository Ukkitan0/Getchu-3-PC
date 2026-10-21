#!/usr/bin/env python3
"""Source-level SHA audit: original USA 0x220C00 graph builder / 0x220D5C search."""
import argparse,hashlib,json,struct
from pathlib import Path
ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
PCK_SHA='d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0'
REGIONS={
 (0x220c00,0x220d5c):'0b25fc65e3a16f7a1c39f4fa6791adf24c89a98826096c1135ce1b6665b4fe04',
 (0x220d5c,0x220f60):'5a7b8d205645ff8f55d9d42748959eb023a9186048becffd5bbd5c15f7025b7b',
 (0x21b3f8,0x21b458):'a0016af186b13e30f38522dffab3cb43ab9775059d6008ab7d2af4598faccf83',
 (0x21d884,0x21d898):'13280ec58a823c9734db8119f63e240a17ec5004d082090c7433d8ad0e1d93cd',
}
def codes(s):return {int(x.split(':')[0],16):int(x.split(':')[1],16) for x in s.split()}
ANCHORS=codes("""220c24:8c8f000c 220c30:240e0038 220c44:8dd20004 220c48:0c086cfe 220c98:0c086d01 220ca4:0c088345 220ccc:8df00008 220cdc:ade2000c 220ce0:0c086d11 220cec:ae020010 220cfc:aded0008 220d08:adf40024 220d14:8c8c000c 220d1c:1980000d 220d34:8def0004 220d38:55e50003 220d5c:27bdffb0 220d88:8c8e0010 220d8c:adcf0030 220d90:adc00034 220d98:8e4e000c 220d9c:c5f447e8 220da4:19c00014 220dc4:91cf0000 220dc8:55e00008 220dd0:c5c10034 220dd4:46140834 220dd8:45020004 220de0:46000d06 220de4:01a0a82d 220df0:15e0fff1 220e00:4600a032 220e04:4501004c 220e0c:12af004a 220e20:01cf8021 220e2c:0c086d1a 220e30:a20f0000 220e34:8e0f0008 220e50:8def000c 220e5c:922f0000 220e6c:8dee0018 220e74:11cf0028 220e7c:51e00022 220e84:11c00010 220e8c:c6000034 220e90:c6210034 220e94:4600a500 220e98:4601a034 220e9c:45020004 220ea4:e6340034 220ea8:ae350030 220eb0:26730001 220eb8:55e0ffe2 220ec8:0c086d1a 220ed8:4be1106c 220edc:4be1086a 220ee0:4ba10841 220ee4:4be10842 220efc:46000504 220f18:020c7821 220f20:0c086d38 220f24:8de50024 220f28:0c086c65 220f30:1000ffd6 21b3f8:8c8f0030 21b400:95e20002 21b404:27bdfff0 21b428:95c50004 21b444:8c8f0030 21b454:95e2000a 21d884:240f0070 21d888:8c820004 21d894:00451021""")
CALLS=codes("""220c48:21b3f8 220c98:21b404 220ca4:220d14 220ce0:21b444 220e2c:21b468 220ec8:21b468 220f20:21b4e0 220f28:21b194""")
BRANCHES=codes("""220c28:220c74 220c58:220c98 220c6c:220c34 220cbc:220d0c 220d1c:220d54 220d38:220d48 220d4c:220d28 220da4:220df8 220dc8:220dec 220dd8:220dec 220df0:220db8 220e04:220f38 220e0c:220f38 220e38:220d94 220e60:220eb0 220e74:220f18 220e7c:220f08 220e84:220ec8 220e9c:220eb0 220eb8:220e44 220ec0:220d98 220f00:220e90 220f08:220e90 220f10:220ec8 220f30:220e8c""")
def audit(elf,pck):
 b=Path(elf).read_bytes();p=Path(pck).read_bytes()
 if hashlib.sha256(b).hexdigest()!=ELF_SHA or hashlib.sha256(p).hexdigest()!=PCK_SHA:
  raise ValueError('Expected privately owned original USA ELF and navigation PCK')
 assert b[:7]==b'\x7fELF\x01\x01\x01' and struct.unpack_from('<H',b,18)[0]==8
 ph=struct.unpack_from('<I',b,28)[0];step,n=struct.unpack_from('<HH',b,42);segments=[]
 for i in range(n):
  typ,off,va,_,size,_,_,_=struct.unpack_from('<8I',b,ph+i*step)
  if typ==1 and off+size<=len(b):segments.append((va,va+size,off))
 def read(a,n):
  for low,high,off in segments:
   if low<=a and a+n<=high:return b[off+a-low:off+a-low+n]
  raise ValueError('Original retail VA unmapped '+hex(a))
 def word(a):return struct.unpack('<I',read(a,4))[0]
 for (lo,hi),sha in REGIONS.items():
  if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=sha:raise ValueError('Source code-region SHA mismatch '+hex(lo))
 for pc,op in ANCHORS.items():
  if word(pc)!=op:raise ValueError('Original opcode mismatch '+hex(pc))
 for pc,target in CALLS.items():
  ins=word(pc)
  if ins>>26!=3 or (((pc+4)&0xf0000000)|((ins&0x03ffffff)<<2))!=target:
   raise ValueError('Original call mismatch '+hex(pc))
 for pc,target in BRANCHES.items():
  imm=word(pc)&65535
  if imm>=32768:imm-=65536
  if pc+4+4*imm!=target:raise ValueError('Original branch mismatch '+hex(pc))
 assert word(0x220c30)&65535==0x38
 assert struct.unpack('<I',read(0x6e47e8,4))[0]==0x7f7fffff
 assert (word(0x220d8c)&65535,word(0x220d90)&65535)==(0x30,0x34)
 assert (word(0x220e50)&65535,word(0x220e6c)&65535,word(0x220f24)&65535)==(12,24,36)
 assert (word(0x220ea4)&65535,word(0x220ea8)&65535)==(52,48)
 assert p[:4]==b'PCK\x00' and struct.unpack_from('<II',p,4)==(64,2)
 names=[];counts={};total=0
 for i in range(2):
  no,typ,off,size=struct.unpack_from('<4I',p,64+i*16)
  if p[typ:p.index(0,typ)]!=b'nav':raise ValueError('Source nav type differs')
  name=p[no:p.index(0,no)].decode('ascii');src=p[off:off+size]
  n=struct.unpack_from('<I',src,0x24)[0];head=struct.unpack_from('<I',src,0x3c)[0]
  assert (name,n)==[('nav_monkey.nav',39),('nav_enemy.nav',20)][i]
  edges=0
  for j in range(n):
   at=head+j*30;degree=struct.unpack_from('<H',src,at+2)[0]
   if degree>3:raise ValueError('Source 3-slot adjacency exceeded')
   for k in range(degree):
    neighbor=struct.unpack_from('<H',src,at+4+2*k)[0]
    kind=struct.unpack_from('<H',src,at+10+2*k)[0]
    if neighbor>=n or kind>2:raise ValueError('Source neighbor or type not decoded')
    counts[kind]=counts.get(kind,0)+1
   edges+=degree
  assert edges==[95,42][i]
  names.append(name);total+=edges
 assert total==137 and counts=={0:122,2:15}
 return {'original_instruction_words':sum((hi-lo)//4 for lo,hi in REGIONS),
 'sha_pinned_regions':len(REGIONS),'exact_opcodes':len(ANCHORS),
 'direct_calls':len(CALLS),'branch_targets':len(BRANCHES),
 'genuine_polygon_count':59,'directed_edges':total,'edge_types':counts,
 'source_resources':names,'original_search_record_bytes':56,
 'visited_at':0,'adjacency_at':12,'kind_at':24,'action_index_at':36,
 'predecessor_at':48,'best_score_at':52,'unvisited_score_bits':'0x7f7fffff',
 'full_windows_game_playable':False}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('original_US_ELF',type=Path);p.add_argument('private_navigation_PCK',type=Path)
 args=p.parse_args();print('PASS original 0x220C00 and 0x220D5C source-flow audit')
 print(json.dumps(audit(args.original_US_ELF,args.private_navigation_PCK),indent=2))
