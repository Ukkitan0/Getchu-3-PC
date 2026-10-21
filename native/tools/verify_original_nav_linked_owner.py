#!/usr/bin/env python3
"""Audit true original AE3 USA navigation-list construction/lifetime.

Owner-private exact SCUS_975.01 ELF required. Checks complete original code
windows, exact MIPS instructions and JAL targets; game bytes stay private.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
WINDOWS={
 (0x21f604,0x21f64c):'31ec7155290e27517a64b4fb91ca50fab2d85a684b1f4d8fa2bd96a14ac758c6',
 (0x21f64c,0x21f6d4):'fd28331b66f0768fcc46f3014a50ff5696909c9ac5d5221c1029016f3b830f67',
 (0x21f6d4,0x21f804):'8d118549d487c40d3e9b8dc8aa4dce44e12c7a71dc12cbeb7ad953122b9702a5',
 (0x21f804,0x21f82c):'f6b61e96a167e005e54c073dd91d0d5ad1865df595ad1cfc8dd4a02fd375455b',
 (0x21f82c,0x21f89c):'3e392b490c7f7e763cae029437d4b354d5af6bc17633260841438d4417cc4220',
 (0x4e3d10,0x4e3d9c):'250eae1883fe291d1fe4d7855cfbff046281fe31d6c37dbaf0efc64b5a6e7474',
 (0x4e3dac,0x4e3dec):'0adb0b867adcfc351b1dc62d01bca43aaf4f7c1b665e7be27ea76ac6490247a3',
 (0x4e3eb4,0x4e3ef8):'948bc124a936ec94c081140489a8ecb7da924233f7a64412adab58912f7eb396',
 (0x4e3f40,0x4e3f8c):'356704a23bcfb96f3aff254c5ccd742d31228af44c0c09c595d45357886e758e',
 (0x4e4154,0x4e41bc):'73a576fc50da8b2f9062ae47e4fce9ed71e84dadef781d04cfc1c3cbfd40bb6c',
}
ANCHORS={
 0x21f608:0xac850000,0x21f61c:0x0c106e00,0x21f62c:0x0c138fad,
 0x21f634:0xae110004,0x21f658:0x8c900004,0x21f664:0x0c138fbe,
 0x21f698:0xffbf0008,0x21f6a8:0x0c138fbe,0x21f700:0x8dae0000,
 0x21f704:0x8dcf0014,0x21f728:0x0c087621,0x21f738:0x0c086d1a,
 0x21f750:0x0c087bcf,0x21f758:0x44800800,0x21f760:0x45000019,
 0x21f768:0x8ed10004,0x21f770:0x0c138f6b,0x21f774:0x0220202d,
 0x21f784:0x0c113908,0x21f79c:0xac4f0008,0x21f7a0:0xac4e0000,
 0x21f7ac:0xac4f0004,0x21f7b8:0xade20000,0x21f7c0:0xade20004,
 0x21f818:0x0c138f44,0x21f840:0x0c138f6b,0x21f84c:0x1440000f,
 0x21f854:0x8e0f0000,0x21f858:0x0c138fd0,0x21f864:0xac4f0000,
 0x21f86c:0x8def0004,0x21f87c:0xade20000,0x21f884:0xade20004,
 0x4e3d10:0x27bdff90,0x4e3d20:0x8c8f0000,0x4e3d4c:0x8d8f0000,
 0x4e3d6c:0x0c138fc8,0x4e3dac:0x8c840000,0x4e3db4:0x8c8f0000,
 0x4e3dc8:0x8dcf0008,0x4e3dcc:0x11e50003,0x4e3edc:0xae020000,
 0x4e3ee0:0xac420004,0x4e3ee4:0xac420000,0x4e3f50:0x0c040561,
 0x4e3f78:0xac4f0008,0x4e4168:0x8c8f0000,0x4e4184:0x0c138fc8,
 0x4e41a4:0xadef0004,0x4e41a8:0xadef0000,
}
CALLS={
 0x21f61c:0x41b800,0x21f62c:0x4e3eb4,
 0x21f664:0x4e3ef8,0x21f678:0x41a3b0,
 0x21f6a8:0x4e3ef8,0x21f6bc:0x41a3b0,
 0x21f728:0x21d884,0x21f738:0x21b468,
 0x21f750:0x21ef3c,0x21f770:0x4e3dac,
 0x21f784:0x44e420,0x21f818:0x4e3d10,
 0x21f840:0x4e3dac,0x21f858:0x4e3f40,
 0x4e3d6c:0x4e3f20,0x4e3ec4:0x101584,
 0x4e3ed4:0x44e280,0x4e3f50:0x101584,
 0x4e3f60:0x44e280,0x4e4184:0x4e3f20,
}
def verify(path:Path):
 data=path.read_bytes()
 if hashlib.sha256(data).hexdigest()!=SHA:
  raise ValueError('Expected original USA owner-private SCUS_975.01 ELF')
 if data[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',data,18)[0]!=8:
  raise ValueError('Expected original MIPS little-endian ELF32')
 ph=struct.unpack_from('<I',data,28)
 step,num=struct.unpack_from('<HH',data,42)
 seg=[]
 for i in range(num):
  typ,off,va,_,size,_,_,_=struct.unpack_from('<8I',data,ph[0]+i*step)
  if typ==1 and off+size<=len(data):seg.append((va,va+size,off))
 def read(addr,size):
  for lo,hi,off in seg:
   if lo<=addr and addr+size<=hi:return data[off+addr-lo:off+addr-lo+size]
  raise ValueError('Unmapped original ELF VA '+hex(addr))
 def word(addr):return struct.unpack('<I',read(addr,4))[0]
 for (lo,hi),digest in WINDOWS.items():
  if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=digest:
   raise ValueError('Original list engine code window differs '+hex(lo))
 for addr,w in ANCHORS.items():
  if word(addr)!=w:raise ValueError('Original list opcode differs '+hex(addr))
 for addr,target in CALLS.items():
  w=word(addr)
  dest=((addr+4)&0xf0000000)|((w&0x3ffffff)<<2)
  if w>>26 not in (2,3) or dest!=target:
   raise ValueError('Original list J/JAL destination differs '+hex(addr))
 return {'format':'ae3/original-nav-linked-list-ownership/v1',
  'original_elf_sha256':SHA,'verified_full_code_regions':len(WINDOWS),
  'verified_original_instruction_words':sum((b-a)//4 for a,b in WINDOWS),
  'verified_independent_opcode_sites':len(ANCHORS),
  'verified_direct_call_targets':len(CALLS),
  'original_node_layout_bytes':12,
  'original_pointer_fields':{'next':0,'prev':4,'polygon':8},
  'original_source_functions':['0x21f604','0x21f64c','0x21f690','0x21f6d4',
   '0x21f804','0x21f82c','0x4e3d10','0x4e3dac','0x4e3eb4',
   '0x4e3f40','0x4e4154'],
  'original_population_predicate_untranslated':'0x21ef3c',
  'native_game_boot_verified':False}
if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('original_elf',type=Path)
 parser.add_argument('--out',type=Path)
 args=parser.parse_args()
 report=verify(args.original_elf)
 output=json.dumps(report,indent=2)+'\n'
 if args.out:args.out.write_text(output,encoding='utf-8')
 else:print(output,end='')
 print('PASS original list:',report['verified_original_instruction_words'],
       'machine instructions,',report['verified_direct_call_targets'],'J/JAL targets')
