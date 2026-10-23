#!/usr/bin/env python3
"""Pin original 0x558030..0x558084 retail child post-movement block.

Real private USA ELF SCUS_975.01 is required and never committed.
Verifies ALL 21 MIPS instructions, source branch-likely delay slots,
genuine 314C44 JAL and source constant +1.0 at 0x704B0C.
The upstream original 0x30C0A0/0x276538 and 557F54 VF/collision
paths are NOT translated by this narrowly complete control block.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ELF_SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
START,END=0x558030,0x558084
WINDOW_SHA="e4d59bf362eda8e908c4ffc4167674760b73b59f6d78710be2e9efddc9cfc3e4"
SOURCE_HEX="""
C60100B4 25AD4B0C C5A00000 46010036 45010002
240F0001 0000782D 31EF00FF 51E00007 8E24001C
00000000 00000000 4614A803 240E0002 1000FF86
E6200070 02A0282D 0C0C5311 92060009 1000FF81
0000702D
"""
WORDS={START+i*4:int(w,16) for i,w in enumerate(SOURCE_HEX.split())}
def read(data,va,length):
    off=struct.unpack_from("<I",data,28)[0]
    stride,count=struct.unpack_from("<HH",data,42)
    for index in range(count):
        typ,fileoff,base,_,size,_,_,_=struct.unpack_from(
            "<8I",data,off+index*stride)
        if typ==1 and base<=va and va+length<=base+size:
            return data[fileoff+va-base:fileoff+va-base+length]
    raise ValueError("original retail source address absent "+hex(va))
def verify(path):
    data=Path(path).read_bytes()
    if hashlib.sha256(data).hexdigest()!=ELF_SHA:
        raise ValueError("not pinned USA SCUS_975.01")
    if data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0]!=8:
        raise ValueError("original executable must be little-endian MIPS32 ELF")
    raw=read(data,START,END-START)
    if len(WORDS)!=21 or hashlib.sha256(raw).hexdigest()!=WINDOW_SHA:
        raise ValueError("original 21-word 558030 window hash changed")
    for va,opcode in WORDS.items():
        actual=struct.unpack("<I",read(data,va,4))[0]
        if opcode!=actual:
            raise ValueError("original MIPS source word changed "+hex(va))
    for va,target in {
        0x558040:0x55804C, 0x558050:0x558070,
        0x558068:0x557E84, 0x55807C:0x557E84,
    }.items():
        w=WORDS[va]
        disp=w&65535
        if disp&32768:disp-=65536
        if va+4+disp*4!=target:
            raise ValueError("source original branch target mismatch "+hex(va))
    j=WORDS[0x558074]
    if j>>26!=3 or ((0x558078&0xF0000000)|((j&0x03FFFFFF)<<2))!=0x314C44:
        raise ValueError("real original 314C44 JAL target differs")
    if struct.unpack("<I",read(data,0x704B0C,4))[0]!=0x3F800000:
        raise ValueError("original source progress threshold changed")
    return {
        "retail_elf_sha256":ELF_SHA,
        "source_window":"0x558030..0x558084",
        "source_window_sha256":WINDOW_SHA,
        "original_instruction_count":len(WORDS),
        "original_child_status_float_offset":"+0xB4",
        "terminal_original_status":2,
        "terminal_parent_float_write":"+0x70 = f21 / f20",
        "nonterminal_original_status":0,
        "nonterminal_source_call":"0x314C44(parent+0x1C,s5,child+0x09)",
        "retail_callers_required":["0x30C0A0","0x276538"],
        "real_full_actor_frame_recovered":False,
        "playable_game":False
    }
if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("original_elf",type=Path)
    print(json.dumps(verify(p.parse_args().original_elf),indent=2))
