#!/usr/bin/env python3
"""Audit source 0x57BCDC–0x57BD54 actor movement gate in private USA ELF."""
import hashlib
import struct
import argparse
from pathlib import Path

ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
BODY_SHA='c93c29dad7891ab3e2af3a5ad39e9cccc2eebd5d89604e762f043e167dc9fcf5'

def verify(path):
    b=Path(path).read_bytes()
    assert hashlib.sha256(b).hexdigest()==ELF_SHA,'Incorrect retail SCUS_975.01'
    assert b[:7]==b'\x7fELF\x01\x01\x01'
    ph=struct.unpack_from('<I',b,28)[0]
    step,count=struct.unpack_from('<HH',b,42)
    mappings=[]
    for i in range(count):
        t,off,va,_,size,_,_,_=struct.unpack_from('<8I',b,ph+i*step)
        if t==1:mappings.append((va,va+size,off))
    def read(va,length):
        for lo,hi,off in mappings:
            if lo<=va and va+length<=hi:return b[off+va-lo:off+va-lo+length]
        raise AssertionError(f'Original source VA {va:#x} not mapped')
    def w(va):return struct.unpack('<I',read(va,4))[0]
    assert hashlib.sha256(read(0x57bcdc,0x7c)).hexdigest()==BODY_SHA
    anchors={
        0x57bcdc:0x908f0030, # LBU original controller+0x30
        0x57bce0:0x11e00012, # BEQZ skip VU0 math when flag disabled
        0x57bcec:0xd8810020, # LQC2 VF1,controller+0x20
        0x57bcf0:0x8def0014, # LW owner+0x14
        0x57bcf4:0xd9e20040, # LQC2 VF2,owner+0x40
        0x57bcf8:0x4be2086c, # VSUB.xyzw VF1,VF1,VF2
        0x57bcfc:0x48240800, # QMFC2 a0,VF1
        0x57bd00:0x0c11d22b, # JAL original 0x4748AC XYZ dot
        0x57bd04:0x48250800, # QMFC2 a1,VF1 delay
        0x57bd10:0x46000004, # SQRT.S f0,f0
        0x57bd1c:0xc5a10000, # LWC1 f1,0(a1) source threshold
        0x57bd20:0x46010034, # C.OLT.S f0,f1
        0x57bd24:0x45010002, # BC1T -> set true
        0x57bd28:0x240f0001, # positive value in delay slot
        0x57bd2c:0x0000782d, # false gate sets t7=zero
        0x57bd30:0x31ef00ff, # unsigned byte mask
        0x57bd34:0x51e00012, # BEQZL -> skip clear branch
        0x57bd3c:0xffa00000, # SD zero,0(sp)
        0x57bd4c:0xae8f000c, # SW zero to controller+0xc
        0x57bd54:0xae8e0008, # SW zero to controller+0x8
    }
    for va,expected in anchors.items():
        assert w(va)==expected, f'Retail opcode changed at {va:#x}'
    assert w(0x708a20)==0x40000000,'Original radius threshold not 2.0f'
    def branch_target(at):
        imm=struct.unpack('<h',struct.pack('<H',w(at)&0xffff))[0]
        return at+4+imm*4
    assert branch_target(0x57bce0)==0x57bd2c
    assert branch_target(0x57bd24)==0x57bd30
    assert branch_target(0x57bd34)==0x57bd80
    instr=w(0x57bcf8)
    assert (instr&63,(instr>>6)&31,(instr>>11)&31,(instr>>16)&31,(instr>>21)&15)==(0x2c,1,1,2,15)
    assert ((w(0x57bcec)>>26),(w(0x57bcec)>>16)&31,(w(0x57bcec)>>21)&31,w(0x57bcec)&0xffff)==(54,1,4,0x20)
    assert ((w(0x57bcf4)>>26),(w(0x57bcf4)>>16)&31,w(0x57bcf4)&0xffff)==(54,2,0x40)
    assert w(0x57bd00)>>26==3 and (w(0x57bd00)&0x03ffffff)<<2==0x4748ac
    return 'PASS source original 31-instruction 0x57BCDC-57BD54 actor distance/reset gate'
if __name__=='__main__':
    a=argparse.ArgumentParser(description=__doc__)
    a.add_argument('original_USA_elf',type=Path)
    print(verify(a.parse_args().original_USA_elf))
