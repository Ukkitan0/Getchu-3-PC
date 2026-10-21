#!/usr/bin/env python3
"""Pin original retail USA AE3 VU0 NavGraph type-1 length and route relaxation.

This verifies the exact owner-provided US retail ELF instructions. No
retail executable or original instruction blob is redistributed.
"""
import argparse,hashlib,json,struct
from pathlib import Path
ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
REGIONS={
 (0x21b178,0x21b194):'f451cc60bb1ce68ab4813b06891ca5558e9f334dbdea41ff3b2d0ad21d1f353e',
 (0x21b194,0x21b240):'398a5def855a603e809038f0c8cf342b97134c37d63b86957385c3c49448ed1e',
 (0x21b4e0,0x21b4f0):'acd5944578702a3ea44d35b19866229559b232f963598876c28820a1a017bb21',
 (0x21b468,0x21b4e0):'ed9b248a9cdcfd4f8b30859b3c6bacb6e0d32e2313df293e017b975cbeb9fefd',
 (0x220ec8,0x220f08):'0c49dcc017e95fb7d6554202a80b779b7ca705a627608d17a9e652151176e52e',
 (0x220e68,0x220e8c):'dae55004defabaf6f7705fe317f7aef495bbc422b4121ec8aa22c37cb1e7d90b',
 (0x220e8c,0x220eb0):'0f3756f6b2f65795591e9617885ba715e4e775138a6afbffbd85d39c99ee6007',
 (0x220f18,0x220f38):'17986289fef1af0736e0430bd9828366532560802b9080854039d1aa83b00d0b',
}
ANCHORS={
 0x21b178:0x03e00008,0x21b180:0x8c8f0004,0x21b190:0x78a20000,
 0x21b198:0x44800000,0x21b1a4:0x46000506,
 0x21b1b8:0x0c086c5e,0x21b1c0:0x2442ffff,0x21b1c8:0x0202102a,
 0x21b1cc:0x14400008,0x21b1d4:0x4600a006,
 0x21b1f0:0x0c086c60,0x21b1f8:0x7fa20000,
 0x21b200:0x0c086c60,0x21b208:0x48a21000,
 0x21b20c:0xdba10000,0x21b210:0x4be2086c,
 0x21b214:0x4be1086a,0x21b218:0x4ba10841,
 0x21b21c:0x4be10842,0x21b220:0x482f0800,
 0x21b228:0x448f0000,0x21b234:0x46000004,
 0x21b238:0x1000ffdf,0x21b23c:0x4600a500,
 0x21b4e0:0x00052880,0x21b4ec:0x8ca20004,
 0x21b46c:0x8c8e0030,0x21b478:0x95cf0016,
 0x21b484:0x48a21000,0x21b494:0x4be110a8,
 0x21b498:0x15e0fffc,0x21b4a0:0x48221000,
 0x21b4a4:0x95cf0016,0x21b4ac:0x448f0800,
 0x21b4b4:0x46800860,0x21b4bc:0xc5e00000,
 0x21b4c8:0x46010083,0x21b4cc:0x440f1000,
 0x21b4d0:0x48af0800,0x21b4d4:0x4be11058,
 0x21b4dc:0x48220800,
 0x220ec8:0x0c086d1a,0x220ed0:0x48a20800,
 0x220ed4:0x48b41000,0x220ed8:0x4be1106c,
 0x220edc:0x4be1086a,0x220ee0:0x4ba10841,
 0x220ee4:0x4be10842,0x220ee8:0x482e0800,
 0x220ef0:0x448f0000,0x220efc:0x46000504,
 0x220f00:0x1000ffe3,
 0x220e74:0x11cf0028,0x220e8c:0xc6000034,
 0x220e90:0xc6210034,0x220e94:0x4600a500,
 0x220e98:0x4601a034,0x220e9c:0x45020004,
 0x220ea4:0xe6340034,0x220ea8:0xae350030,
 0x220f1c:0x8e040004,0x220f20:0x0c086d38,
 0x220f24:0x8de50024,0x220f28:0x0c086c65,
 0x220f2c:0x0040202d,0x220f34:0x46000506,
}
CALLS={0x21b1b8:0x21b178,0x21b1f0:0x21b180,0x21b200:0x21b180,
       0x220f20:0x21b4e0,0x220f28:0x21b194,
       0x220ec8:0x21b468}
BRANCHES={0x21b1cc:0x21b1f0,0x21b238:0x21b1b8,
          0x220e74:0x220f18,0x220e9c:0x220eb0,
          0x220f30:0x220e8c,0x21b47c:0x21b4ac,
          0x21b498:0x21b48c,0x220f00:0x220e90}
