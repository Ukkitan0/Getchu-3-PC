#!/usr/bin/env python3
"""Audit private USA SCUS_975.01 4x4 inverse and retail caller data flow."""
import argparse,hashlib,struct
from collections import Counter
from pathlib import Path
ELF_SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
REGION_SHA="c9d01ad7cda7a4fa5dc9108705d29dec03dd66a8ec73de697c7028351eb40747"
START,END=0x359f44,0x35a17c
def verify(path):
    b=path.read_bytes()
    if hashlib.sha256(b).hexdigest()!=ELF_SHA:
        raise ValueError("Wrong retail original USA ELF")
    if b[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",b,18)[0]!=8:
        raise ValueError("Expected MIPS ELF32 LE")
    ph=struct.unpack_from("<I",b,28)[0]
    stride,num=struct.unpack_from("<HH",b,42)
    seg=[]
    for i in range(num):
        typ,off,va,_,sz,_,_,_=struct.unpack_from("<8I",b,ph+i*stride)
        if typ==1 and off+sz<=len(b):seg.append((va,va+sz,off))
    def read(lo,hi):
        for a,z,off in seg:
            if a<=lo and hi<=z:return b[off+lo-a:off+hi-a]
        raise ValueError("Original VA unmapped "+hex(lo))
    def w(a):return struct.unpack("<I",read(a,a+4))[0]
    if hashlib.sha256(read(START,END)).hexdigest()!=REGION_SHA:
        raise ValueError("Retail inverse code region differs")
    words=[w(a) for a in range(START,END,4)]
    assert len(words)==142
    assert Counter(x>>26 for x in words)==Counter({0x12:132,0x36:4,0x3e:4,0:2})
    for addr,vf,offset in ((0x359f44,4,48),(0x359f48,5,32),
                           (0x359f54,2,16),(0x359f68,3,0)):
        op=w(addr)
        assert op>>26==0x36 and ((op>>21)&31,(op>>16)&31,op&65535)==(5,vf,offset)
    # Source VDIV Q = VF0.w / VF3.x, followed by VWAITQ.
    assert w(0x35a14c)==0x4a6303bc and w(0x35a150)==0x4a0003bf
    for addr,fs,fd in ((0x35a154,13,4),(0x35a15c,10,1),
                       (0x35a160,12,2),(0x35a164,11,3)):
        op=w(addr)
        assert op>>26==0x12 and op&63==28 and (op>>21)&15==15
        assert ((op>>11)&31,(op>>6)&31)==(fs,fd)
    assert w(0x35a158)==0x0080102d
    assert w(0x35a174)==0x03e00008
    for addr,vf,offset in ((0x35a168,4,48),(0x35a16c,1,0),
                           (0x35a170,2,16),(0x35a178,3,32)):
        op=w(addr)
        assert op>>26==0x3e and ((op>>21)&31,(op>>16)&31,op&65535)==(4,vf,offset)
    opmula=[x for x in words if (x>>26)==0x12 and (x&63)==0x3e and
            ((x&3)|((x>>4)&0x7c))==46]
    opmsub=[x for x in words if (x>>26)==0x12 and (x&63)==0x2e]
    assert len(opmula)==12 and len(opmsub)==12
    # Retail 0x21EFA4 constructs original quaternion matrix, then
    # copies four 128-bit columns verbatim before the inverse call.
    build=w(0x21efa4)
    assert build>>26==3 and ((build&0x3ffffff)<<2)==0x359e74
    for addr,expected in ((0x21efb0,0x784f0000),(0x21efb8,0x7faf0000),
                          (0x21efbc,0x784f0010),(0x21efc0,0x7faf0010),
                          (0x21efc4,0x784f0020),(0x21efc8,0x7faf0020),
                          (0x21efcc,0x784f0030),(0x21efd4,0x7faf0030),
                          (0x21efe0,0x03a0282d)):
        assert w(addr)==expected,hex(addr)
    inverseCall=w(0x21efdc)
    assert inverseCall>>26==3 and ((inverseCall&0x3ffffff)<<2)==START
    print("PASS: 142 original instructions (132 COP2, 4 LQC2, 4 SQC2)")
    print("PASS: 24 original VOP cross-product cofactor operations")
    print("PASS: caller quaternion matrix copy, inverse, Q division, return delay slot")
    print("LIMIT: native scalar cofactor result not bit-exact PS2 VU0")
if __name__=="__main__":
    p=argparse.ArgumentParser();p.add_argument("elf",type=Path)
    verify(p.parse_args().elf)
