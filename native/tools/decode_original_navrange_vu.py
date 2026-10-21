#!/usr/bin/env python3
"""Static original USA COP2/VU opcode atlas, not an emulator or gameplay substitute."""
import argparse,hashlib,json,struct
from collections import Counter
from pathlib import Path
SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
RANGES=((0x21ef3c,0x21f5a4),(0x359e80,0x359f44),(0x359f44,0x35a178),(0x441f40,0x441f70))
S1=("VADDx VADDy VADDz VADDw VSUBx VSUBy VSUBz VSUBw "
"VMADDx VMADDy VMADDz VMADDw VMSUBx VMSUBy VMSUBz VMSUBw "
"VMAXx VMAXy VMAXz VMAXw VMINIx VMINIy VMINIz VMINIw "
"VMULx VMULy VMULz VMULw VMULq VMAXi VMULi VMINIi "
"VADDq VMADDq VADDi VMADDi VSUBq VMSUBq VSUBi VMSUBi "
"VADD VMADD VMUL VMAX VSUB VMSUB VOPMSUB VMINI "
"VIADD VISUB VIADDI ? VIAND VIOR ? ? "
"VCALLMS CALLMSR ? ? SPECIAL2 SPECIAL2 SPECIAL2 SPECIAL2").split()
S2=("VADDAx VADDAy VADDAz VADDAw VSUBAx VSUBy VSUBAz VSUBAw "
"VMADDAx VMADDAy VMADDAz VMADDAw VMSUBAx VMSUBAy VMSUBAz VMSUBAw "
"VITOF0 VITOF4 VITOF12 VITOF15 VFTOI0 VFTOI4 VFTOI12 VFTOI15 "
"VMULAx VMULAy VMULAz VMULAw VMULAq VABS VMULAi VCLIPw "
"VADDAq VMADDAq VADDAi VMADDAi VSUBAq VMSUBAq VSUBAi VMSUBAi "
"VADDA VMADDA VMULA ? VSUBA VMSUBA VOPMULA VNOP "
"VMONE VMR32 ? ? VLQI VSQI VLQD VSQD "
"VDIV VSQRT VRSQRT VWAITQ VMTIR VMFIR VILWR VISWR "
"VRNEXT VRGET VRINIT VRXOR ? ? ? ?").split()
assert len(S1)==64 and len(S2)==72
TRANSFERS={0:"MFC2",1:"DMFC2",2:"CFC2",4:"MTC2",5:"DMTC2",6:"CTC2",8:"BC2"}
def decode_word(va,w):
 r={"va":f"0x{va:08X}","word":f"0x{w:08X}"}
 if w>>26 != 0x12:r["family"]="other";return r
 rs=(w>>21)&31
 if rs<16:
  r.update(family="cop2-transfer",op=TRANSFERS.get(rs,"COP2_OTHER"),
           gpr=(w>>16)&31,vreg=(w>>11)&31)
  return r
 special=(w&63)>=0x3c
 n=((w&3)|((w>>4)&0x7c)) if special else (w&63)
 name=S2[n] if special and n<len(S2) else (S1[n] if not special else "?")
 mask=(w>>21)&15
 r.update(family="cop2-vu",op=name,fd=(w>>6)&31,fs=(w>>11)&31,
          ft=(w>>16)&31,mask="".join(c for i,c in enumerate("xyzw") if mask&(8>>i)),
          special2=special,index=n)
 return r
def elf_words(path):
 b=Path(path).read_bytes()
 if hashlib.sha256(b).hexdigest()!=SHA:raise ValueError("Wrong original USA retail ELF")
 if b[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",b,18)[0]!=8:raise ValueError("Not MIPS ELF32 LE")
 ph=struct.unpack_from("<I",b,28)[0]
 stride,num=struct.unpack_from("<HH",b,42)
 segments=[]
 for i in range(num):
  typ,off,va,_,sz,_,_,_=struct.unpack_from("<8I",b,ph+i*stride)
  if typ==1 and off+sz<=len(b):segments.append((va,va+sz,off))
 def read(va):
  for lo,hi,off in segments:
   if lo<=va and va+4<=hi:return struct.unpack_from("<I",b,off+va-lo)[0]
  raise ValueError(f"Unmapped original VA {va:#x}")
 return read
def audit(path):
 read=elf_words(path)
 rows=[decode_word(a,read(a)) for lo,hi in RANGES for a in range(lo,hi,4)]
 ops=[r for r in rows if r["family"]!="other"]
 return {"format":"ae3/original-navrange-vu-atlas/v1","original_words":len(rows),
         "cop2_words":len(ops),"counts":dict(sorted(Counter(r["op"] for r in ops).items())),
         "instructions":ops}
if __name__=="__main__":
 p=argparse.ArgumentParser()
 p.add_argument("elf",type=Path);p.add_argument("--json",action="store_true")
 a=p.parse_args(); r=audit(a.elf)
 if a.json:print(json.dumps(r,indent=2))
 else:
  print("Original words:",r["original_words"],"decoded COP2:",r["cop2_words"])
  for op,c in r["counts"].items():print(op,c)
