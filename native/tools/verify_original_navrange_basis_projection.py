#!/usr/bin/env python3
"""Verify original retail SCUS_975.01 VU0 basis normalization and dot paths.

Reads the owner's original ELF privately. Does not redistribute game bytes.
"""
import argparse
import hashlib
import struct
from pathlib import Path

SHA = '5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
ANCHORS = {
    # Basis 0: VMUL squares, 3-coordinate length, VRSQRT and dot
    0x21F028:0x4BE5292A,0x21F040:0x4BA42101,0x21F058:0x4BE42102,
    0x21F064:0x4A6403BE,0x21F07C:0x4A0003BF,0x21F080:0x4BE0291C,
    0x21F0B0:0x4BE4106A,0x21F0D0:0x4BA10841,0x21F0DC:0x4BE10842,
    # Basis 1 normalization and dot
    0x21F0AC:0x48AD2800,0x21F0B8:0x4BE528EA,
    0x21F0D8:0x4BA318C1,0x21F0E0:0x4BE318C2,
    0x21F0E8:0x4A6303BE,0x21F11C:0x4A0003BF,0x21F120:0x4BE028DC,
    0x21F12C:0x4BE3106A,0x21F130:0x4BA10841,0x21F134:0x4BE10842,
    # Basis 2 normalization and dot
    0x21F174:0x48AF1800,0x21F180:0x4BE3186A,
    0x21F190:0x4BA10841,0x21F194:0x4BE10842,
    0x21F198:0x4A6103BE,0x21F19C:0x4A0003BF,
    0x21F1A0:0x4BE0185C,0x21F1A8:0xDBA20380,
    0x21F1B0:0x4BE110AA,0x21F1B4:0x4BA21081,0x21F1B8:0x4BE21082,
}
REGION_START = 0x21F020
REGION_END = 0x21F1BC
REGION_SHA = '789a41c080cc5a288c35bc7e5955f9bba4ce9889a4d729b053e8e70795d972a5'

def verify(elf):
    b=elf.read_bytes()
    if hashlib.sha256(b).hexdigest()!=SHA:
        raise ValueError('Wrong USA retail original executable')
    if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
        raise ValueError('Expected little-endian original MIPS ELF32')
    phoff=struct.unpack_from('<I',b,28)[0]
    phentsize,phnum=struct.unpack_from('<HH',b,42)
    segments=[]
    for n in range(phnum):
        typ,off,va,_,filesize,_,_,_=struct.unpack_from('<8I',b,phoff+n*phentsize)
        if typ==1 and off+filesize<=len(b):segments.append((va,va+filesize,off))
    def read(va,size):
        for low,high,off in segments:
            if low<=va and va+size<=high:
                return b[off+va-low:off+va-low+size]
        raise ValueError(f'Unmapped original address {va:#x}')
    def word(va):return struct.unpack('<I',read(va,4))[0]
    whole=read(REGION_START,REGION_END-REGION_START)
    h=hashlib.sha256(whole).hexdigest()
    if h!=REGION_SHA:
        raise ValueError('Original normalization/dot region differs')
    for address,op in ANCHORS.items():
        if word(address)!=op:raise ValueError(f'Original opcode mismatch {address:#x}')
    for address,ft in ((0x21F064,4),(0x21F0E8,3),(0x21F198,1)):
        op=word(address)
        assert (op>>26)==0x12 and (op&0x7ff)==0x3be
        assert ((op>>11)&31,(op>>16)&31)==(0,ft)
        assert ((op>>23)&3,(op>>21)&3)==(0,3),hex(address)
    for address,fs,fd in ((0x21F080,5,4),(0x21F120,5,3),(0x21F1A0,3,1)):
        op=word(address)
        assert op>>26==0x12 and (op&63)==28
        assert ((op>>11)&31,(op>>6)&31,(op>>21)&15)==(fs,fd,15)
    for a in (0x21F0D0,0x21F0D8,0x21F130,0x21F190,0x21F1B4):
        assert (word(a)>>21)&15==13,hex(a)
    for a in (0x21F058,0x21F0DC,0x21F0E0,0x21F134,0x21F194,0x21F1B8):
        assert (word(a)>>21)&15==15,hex(a)
    assert (word(0x21F1A8)>>26,(word(0x21F1A8)>>21)&31,
            (word(0x21F1A8)>>16)&31,word(0x21F1A8)&0xffff)==(0x36,29,2,0x380)
    print(f'PASS: {len(ANCHORS)} exact opcode anchors; {len(whole)//4} whole original instruction words')
    print('PASS: three VRSQRT Q selectors, VMULq and original dot reduction masks')
    print('Source region SHA-256:',h)
    print('UNRESOLVED: three prepared source-vector MMI input mappings and PS2 VU0 numeric parity')

if __name__=='__main__':
    p=argparse.ArgumentParser()
    p.add_argument('original_elf',type=Path)
    verify(p.parse_args().original_elf)
