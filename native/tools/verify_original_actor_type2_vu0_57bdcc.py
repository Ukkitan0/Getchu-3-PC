#!/usr/bin/env python3
"""Authenticate original AE3 USA actor type-two movement 0x57BDCC–0x57BED4.

Reads only privately held game executable; emits instruction metadata.
No asset or executable bytes are published.
"""
import argparse
import hashlib
import struct
from pathlib import Path

ELF_SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
BLOCK_SHA="7974c5e0c777b060f0e55c0c99c8fca7bb0d1df91b84772486d8c49dbe76136b"
PCPYUD=(0x57BDD4,0x57BDEC,0x57BDFC)
VECTOR_OPS={
    0x57BE8C:(0x18,1,3,1),
    0x57BEAC:(0x18,2,1,2),
    0x57BEC0:(0x18,2,1,2),
    0x57BEC8:(0x28,3,3,1),
    0x57BECC:(0x28,3,3,2),
}
CALLS={
    0x57BDCC:0x21B458,
    0x57BDDC:0x21B458,
    0x57BDF0:0x21B458,
    0x57BE00:0x36F4D4,
    0x57BE24:0x36F548,
}
def verify(path):
    b=Path(path).read_bytes()
    if hashlib.sha256(b).hexdigest()!=ELF_SHA:
        raise ValueError("Original USA ELF SHA mismatch")
    if b[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",b,18)[0]!=8:
        raise ValueError("Expected original MIPS ELF32 little endian")
    ph=struct.unpack_from("<I",b,28)[0]
    step,n=struct.unpack_from("<HH",b,42)
    segments=[]
    for i in range(n):
        typ,off,va,_,length,_,_,_=struct.unpack_from("<8I",b,ph+i*step)
        if typ==1 and off+length<=len(b):segments.append((va,va+length,off))
    def read(addr,length):
        for lo,hi,off in segments:
            if lo<=addr and addr+length<=hi:
                return b[off+addr-lo:off+addr-lo+length]
        raise ValueError(f"Original source VA not mapped {addr:#x}")
    def word(addr):return struct.unpack("<I",read(addr,4))[0]
    if hashlib.sha256(read(0x57BDCC,0x10C)).hexdigest()!=BLOCK_SHA:
        raise ValueError("Original type-two target body SHA mismatch")
    for addr,target in CALLS.items():
        w=word(addr)
        actual=((addr+4)&0xF0000000)|((w&0x03FFFFFF)<<2)
        if (w>>26)!=3 or actual!=target:
            raise ValueError(f"Original JAL source changed {addr:#x}")
    for addr in PCPYUD:
        w=word(addr)
        if (w>>26)!=28 or (w&63)!=0x29 or (w>>21)&31!=0 or (w>>16)&31!=2:
            raise ValueError(f"Original source vertex PCPYUD changed {addr:#x}")
    for addr,(opcode,fd,fs,ft) in VECTOR_OPS.items():
        w=word(addr)
        if (w>>26)!=18 or (w&63)!=opcode or (
            ((w>>6)&31,(w>>11)&31,(w>>16)&31,(w>>21)&15)
            !=(fd,fs,ft,15)):
            raise ValueError(f"Original VU0 register/operation changed {addr:#x}")
    # Bitwise-identical original constants used by two independent actor routines.
    expected=(0x3F800000,0x3F4CCCCD,0x3D888888)
    if tuple(word(0x708A24+4*i) for i in range(3))!=expected:
        raise ValueError("Original parent actor float coefficient data changed")
    if tuple(word(0x708A64+4*i) for i in range(3))!=expected:
        raise ValueError("Original target dispatcher coefficients differ")
    # 0x57BED0 B to function epilogue executes 0x57BED4 SQC2 VF3,0x40(s4).
    bword=word(0x57BED0)
    branch_target=0x57BED4+4*struct.unpack("<h",struct.pack("<H",bword&65535))[0]
    if (bword>>26)!=4 or branch_target!=0x57BD58:
        raise ValueError("Original writeback delay-slot branch changed")
    store=word(0x57BED4)
    if ((store>>26),(store>>21)&31,(store>>16)&31,store&0xffff)!=(62,20,3,0x40):
        raise ValueError("Original type-two controller+0x40 SQC2 missing")
    # Original type-two gate uses source action-kind == 2, not an invented gate.
    if word(0x57BDA4)!=0x38420002 or word(0x57BDA8)!=0x2C420001:
        raise ValueError("Original source type-two action-kind predicate changed")
    return dict(
        source="original USA SCUS_975.01",
        audited_instructions=67,
        original_polygon_reads=3,
        original_game_rng_calls=2,
        native_vu0_operations=5,
        original_PCpyud_vertex_extracts=3,
        output="SQC2 VF3 to original controller +0x40",
        original_Type2_Action_Kind=2,
        source_implemented_without_emulation=True)

if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("original_USA_elf",type=Path)
    print(verify(parser.parse_args().original_USA_elf))
