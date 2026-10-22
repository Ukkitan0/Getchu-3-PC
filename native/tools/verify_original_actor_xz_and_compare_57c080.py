#!/usr/bin/env python3
"""Audit original actor XZ proximity and packed controller comparison.

Reads owner-private USA SCUS_975.01; prints instruction metadata only.
No private original executable or resource bytes are committed.
"""
from __future__ import annotations
import argparse
import hashlib
import struct
from pathlib import Path

ELF_SHA = "5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
XZ_SHA = "b8bd2a93fc986aa2b848782734ea73fb0d3842f51bd7056e06e6fa35eeba171a"
COMPARE_SHA = "1c2f033d74ea08980d19bd8a54cd5f7ddc0d1eba4861af8b774df17fced955c3"

def verify(path: Path) -> dict[str,object]:
    b = path.read_bytes()
    assert hashlib.sha256(b).hexdigest()==ELF_SHA, "Original USA executable SHA differs"
    assert b[:7]==b"\x7fELF\x01\x01\x01" and struct.unpack_from("<H",b,18)[0]==8
    ph=struct.unpack_from("<I",b,28)[0]
    stride,n=struct.unpack_from("<HH",b,42)
    segments=[]
    for i in range(n):
        typ,off,va,_,size,_,_,_=struct.unpack_from("<8I",b,ph+i*stride)
        if typ==1:
            assert off+size<=len(b)
            segments.append((va,va+size,off))
    def read(va,length):
        for lo,hi,off in segments:
            if lo<=va and va+length<=hi:
                return b[off+va-lo:off+va-lo+length]
        raise AssertionError(f"Original VA {va:#x} unmapped")
    def word(va): return struct.unpack("<I",read(va,4))[0]
    assert hashlib.sha256(read(0x57c07c,0x70)).hexdigest()==XZ_SHA
    assert hashlib.sha256(read(0x57c1d4,0x40)).hexdigest()==COMPARE_SHA
    opcodes={
        0x57c07c:0x70029ca9, # original PCPYUD after 0x21B854
        0x57c080:0x48b30800, # QMTC2 s3,VF1
        0x57c084:0x0000702d, # t6 zero for MMI source shuffle
        0x57c088:0x48b41000, # QMTC2 s4,VF2
        0x57c08c:0x240d0001, # t5 initially true
        0x57c090:0x4be2086c, # VSUB VF1,VF1,VF2 xyzw
        0x57c094:0x482f0800, # QMFC2 t7,VF1
        0x57c098:0x71cf7488, # PEXTLW t6,t6,t7
        0x57c09c:0x71ef7ba9, # PCPYUD t7,t7,t7
        0x57c0a0:0x71ee7b89, # PCPYLD t7,t7,t6
        0x57c0a4:0x48af1800, # QMTC2 t7,VF3
        0x57c0a8:0x4be3186a, # VMUL.xyzw VF1,VF3,VF3
        0x57c0b4:0xc5e10000, # LWC1 f1,original 0x708A44 (2.0)
        0x57c0b8:0x4ba10841, # VADDy xyw
        0x57c0bc:0x4be10842, # VADDz xyzw
        0x57c0c0:0x482e0800, # QMFC2 t6,VF1
        0x57c0c8:0x448e0000, # MTC1 t6,f0
        0x57c0d4:0x46000004, # SQRT.S f0,f0
        0x57c0d8:0x46010034, # C.OLT.S f0,f1
        0x57c0dc:0x45020001, # BC1FL t5 clearing in delay slot
        0x57c0e0:0x0000682d,
        0x57c0e4:0x31af00ff,
        0x57c0e8:0x11e0000d,
        0x57c1d4:0x924f0030, # variant byte+0x30
        0x57c1d8:0x11e00009,
        0x57c1e0:0x7a4e0020, # LQ t6,+0x20
        0x57c1e4:0x7a4f0040, # LQ t7,+0x40
        0x57c1e8:0x71ee78a8, # PCEQW t7,t7,t6
        0x57c1ec:0x700f7dc8, # PPACH t7,zero,t7
        0x57c1f0:0x000f7827, # NOR
        0x57c1f4:0x2def0001, # SLTIU
        0x57c1f8:0x51e00003, # BEQZL
        0x57c1fc:0x24020002,
        0x57c200:0x240d0001,
        0x57c204:0x24020002,
        0x57c208:0x240f0001,
        0x57c20c:0x1000ff8b,
        0x57c210:0x01ed100a, # MOVZ v0,t7,t5
    }
    for at,expect in opcodes.items():
        assert word(at)==expect,f"Original opcode mismatch {at:#x}"
    assert word(0x708a44)==0x40000000, "Original scalar threshold differs"
    def fields(at):
        x=word(at)
        return ((x>>21)&31,(x>>16)&31,(x>>11)&31,(x>>6)&31,x&63)
    assert fields(0x57c098)==(14,15,14,18,8)   # PEXTLW
    assert fields(0x57c09c)==(15,15,15,14,41)  # PCPYUD
    assert fields(0x57c0a0)==(15,14,15,14,9)   # PCPYLD
    assert fields(0x57c1e8)==(15,14,15,2,40)   # PCEQW
    assert fields(0x57c1ec)==(0,15,15,23,8)    # PPACH
    for at,op,fd,fs,ft,mask in [
        (0x57c090,0x2c,1,1,2,15),(0x57c0a8,0x2a,1,3,3,15),
        (0x57c0b8,0x01,1,1,1,13),(0x57c0bc,0x02,1,1,1,15)]:
        w=word(at)
        assert w>>26==18 and (w&63,(w>>6)&31,(w>>11)&31,
               (w>>16)&31,(w>>21)&15)==(op,fd,fs,ft,mask),hex(at)
    def target(at):
        raw=word(at)&0xffff
        signed=raw if raw<32768 else raw-65536
        return at+4+4*signed
    for at,dst in [(0x57c0dc,0x57c0e4),(0x57c0e8,0x57c120),
                   (0x57c1d8,0x57c200),(0x57c1f8,0x57c208),
                   (0x57c20c,0x57c03c)]:
        assert target(at)==dst,hex(at)
    return {"original_elf_sha256":ELF_SHA,
            "xz_instruction_words":28,"compare_instruction_words":16,
            "explicit_source_opcode_sites":len(opcodes),
            "original_xz_threshold_bits":"0x40000000",
            "full_windows_game_boot_verified":False}

if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("original_USA_elf",type=Path)
    print(verify(p.parse_args().original_USA_elf))
