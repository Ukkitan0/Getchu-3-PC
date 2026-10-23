#!/usr/bin/env python3
"""Audit source retail USA 0x557F54..0x557FC8 VU0 movement epilogue.

The original private SCUS_975.01 must be supplied by the owner.
All 29 original source words, original VRSQRT/WAITQ/MULq,
branch targets and 0x95/100 threshold are checked. The native
ordinary exact subset covers bit1-set, untagged parent, positive
exact source VF1 lengths only. Original 36F8C8 regeneration for
bit1-clear is a separately recovered function.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ELF_SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
START,END=0x557F54,0x557FC8
WINDOW_SHA="9aaaa5e8ebb1c3ddb3612f6271fc20dd137f49da76eabdeaaf70918ba9df0020"
SOURCE_HEX="""
8E0F00B8 31EF0002 11E0004D 46000546 7A0200A0
48A20800 4BE108AA 4BA21081 4BE21082 4A6203BE
4A0003BF 4BE0089C 920F0008 11E0000F 48331000
DA810050 240E0001 4BE2086A 4BA10841 4BE10842
482D0800 000D7800 3C0D0070 448F0800 25AD4B08
C5A00000 46000834 4503FFB1 DFB00010
"""
WORDS={START+i*4:int(w,16) for i,w in enumerate(SOURCE_HEX.split())}
def read(data,va,n):
    ph=struct.unpack_from("<I",data,28)[0]
    stride,count=struct.unpack_from("<HH",data,42)
    for i in range(count):
        kind,off,base,_,length,_,_,_=struct.unpack_from(
            "<8I",data,ph+i*stride)
        if kind==1 and base<=va and va+n<=base+length:
            return data[off+va-base:off+va-base+n]
    raise ValueError("original source VA absent "+hex(va))

def verify(path):
    data=Path(path).read_bytes()
    if hashlib.sha256(data).hexdigest()!=ELF_SHA:
        raise ValueError("not pinned original Ape Escape 3 USA SCUS_975.01")
    if data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0]!=8:
        raise ValueError("source original not MIPS32 little-endian ELF")
    if len(WORDS)!=29 or hashlib.sha256(read(data,START,END-START)).hexdigest()!=WINDOW_SHA:
        raise ValueError("original 557F54 post-step 29-word window changed")
    for va,w in WORDS.items():
        if struct.unpack("<I",read(data,va,4))[0]!=w:
            raise ValueError("retail original VU0 source opcode differs "+hex(va))
    branches={
        0x557F5C:0x558094, 0x557F88:0x557FC8,
        0x557F90:0x5580D4, 0x557FC0:0x557E88
    }
    for pc,target in branches.items():
        imm=WORDS[pc]&0xFFFF
        if imm>=32768:imm-=65536
        if pc+4+(imm<<2)!=target:
            raise ValueError("source actual branch target mismatch "+hex(pc))
    if WORDS[0x557F78]&63!=0x3E or WORDS[0x557F7C]&63!=0x3F:
        raise ValueError("source original VRSQRT/VWAITQ altered")
    if WORDS[0x557F80]&63!=0x1C:
        raise ValueError("original 557F80 VMULq altered")
    if struct.unpack("<I",read(data,0x704B08,4))[0]!=0x3F733333:
        raise ValueError("source original 0.95f epsilon differs")
    return {
        "original_elf_sha256":ELF_SHA,
        "source_window":"0x557F54..0x557FC8",
        "source_window_sha256":WINDOW_SHA,
        "source_instruction_count":len(WORDS),
        "retail_epsilon_float_bits":"0x3F733333",
        "source_vu0":"VF2 = VF1 * VRSQRT(VF0.w,VF2.x)",
        "source_early_status":"1 when original child+8 nonzero and VF1 dot normalized VF2 <0.95",
        "full_actor_gameplay_frame_complete":False,
        "native_windows_game_playable":False
    }
if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("original_elf",type=Path)
    print(json.dumps(verify(p.parse_args().original_elf),indent=2))
