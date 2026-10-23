#!/usr/bin/env python3
"""Verify original USA 0x317B9C resource select and 0x557ED8 normal setup.

Pins BOTH complete MIPS32 function/entry windows against the original
private SHA-256-identified SCUS_975.01 executable, with each source
instruction pinned individually. 0x557ED8..0x557F30 is the recovered
normal setup PREFIX. 0x557F34/557F38/557F3C are verified only as
the boundary into unrecovered 0x317B34 gameplay continuation.

The user's original ELF remains private. No binary payload is committed.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ORIGINAL_ELF_SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
WINDOWS={
    (0x317B9C,0x317BD4):
        "f707fecf862b59b269a01af92dfa8983b113f96aecb8ca9d876bb5bb333cf296",
    (0x557ED8,0x557F40):
        "dc27cc5e5af609496f6e01ba0420d7aedb0a97ac3da5a3b409b24002733379d0",
}
SOURCE_WORDS={
    0x317B9C:0x8C8D0010, # parent +0x10 resource base
    0x317BA0:0x30A500FF, # ANDI child mode low byte
    0x317BA4:0x8C8E0040, # parent+0x40 ownership
    0x317BA8:0x240F0001,
    0x317BAC:0x11CF0007, # BEQ state40,1; to 317BCC
    0x317BB0:0x25A201C0, # delay v0=base+0x1C0
    0x317BB4:0x50A00005, # BEQZL mode to 317BCC
    0x317BB8:0x25A20220, # branch likely delay base+0x220
    0x317BBC:0x8C8E0034, # read parent+0x34 ONLY if mode !=0
    0x317BC0:0x25AF0120, # base+0x120
    0x317BC4:0x25A20140, # base+0x140
    0x317BC8:0x01EE100A, # MOVZ selects base+0x120 if word34=0
    0x317BCC:0x03E00008, # return v0
    0x317BD0:0x00000000, # delay nop
    0x557ED8:0x4480A000, # source f20=0
    0x557EDC:0x0220202D, # a0=original parent
    0x557EE0:0x92050009, # original child+9 mode byte BEFORE stores
    0x557EE4:0xE61400B4, # original child+B4=0
    0x557EE8:0x0C0C5EE7, # JAL original 317B9C
    0x557EEC:0xAE0000B8, # JAL delay child+B8=0
    0x557EF0:0x4600A306, # f12=source f20=0
    0x557EF4:0x0040282D, # a1=returned original resource VA
    0x557EF8:0x0280202D, # a0=real parent+14
    0x557EFC:0x0000302D, # a2=0
    0x557F00:0x0C09DADA, # JAL original 276B68
    0x557F04:0x0000382D, # delay a3=0
    0x557F08:0x10400004, # BEQZ source return skip 2767F4
    0x557F0C:0x0280202D, # delay a0=parent+14
    0x557F10:0x0C09D9FD, # JAL 2767F4 only when v0!=0
    0x557F14:0x0280202D, # JAL delay a0=parent+14
    0x557F18:0x0280202D, # fallthrough a0=parent+14
    0x557F1C:0x0000282D, # a1=0
    0x557F20:0x0C09DCF5, # JAL original 2773D4 always
    0x557F24:0x0000302D, # delay a2=0
    0x557F28:0x240F0001, # load original next-state flag
    0x557F2C:0xA20F00C0, # write original child+C0=1
    0x557F30:0x92050009, # reload mode byte for next source call
    0x557F34:0x0220202D, # a0=original parent
    0x557F38:0x0C0C5ECD, # JAL unrecovered original 317B34
    0x557F3C:0x26120010, # setup child+10 for future 36FB18
}
def fetch(data,va,n):
    phoff=struct.unpack_from("<I",data,28)[0]
    width,count=struct.unpack_from("<HH",data,42)
    for i in range(count):
        kind,off,base,_,file_count,_,_,_=struct.unpack_from(
            "<8I",data,phoff+i*width)
        if kind==1 and base<=va and va+n<=base+file_count:
            return data[off+va-base:off+va-base+n]
    raise ValueError("retail function VA not ELF-backed "+hex(va))
def verify(path):
    data=Path(path).read_bytes()
    if hashlib.sha256(data).hexdigest()!=ORIGINAL_ELF_SHA:
        raise ValueError("wrong retail USA SCUS_975.01 build")
    if data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0]!=8:
        raise ValueError("not MIPS little-endian ELF32")
    for (start,end),digest in WINDOWS.items():
        if hashlib.sha256(fetch(data,start,end-start)).hexdigest()!=digest:
            raise ValueError("original source function changed "+hex(start))
    for pc,opcode in SOURCE_WORDS.items():
        if struct.unpack("<I",fetch(data,pc,4))[0]!=opcode:
            raise ValueError("source instruction mismatch "+hex(pc))
    # Original J/JAL branch location crosschecks. Known instructions
    # are SHA-pinned above; check the actual original branch targets.
    targets={
        0x317BAC:0x317BCC,
        0x317BB4:0x317BCC,
        0x557F08:0x557F1C,
    }
    for pc,target in targets.items():
        opcode=SOURCE_WORDS[pc]
        offset=opcode&0xFFFF
        if offset>=0x8000:offset-=0x10000
        if pc+4+offset*4!=target:
            raise ValueError("source branch target mismatch "+hex(pc))
    for pc,target in {
        0x557EE8:0x317B9C,
        0x557F00:0x276B68,
        0x557F10:0x2767F4,
        0x557F20:0x2773D4,
        0x557F38:0x317B34,
    }.items():
        op=SOURCE_WORDS[pc]
        if op>>26!=3 or ((pc+4)&0xF0000000)|((op&0x03FFFFFF)<<2)!=target:
            raise ValueError("source original JAL target mismatch "+hex(pc))
    return {
        "retail_elf_sha256":ORIGINAL_ELF_SHA,
        "original_methods":["0x317B9C","0x557ED8"],
        "verified_windows":len(WINDOWS),
        "original_source_instruction_sites":len(SOURCE_WORDS),
        "source_original_selector_words":14,
        "source_normal_setup_words":23,
        "resource_selector_offsets":["+0x1C0","+0x220","+0x120","+0x140"],
        "child_normal_setup_writes":["+0xB4=0","+0xB8=0","+0xC0=1"],
        "source_calls_in_original_order":["0x317B9C","0x276B68","0x2767F4?","0x2773D4"],
        "normal_gameplay_continuation":"0x557F34 -> 0x317B34 (unrecovered)",
        "native_source_setup_translated":True,
        "full_original_actor_update_implemented":False,
        "playable_game":False}
if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("original_elf",type=Path)
    print(json.dumps(verify(p.parse_args().original_elf),indent=2))
