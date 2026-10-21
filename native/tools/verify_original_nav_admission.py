#!/usr/bin/env python3
"""Audit original Ape Escape 3 USA NavGraph workspace admission.

Reads the owner's exact retail ELF and writes metadata only. No
commercial game bytes are included or published.
"""
import argparse,hashlib,json,struct
from pathlib import Path
SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
REGIONS={
(0x2207d0,0x220994):"0d080b45e4ca0e8f77eab6f804341d2dd1ebf63fbf60961e36c9af25f68b67bf",
(0x220aa4,0x220c00):"a04c1c570197cea97e26e1c65278d2802c23b0e4c8a3cfe970207fb27a6dddc5",
(0x21afb8,0x21b040):"7eb90f6245ffa39cd0e3b4e474b412f53011bb7d400860292b48aebe61158f78",
(0x4e3dac,0x4e3dec):"0adb0b867adcfc351b1dc62d01bca43aaf4f7c1b665e7be27ea76ac6490247a3",
}
ANCHORS={
0x2207d8:0xac80000c,0x220820:0x0c106e2c,0x220828:0x0040682d,
0x220834:0xa1a00000,0x220840:0xada00008,0x22084c:0xadaf0030,
0x220858:0xaea20000,0x220868:0x0200202d,0x220890:0x0c0882a9,
0x220898:0x0c088300,0x2208a0:0x0c088357,0x2208b4:0x12cf001b,
0x220924:0x0c0883d8,0x220938:0x0c08840c,0x22096c:0x0c0882a9,
0x220aa4:0x27bdffc0,0x220ad8:0x15000004,0x220ae4:0x0c086bee,
0x220af0:0x0c086c05,0x220b04:0x10800007,0x220b0c:0x12800005,
0x220b14:0x0c087e27,0x220b24:0x0c086d1a,0x220b58:0x46000004,
0x220b5c:0x46140034,0x220b60:0x45000013,0x220b68:0x8e4e000c,
0x220b74:0x01cf7818,0x220b80:0x25ce0001,0x220b88:0xae4e000c,
0x220b8c:0x0c086c0e,0x220b94:0x0c086cfe,0x220bd4:0x0c086d01,
0x220bf0:0x0c0882a9,0x21afb8:0x8c8f0004,0x21afcc:0x25ef0001,
0x21afd0:0x15e0000e,0x21afd4:0xac8f000c,0x21aff8:0xade00028,
0x21b014:0x8c8f0004,0x21b01c:0x8def000c,0x21b02c:0xac800008,
0x21b038:0x03e00008,0x4e3dac:0x8c840000,0x4e3dc8:0x8dcf0008,
0x4e3dcc:0x11e50003,0x4e3de4:0x1000fffd,
}
CALLS={
0x220820:0x41b8b0,0x22086c:0x21f89c,0x220890:0x220aa4,
0x220898:0x220c00,0x2208a0:0x220d5c,0x2208c0:0x220994,
0x220924:0x220f60,0x220938:0x221030,0x22094c:0x21f8b8,
0x22096c:0x220aa4,0x220974:0x220c00,0x22097c:0x220d5c,
0x220984:0x21b468,0x220ae4:0x21afb8,0x220af0:0x21b014,
0x220b14:0x21f89c,0x220b24:0x21b468,0x220b8c:0x21b038,
0x220b94:0x21b3f8,0x220bd4:0x21b404,0x220bf0:0x220aa4,
}
def verify(path):
 data=path.read_bytes()
 if hashlib.sha256(data).hexdigest()!=SHA:
  raise ValueError("Exact owner-private USA SCUS_975.01 required")
 if data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0]!=8:
  raise ValueError("Expected original little-endian MIPS ELF32")
 ph=struct.unpack_from("<I",data,28)[0];stride,n=struct.unpack_from("<HH",data,42)
 seg=[]
 for i in range(n):
  kind,off,va,_,size,_,_,_=struct.unpack_from("<8I",data,ph+i*stride)
  if kind==1 and off+size<=len(data):seg.append((va,va+size,off))
 def read(va,size):
  for lo,hi,offset in seg:
   if lo<=va and va+size<=hi:return data[offset+va-lo:offset+va-lo+size]
  raise ValueError("Unmapped original ELF address "+hex(va))
 def word(va):return struct.unpack("<I",read(va,4))[0]
 for (start,end),digest in REGIONS.items():
  if hashlib.sha256(read(start,end-start)).hexdigest()!=digest:
   raise ValueError("Original function bytes changed "+hex(start))
 for va,op in ANCHORS.items():
  if word(va)!=op:raise ValueError("Original opcode mismatch "+hex(va))
 for va,target in CALLS.items():
  op=word(va)
  actual=((va+4)&0xf0000000)|((op&0x3ffffff)<<2)
  if op>>26 not in (2,3) or actual!=target:
   raise ValueError("Original J/JAL target mismatch "+hex(va))
 if word(0x6e47e4)!=0x7f7fffff:
  raise ValueError("Original 256-node initialization distance constant changed")
 return {"format":"ae3/native-original-route-admission/v1",
  "original_elf_sha256":SHA,"verified_code_windows":len(REGIONS),
  "original_instruction_words":sum((end-start)//4 for start,end in REGIONS),
  "independent_original_opcode_sites":len(ANCHORS),
  "original_direct_J_JAL_targets":len(CALLS),
  "source_functions":["0x2207d0","0x220aa4","0x21afb8",
   "0x21b014","0x21b038","0x4e3dac"],
  "source_fixed_alloc_bytes":0x3800,"source_node_stride":0x38,
  "source_max_workspace_nodes":256,
  "limitations":["original 0x2207d0 fallback 0x21f8b8 is not ported",
   "real game graph/list source ownership is not restored",
   "original actor movement and rendering not operational"]}
if __name__=="__main__":
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument("original_elf",type=Path);p.add_argument("--out",type=Path)
 a=p.parse_args();v=verify(a.original_elf)
 payload=json.dumps(v,indent=2)+"\n"
 if a.out:a.out.write_text(payload,encoding="utf-8")
 else:print(payload,end="")
 print("PASS: verified",v["original_instruction_words"],"source MIPS words")
