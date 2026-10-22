#!/usr/bin/env python3
"""Verify the direct native 0x57CA68 VU0 transform against owner-private USA ELF.

Produces only instruction metadata, not proprietary ELF/game-resource bytes.
"""
import argparse
import hashlib
import struct
from pathlib import Path

ORIGINAL_SHA = "5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
FALLBACK_SHA = "6e33608fffc7f8f6980c8eefaee4f856925ba2b0d60fa7fd095dbc8f9c80c886"
PCPYUD = {
    0x57CA78: (23, 5),  # owner a1, stored in s7
    0x57CA88: (22, 6),  # controller a2, stored in s6
    0x57CAB0: (21, 2),  # vertex zero, stored in s5
    0x57CAC8: (20, 2),  # vertex one, stored in s4
    0x57CAD8: (19, 2),  # vertex two, stored in s3
    0x57CCD4: (17, 16), # winning repeated candidate s0->s1
    0x57CCE0: (2, 17),  # source return s1->v0, lower 64 zero
}
TRANSFER = {
    0x57CB7C: (1, 17, 3),  # QMFC2 s1,vf3
    0x57CBC0: (1, 5, 3),   # QMFC2 a1,vf3
    0x57CBC8: (1, 4, 2),   # QMFC2 a0,vf2
    0x57CC78: (1, 16, 2), # QMFC2 s0,vf2
    0x57CCB8: (1, 5, 3),  # QMFC2 a1,vf3
    0x57CCC0: (1, 4, 2),  # QMFC2 a0,vf2
}
OPS = {
    0x18:"VMULx", 0x1c:"VMULq",0x28:"VADD",0x2a:"VMUL",0x2c:"VSUB",
    1:"VADDy",2:"VADDz",
}
SPECIAL={0x30:"VMOVE",0x3a:"VRINIT",0x3b:"VRXOR"}
INITIAL=("VMULx","VMULx","VADD","VMULx","VADD","VMOVE","VSUB","VMUL",
         "VADDy","VADDz","VRINIT","VRXOR","VMULq","VSUB","VMUL",
         "VADDy","VADDz","VRINIT","VRXOR","VMULq")
REPEATED=("VMULx","VMULx","VADD","VMULx","VADD","VSUB","VMUL",
          "VADDy","VADDz","VRINIT","VRXOR","VMULq","VSUB","VMUL",
          "VADDy","VADDz","VRINIT","VRXOR","VMULq")
