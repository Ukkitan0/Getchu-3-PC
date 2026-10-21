#!/usr/bin/env python3
"""Verify original Ape Escape 3 USA resource-to-predicate argument provenance."""
import argparse,hashlib,struct
from pathlib import Path
SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
def verify(path):
    b=path.read_bytes()
    if hashlib.sha256(b).hexdigest()!=SHA:raise ValueError("Unexpected original USA ELF")
    if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
        raise ValueError("Expected original little-endian MIPS ELF32")
    ph=struct.unpack_from('<I',b,28)[0]
    stride,count=struct.unpack_from('<HH',b,42)
    regions=[]
    for i in range(count):
        kind,off,va,_,size,_,_,_=struct.unpack_from('<8I',b,ph+i*stride)
        if kind==1 and off+size<=len(b):regions.append((va,va+size,off))
    def w(addr):
        for lo,hi,off in regions:
            if lo<=addr and addr+4<=hi:
                return struct.unpack_from('<I',b,off+addr-lo)[0]
        raise ValueError(f"Unmapped VA {addr:#x}")
    def lq(addr,rt,base,offset):
        x=w(addr)
        assert (x>>26,(x>>21)&31,(x>>16)&31,x&65535)==(0x1e,base,rt,offset),hex(addr)
    def por(addr,rd,rt):
        x=w(addr)
        assert (x>>26,x&63,(x>>6)&31,(x>>21)&31,(x>>16)&31,(x>>11)&31)==(0x1c,0x29,18,0,rt,rd),hex(addr)
    def jal(addr,target):
        x=w(addr)
        assert x>>26==3 and (((addr+4)&0xf0000000)|((x&0x3ffffff)<<2))==target,hex(addr)
    assert w(0x4e40b4)==0x00a0902d # s2=resource pointer
    lq(0x4e4110,7,18,0x30) # a3
    lq(0x4e4118,6,18,0x40) # a2
    lq(0x4e4120,5,18,0x20) # a1, delay slot of JAL below
    jal(0x4e411c,0x21f6d4)
    for a,rd,rt in ((0x21f6e0,4,5),(0x21f70c,5,6),(0x21f718,21,7),
                    (0x21f71c,20,5),(0x21f720,19,4)):
        por(a,rd,rt)
    jal(0x21f738,0x21b468)
    for a,rd,rt in ((0x21f740,5,20),(0x21f744,7,2),
                    (0x21f748,4,19),(0x21f74c,6,21)):
        por(a,rd,rt)
    jal(0x21f750,0x21ef3c)
    assert w(0x21f754)==0x0000402d # t0=0 delay slot
    for a,rd,rt in ((0x21ef54,23,4),(0x21ef5c,16,6),
                    (0x21ef64,17,7),(0x21ef9c,5,16)):
        por(a,rd,rt)
    jal(0x21efa4,0x359e74)
    jal(0x21efdc,0x359f44)
    print("PASS: original resource +0x20/+0x30/+0x40 source geometry vectors")
    print("PASS: polygon centroid and t0=0, 3 LQ, 13 POR, 5 JAL checks")
    print("LIMIT: 0x21EF3C final VU0 eligibility scalar remains unresolved")
if __name__=='__main__':
    p=argparse.ArgumentParser()
    p.add_argument('retail_elf',type=Path)
    verify(p.parse_args().retail_elf)
