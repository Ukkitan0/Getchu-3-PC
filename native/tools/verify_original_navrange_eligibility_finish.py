#!/usr/bin/env python3
"""Verify source 0x21F3C8 optional vector output and normal navrange no-write path.

Private retail ELF remains on the user's computer; no PS2 emulation.
"""
import argparse
import hashlib
import struct
from pathlib import Path
SHA = '5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
EXPECTED = {
    0x21f3c8:0x13c0000c, # BEQ S8,zero,target 0x21F3FC
    0x21f3cc:0xdba20360, # LQC2 VF2,sp+0x360 in delay slot
    0x21f3d0:0xdba10000, # LQC2 VF1,sp+0x0
    0x21f3d4:0x4be209bc, # VMULAx ACC,VF1,VF2.x
    0x21f3d8:0xdba10010, # LQC2 VF1,sp+0x10
    0x21f3dc:0x4be208bd, # VMADDAy ACC,VF1,VF2.y
    0x21f3e0:0xdba10020, # LQC2 VF1,sp+0x20
    0x21f3e4:0x4be208be, # VMADDAz ACC,VF1,VF2.z
    0x21f3e8:0xdba10030, # LQC2 VF1,sp+0x30
    0x21f3ec:0x4be2084b, # VMADDw VF1,ACC,VF1,VF2.w
    0x21f3f0:0x48b71000, # QMTC2 s7,VF2
    0x21f3f4:0x4be20868, # VADD.xyzw VF1,VF1,VF2
    0x21f3f8:0xfbc10000, # SQC2 VF1,0(s8)
    0x21f3fc:0x4600a006, # MOV.S F0,F20
    0x21ef6c:0x0100f02d, # DADDU s8,t0,zero
    0x21f754:0x0000402d, # DADDU t0,zero,zero: caller t0=0
    0x21f750:0x0c087bcf, # JAL 0x21EF3C
}
def verify(elf):
    buf=elf.read_bytes()
    if hashlib.sha256(buf).hexdigest()!=SHA:
        raise ValueError('Wrong original USA SCUS_975.01')
    if buf[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',buf,18)[0]!=8:
        raise ValueError('Expected little-endian MIPS ELF32')
    start=struct.unpack_from('<I',buf,28)[0]
    stride,n=struct.unpack_from('<HH',buf,42)
    segments=[]
    for i in range(n):
        typ,off,va,_,size,_,_,_=struct.unpack_from('<8I',buf,start+i*stride)
        if typ==1 and off+size<=len(buf):segments.append((va,va+size,off))
    def word(va):
        for lo,hi,off in segments:
            if lo<=va and va+4<=hi:
                return struct.unpack_from('<I',buf,off+va-lo)[0]
        raise ValueError('Unmapped original address '+hex(va))
    for addr,expected in EXPECTED.items():
        if word(addr)!=expected:
            raise ValueError(f'Original opcode mismatch at {addr:#x}: {word(addr):#x}')
    branch=word(0x21f3c8)&0xffff
    if branch&0x8000:branch-=0x10000
    assert 0x21f3c8+4+4*branch==0x21f3fc
    for a,reg,offset in ((0x21f3cc,2,0x360),(0x21f3d0,1,0x0),
                         (0x21f3d8,1,0x10),(0x21f3e0,1,0x20),
                         (0x21f3e8,1,0x30)):
        w=word(a)
        assert (w>>26,(w>>16)&31,(w>>21)&31,w&0xffff)==(0x36,reg,29,offset)
    for a,func in ((0x21f3d4,0x3c),(0x21f3dc,0x3d),
                   (0x21f3e4,0x3e),(0x21f3ec,0x0b),(0x21f3f4,0x28)):
        w=word(a)
        assert w>>26==0x12 and (w&63)==func
    print('PASS:',len(EXPECTED),'retail opcode checks, vector input offsets and optional store')
    print('PASS: caller t0=0 means no vector write on navrange polygon iteration')
    print('LIMIT: source score F20 still needs complete original eligibility recovery')
if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('elf',type=Path)
    verify(parser.parse_args().elf)
