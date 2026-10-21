#!/usr/bin/env python3
"""Verify original AE3 USA VF/packed MMI geometry against owner's retail ELF.

This statically audits a hash-pinned executable, outputs metadata only,
and never embeds commercial game bytes or runs original PS2 hardware code.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
REGIONS={
 (0x21b468,0x21b4e0):"ed9b248a9cdcfd4f8b30859b3c6bacb6e0d32e2313df293e017b975cbeb9fefd",
 (0x21b788,0x21b854):"c434176b035c929b0b12062267efb4b069e5fc796a3a715da1d9375a763c7d90",
 (0x21b194,0x21b240):"398a5def855a603e809038f0c8cf342b97134c37d63b86957385c3c49448ed1e",
 (0x21d940,0x21d978):"909733703ef2de7269ef7d8f7517680b2f7de0ffe8de6df390749b4d6d5d1181",
 (0x21d9f0,0x21da24):"7b086dd0a584be5a1c394cd790232aa9be5e4524f6904ee4b54a48a6b8bb901e",
 (0x21da88,0x21dad0):"7915e1ed2e36f892a40741e60e1a04a7970b94b2e57d253373f25194b0d8feaa"
}
ANCHORS={
 0x21b470:0x71ef7cc8,0x21b474:0x71ef14c8,
 0x21b480:0x48a21000,0x21b484:0x48a21000,
 0x21b48c:0xd8810000,0x21b494:0x4be110a8,
 0x21b4a0:0x48221000,0x21b4a8:0x48a21000,
 0x21b4c8:0x46010083,0x21b4d0:0x48af0800,
 0x21b4d4:0x4be11058,0x21b4dc:0x48220800,
 0x21b790:0x48a52000,0x21b7ac:0x4be21b3c,
 0x21b7b4:0x4b60006c,0x21b7c0:0x4a800043,
 0x21b7c4:0x4be2196c,0x21b7cc:0x482f2800,
 0x21b7d0:0x71cf7488,0x21b7d4:0x71ef7ba9,
 0x21b7d8:0x71ee7b89,0x21b7dc:0x48af2800,
 0x21b7e0:0x4bc50afe,0x21b7e4:0x4bc1286e,
 0x21b7e8:0x4be208aa,0x21b7ec:0x4be4086a,
 0x21b7f0:0x4ba21081,0x21b7f4:0x4ba10841,
 0x21b7f8:0x4be21082,0x21b7fc:0x4be10842,
 0x21b80c:0x460c0001,0x21b81c:0x46000834,
 0x21b820:0x45020001,0x21b844:0x4be21b3c,
 0x21b208:0x48a21000,0x21b210:0x4be2086c,
 0x21b214:0x4be1086a,0x21b218:0x4ba10841,
 0x21b21c:0x4be10842,0x21b220:0x482f0800,
 0x21b234:0x46000004,0x21b23c:0x4600a500,
 0x21d950:0x4be2086c,0x21d954:0x4be1086a,
 0x21d958:0x4ba10841,0x21d95c:0x4be10842,
 0x21d9fc:0x4be2086c,0x21da00:0x4be1086a,
 0x21da04:0x4ba10841,0x21da08:0x4be10842,
 0x21da98:0x4be2086c,0x21da9c:0x4be1086a,
 0x21daa0:0x4ba10841,0x21daa4:0x4be10842,
 0x6e3e2c:0x3f800000
}
TARGETS={
 0x21b1b8:0x21b178,0x21b1f0:0x21b180,
 0x21b200:0x21b180,0x21d91c:0x21b788,
 0x21d940:0x21b468,0x21d9ec:0x21b468,
 0x21da88:0x21b468
}
def verify(path):
 data=path.read_bytes()
 if hashlib.sha256(data).hexdigest()!=SHA:
  raise ValueError("Original USA executable is not pinned SCUS_975.01")
 if data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0]!=8:
  raise ValueError("Expected little-endian MIPS ELF32")
 ph=struct.unpack_from("<I",data,28)[0]
 stride,n=struct.unpack_from("<HH",data,42)
 segments=[]
 for i in range(n):
  kind,off,va,_,size,_,_,_=struct.unpack_from("<8I",data,ph+i*stride)
  if kind==1 and off+size<=len(data):segments.append((va,va+size,off))
 if not segments:raise ValueError("No loadable ELF segments")
 def read(addr,size):
  for lo,hi,off in segments:
   if lo<=addr and addr+size<=hi:return data[off+addr-lo:off+addr-lo+size]
  raise ValueError("Unmapped executable address "+hex(addr))
 def word(addr):return struct.unpack("<I",read(addr,4))[0]
 for (start,end),hash_value in REGIONS.items():
  if hashlib.sha256(read(start,end-start)).hexdigest()!=hash_value:
   raise ValueError("Original ELF machine-code window changed: "+hex(start))
 for va,op in ANCHORS.items():
  if word(va)!=op:raise ValueError("Original MIPS/COP2/MMI opcode differs: "+hex(va))
 for pc,expected in TARGETS.items():
  op=word(pc)
  target=((pc+4)&0xf0000000)|((op&0x3ffffff)<<2)
  if op>>26 not in (2,3) or target!=expected:
   raise ValueError("Original direct original ELF branch differs: "+hex(pc))
 return {
  "format":"ae3/verified-original-VU0-math/v1",
  "original_sha256":SHA,
  "verified_complete_windows":len(REGIONS),
  "verified_instruction_words":sum((end-start)//4 for start,end in REGIONS),
  "verified_independent_opcode_anchors":len(ANCHORS),
  "verified_original_direct_calls":len(TARGETS),
  "original_centroid":"0x21B468 VADD.xyzw + DIV.S + VMULx.xyzw",
  "original_plane":"0x21B788 VOPMULA/VOPMSUB + PEXTLW/PCPYUD + VMUL/VADD",
  "original_distance":"0x21B210 VSUB/VMUL/VADDy/VADDz/SQRT.S",
  "original_paths":["0x21B194","0x21D898","0x21DA2C"],
  "limits":"Host float exceptions and PS2-specific rounding not bit-exact for every input; native object ownership, rendering and game boot unfinished"
 }
if __name__=="__main__":
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument("original_elf",type=Path);p.add_argument("--out",type=Path)
 a=p.parse_args();result=verify(a.original_elf)
 body=json.dumps(result,indent=2)+"\n"
 if a.out:
  a.out.parent.mkdir(parents=True,exist_ok=True)
  a.out.write_text(body,encoding="utf-8")
 else:print(body,end="")
 print("PASS original VU0 recovered math; authenticated",result["verified_instruction_words"],"instructions")
