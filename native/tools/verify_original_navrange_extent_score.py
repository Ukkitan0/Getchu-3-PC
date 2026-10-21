#!/usr/bin/env python3
"""Verify original USA SCUS_975.01 navrange F20 source dataflow.

Static ELF instruction checks only; no copyrighted game bytes are published.
"""
import argparse
import hashlib
import struct
from pathlib import Path
ELF_SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
FUNCTION_SHA="fe4c269c68019fbf5a42174e3d61e036f5f4dd80c0c9972e7e49b53a9e4a221a"
START,END=0x21ef3c,0x21f5a4
ANCHORS={
0x21ef5c:0x700684a9, # POR s0,zero,a2 (full quaternion)
0x21ef9c:0x70102ca9, # POR a1,zero,s0
0x21efa4:0x0c0d679d,0x21efdc:0x0c0d67d1,
0x21efe8:0x7bad0370, # original +0x40 source extent
0x21effc:0x06180000,0x21f000:0x700d7ee8,
0x21f008:0x724f74c8,0x21f010:0x726e74c8,
0x21f01c:0x48ae2800,
0x21f094:0x05f80000,0x21f098:0x700d7ee8,
0x21f0a0:0x71f26cc8,0x21f0a4:0x726d6cc8,
0x21f0ac:0x48ad2800,
0x21f15c:0x05f80000,0x21f160:0x700e7ee8,
0x21f168:0x724f7cc8,0x21f16c:0x71f37cc8,
0x21f174:0x48af1800,
0x21f1ac:0x4492a000, # zero initializes F20
0x21f1e0:0x06180000,0x21f1e4:0x700e7ee8,
0x21f208:0x46000b07,0x21f21c:0x460c0034,
0x21f238:0x11e000bc,
0x21f284:0x05f80000,0x21f288:0x700d76e8,
0x21f2c8:0x460c0034,0x21f2e0:0x11e00073,
0x21f330:0x05f80000,0x21f334:0x700d76e8,
0x21f374:0x460c0034,0x21f38c:0x11e00029,
0x21f3fc:0x4600a006,
0x21f750:0x0c087bcf,0x21f754:0x0000402d
}
def ppacw(rs,rt):return [rt[0],rt[2],rs[0],rs[2]]
def axes(ext):
    # Fixed retail MTSAB/QFSRV/PPACW lane permutations:
    # SLL preserves the source GPR's upper 64-bit half.
    zero=[0,0,0,0]
    def shifted(n):
        r=ext+[0,0,0,0]
        v=r[n:n+4]
        return [v[0],0xffffffff if v[0]&0x80000000 else 0,v[2],v[3]]
    x=ppacw(zero,ppacw(zero,shifted(0)))
    y=ppacw(zero,ppacw(shifted(1),zero))
    z=ppacw(ppacw(zero,shifted(2)),zero)
    return x,y,z
def verify(path):
    b=path.read_bytes()
    if hashlib.sha256(b).hexdigest()!=ELF_SHA:raise ValueError("Wrong original USA ELF")
    if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
        raise ValueError("Expected MIPS ELF32 little-endian")
    ph=struct.unpack_from('<I',b,28)[0]
    stride,num=struct.unpack_from('<HH',b,42)
    segments=[]
    for i in range(num):
        typ,off,va,_,size,_,_,_=struct.unpack_from('<8I',b,ph+i*stride)
        if typ==1 and off+size<=len(b):segments.append((va,va+size,off))
    def read(va,n):
        for lo,hi,off in segments:
            if lo<=va and va+n<=hi:return b[off+va-lo:off+va-lo+n]
        raise ValueError("Original VA unmapped: "+hex(va))
    def word(va):return struct.unpack('<I',read(va,4))[0]
    if hashlib.sha256(read(START,END-START)).hexdigest()!=FUNCTION_SHA:
        raise ValueError("Original 410-instruction eligibility routine differs")
    for va,expected in ANCHORS.items():
        if word(va)!=expected:raise ValueError("Source opcode mismatch "+hex(va))
    for va in (0x21ef5c,0x21ef9c):
        w=word(va)
        # MMI3 subtype 18 is POR, NOT PCPYUD (subtype 14).
        assert w>>26==0x1c and w&63==41 and ((w>>6)&31)==18
    for va in (0x21f008,0x21f010,0x21f0a0,0x21f0a4,0x21f168,0x21f16c):
        w=word(va)
        assert w>>26==0x1c and w&63==8 and ((w>>6)&31)==19
    for va in (0x21effc,0x21f094,0x21f15c):
        w=word(va)
        assert w>>26==1 and (w>>16)&31==24 and (w&0xffff)==0
    for ext in ([0x3f800000,0x40000000,0x40400000,0x40800000],
                [0x80000000,0x7fc01234,0xff800000,0x7f800000],
                [1,2,3,4]):
        assert axes(ext)==([ext[0],0,0,0],[0,ext[1],0,0],[0,0,ext[2],0])
    print("PASS:",(END-START)//4,"complete original function words,",
          len(ANCHORS),"exact opcode anchors")
    print("PASS: full-quaternion POR, three PPACW extent-axis lanes, COP1 F20 score")
    print("LIMIT: no live retail navrange fixture or bit-exact PS2-VU0 differential")
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('original_elf',type=Path)
    verify(p.parse_args().original_elf)