# Decode COP2 special1 VU0 macro opcodes by their *exact* semantic
# operation and destination component masks. This is verification,
# not an emulator or dependency on third-party emulator source.
VU0={0x21b494:('VADD',0x28,0xf),0x21b4d4:('VMULx',0x18,0xf),
     0x220ed8:('VSUB',0x2c,0xf),0x220edc:('VMUL',0x2a,0xf),
     0x220ee0:('VADDy',0x01,0xd),0x220ee4:('VADDz',0x02,0xf),
     0x21b210:('VSUB',0x2c,0xf),0x21b214:('VMUL',0x2a,0xf),
     0x21b218:('VADDy',0x01,0xd),0x21b21c:('VADDz',0x02,0xf)}
def verify(path):
 b=Path(path).read_bytes()
 if hashlib.sha256(b).hexdigest()!=ELF_SHA:
  raise ValueError('Incorrect source USA original SCUS_975.01 executable')
 if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
  raise ValueError('Not original little-endian MIPS ELF32')
 ph=struct.unpack_from('<I',b,28)[0];stride,n=struct.unpack_from('<HH',b,42)
 segments=[]
 for i in range(n):
  typ,off,va,_,filesz,_,_,_=struct.unpack_from('<8I',b,ph+i*stride)
  if typ==1 and off+filesz<=len(b):segments.append((va,va+filesz,off))
 def read(va,length):
  for lo,hi,off in segments:
   if lo<=va and va+length<=hi:return b[off+va-lo:off+va-lo+length]
  raise ValueError('Unmapped retail source VA '+hex(va))
 def w(va):return struct.unpack('<I',read(va,4))[0]
 for (lo,hi),expected in REGIONS.items():
  if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=expected:
   raise ValueError('Original region changed at '+hex(lo))
 for va,word in ANCHORS.items():
  if w(va)!=word:raise ValueError('Instruction mismatch at '+hex(va))
 for va,target in CALLS.items():
  word=w(va)
  if (word>>26)!=3 or ((((va+4)&0xf0000000)|((word&0x03ffffff)<<2))!=target):
   raise ValueError('Source call changed at '+hex(va))
 for va,target in BRANCHES.items():
  delta=w(va)&65535
  if delta>=32768:delta-=65536
  if va+4+4*delta!=target:raise ValueError('Source branch changed at '+hex(va))
 for va,(name,op,mask) in VU0.items():
  word=w(va)
  if (word>>26)!=0x12 or ((word>>21)&31)<16 or (word&63)!=op or ((word>>21)&15)!=mask:
   raise ValueError('Original VU0 macro decode mismatch: '+name)
 # Source 0x21B178 retrieves count. 0x21B180 returns vector via LQ.
 assert w(0x21b190)>>26==0x1e
 # 0x21B1F8 stores first source vector with SQ, and 0x21B20C
 # reloads it after the second vector has entered vf2 via QMTC2.
 assert (w(0x21b1f8)>>26,w(0x21b20c)>>26)==(0x1f,0x36)
 assert ((w(0x21b208)>>21)&31,(w(0x21b220)>>21)&31)==(5,1)
 assert struct.unpack('<I',read(0x6e3e2c,4))[0]==0x3f800000
 # Original sqrt.s F0 and f20 += distance in loop branch delay slot.
 assert w(0x21b234)==0x46000004 and w(0x21b23c)==0x4600a500
 # Original direct-distance branch obtains source polygon mean and
 # computes XYZ distance to actor query before shared relax path.
 assert (w(0x21b478)&65535)==0x16
 assert w(0x21b4c8)==0x46010083 # f0/f1 scalar reciprocal
 assert w(0x220ec8)>>26==3 and w(0x220efc)==0x46000504
 # Real actor route relaxer compares candidate against neighbor +0x34,
 # writes that field and predecessor +0x30 only on strict improvement.
 assert w(0x220e98)==0x4601a034 and w(0x220ea4)&65535==0x34
 assert w(0x220ea8)&65535==0x30
 return {'original_elf_sha256':ELF_SHA,'source_regions':len(REGIONS),
  'source_instruction_words':sum((hi-lo)//4 for lo,hi in REGIONS),
  'independent_opcode_anchors':len(ANCHORS),'verified_calls':len(CALLS),
  'branch_targets':len(BRANCHES),
  'VU0_operations':{hex(k):v[0] for k,v in VU0.items()},
  'vector_lanes_for_length':'XYZ only, W excluded',
  'source_child_count_cases':'count<2 -> zero',
  'route_relax':'current + polyline_length < neighbor => update score/predecessor',
  'direct_distance_path':'polygon mean -> XYZ distance -> shared relax',
  'native_IEEE_float_bit_identical_to_VU0':False,
  'original_game_playable':False}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('original_elf',type=Path)
 data=verify(p.parse_args().original_elf)
 print('PASS: authentic 0x21B194 VU0 navigation length and 0x220F28 route relax')
 print(json.dumps(data,indent=2))