def verify(path: Path) -> None:
    data=path.read_bytes()
    assert hashlib.sha256(data).hexdigest()==ORIGINAL_SHA,"Private original ELF SHA mismatch"
    assert data[:7]==b"\x7fELF\x01\x01\x01","Expected original ELF32 little-endian"
    assert struct.unpack_from("<H",data,18)[0]==8,"Expected MIPS ELF"
    ph=struct.unpack_from("<I",data,28)[0]
    stride,count=struct.unpack_from("<HH",data,42)
    load=[]
    for i in range(count):
        t,off,va,_,size,_,_,_=struct.unpack_from("<8I",data,ph+i*stride)
        if t==1:load.append((va,va+size,off))
    def read(va,n):
        for lo,hi,off in load:
            if lo<=va and va+n<=hi:
                return data[off+va-lo:off+va-lo+n]
        raise AssertionError(f"Unmapped original ELF VA {va:#x}")
    def word(va):return struct.unpack("<I",read(va,4))[0]
    assert hashlib.sha256(read(0x57CA68,0x2b4)).hexdigest()==FALLBACK_SHA
    for at,(rd,rt) in PCPYUD.items():
        w=word(at)
        assert w>>26==28 and (w&63)==0x29 and ((w>>6)&31)==18
        assert ((w>>21)&31,(w>>16)&31,(w>>11)&31)==(0,rt,rd),hex(at)
    for at,(rs,rt,rd) in TRANSFER.items():
        w=word(at)
        assert w>>26==18 and ((w>>21)&31,(w>>16)&31,(w>>11)&31)==(rs,rt,rd),hex(at)
    def macro(at):
        w=word(at)
        if w>>26!=18 or ((w>>21)&31)<16:return None
        op=w&63
        if op>=60:
            sub=(w&3)|((w>>4)&0x7c)
            n=SPECIAL.get(sub)
        else:n=OPS.get(op)
        assert n is not None,hex(at)
        return n
    first=tuple(v for at in range(0x57CB4C,0x57CBBC,4) if (v:=macro(at)))
    later=tuple(v for at in range(0x57CC48,0x57CCB4,4) if (v:=macro(at)))
    assert first==INITIAL,(first,INITIAL)
    assert later==REPEATED,(later,REPEATED)
    # Verify the 0x57C7CC TWO projection phases:
    # the first JAL 0x4C1B3C at 0x57C814 uses full incoming a1;
    # subsequent 0x57C890 JAL follows packed 0x57C88C PCPYUD.
    assert word(0x57C7E4)==0x7005aca9,"Saved incoming a1 upper half differs"
    assert word(0x57C814)==0x0c1306cf,"Original first ray call changed"
    assert word(0x57C890)==0x0c1306cf,"Original second ray call changed"
    assert word(0x57C88C)==0x70152ca9,"Source follow-up A1 mask changed"
    # Verify original source PCPYUD arguments and 128-bit target transfers.
    # 0x57C994/C9A8/C9B8: three source vertex PCPYUDs.
    # 0x57C894: caller projection operand PCPYUD a2,zero,v0.
    for at,rd,rt in ((0x57C994,17,2),(0x57C9A8,19,2),
                     (0x57C9B8,18,2),(0x57C894,6,2),
                     (0x57C880,18,2),(0x57C88C,5,21)):
        x=word(at)
        assert (x>>26,x&63,(x>>6)&31,
                (x>>21)&31,(x>>16)&31,(x>>11)&31)==(28,0x29,18,0,rt,rd),hex(at)
    # Verify the connected full-width target-transfer path, not just VU0 math.
    # 0x57C948 QMFC2 v0,VF1; 0x57C988 QMTC2 v0,VF1;
    # 0x57C894 PCPYUD a2,zero,v0. All 128 bits matter.
    assert word(0x57C948)==0x48220800,"Original QMFC2 return changed"
    assert word(0x57C988)==0x48a20800,"Original full-width QMTC2 changed"
    assert word(0x57C894)==0x700234a9,"Original subsequent PCPYUD changed"
    assert word(0x57CA60)==0x1000ffb9,"Variant-0 return branch changed"
    # Exact retail VU0 operand ledger: (source VA, VF destination, VF fs,
    # VF ft, xyzw mask). This is verified against genuine instruction bits,
    # not a replacement vector algorithm or game code bytes.
    source_ops=(
      (0x57CB4C,1,3,1,15),(0x57CB58,2,4,2,15),
      (0x57CB68,1,1,2,15),(0x57CB6C,3,4,5,15),
      (0x57CB70,3,1,3,15),(0x57CB80,3,1,3,15),
      (0x57CB84,1,3,3,15),(0x57CB88,1,1,1,13),
      (0x57CB8C,1,1,1,15),(0x57CB9C,3,3,0,15),
      (0x57CBA0,2,2,4,15),(0x57CBA4,1,2,2,15),
      (0x57CBA8,1,1,1,13),(0x57CBAC,1,1,1,15),
      (0x57CBB8,2,2,0,15),
      (0x57CC48,2,4,1,15),(0x57CC58,1,5,3,15),
      (0x57CC64,2,2,1,15),(0x57CC68,3,4,5,15),
      (0x57CC70,2,2,3,15),(0x57CC74,3,1,2,15),
      (0x57CC7C,1,3,3,15),(0x57CC80,1,1,1,13),
      (0x57CC84,1,1,1,15),(0x57CC94,3,3,0,15),
      (0x57CC98,2,2,4,15),(0x57CC9C,1,2,2,15),
      (0x57CCA0,1,1,1,13),(0x57CCA4,1,1,1,15),
      (0x57CCB0,2,2,0,15),
    )
    for at,fd,fs,ft,mask in source_ops:
        x=word(at)
        assert ((x>>6)&31,(x>>11)&31,(x>>16)&31,
                (x>>21)&15)==(fd,fs,ft,mask),hex(at)
    # VMOVE is special2, so its destination is FT (bits 20..16), not FD.
    assert word(0x57CB74)==0x4be21b3c
    # Four masked VADDy.xyW instructions preserve Z, unlike VADDz.xyzw.
    for at in (0x57CB88,0x57CBA8,0x57CC80,0x57CCA0):
        w=word(at)
        assert macro(at)=="VADDy"
        assert ((w>>21)&15)==0b1101,hex(at)
    for at in (0x57CB9C,0x57CBB8,0x57CC94,0x57CCB0):
        assert macro(at)=="VMULq"
    print("Verified genuine fallback SHA, two VU0 operation sequences,")
    print("7 PCPYUD layouts, 6 QMFC2 operands, 4 masked VADDy and 4 VMULq sites")
if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("original_USA_elf",type=Path)
    verify(parser.parse_args().original_USA_elf)
