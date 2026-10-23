#!/usr/bin/env python3
"""Verify native 0x57C620 controller dispatch against owner-private USA ELF.

No original copyrighted executable bytes are written to the repository.
The retail binary is supplied as an argument and SHA-pinned. This checks
the COMPLETE 107-instruction function window, its original vtable and
the exact source operands/branch instructions used by native translation.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ORIGINAL_ELF_SHA256 = "5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
WINDOW_START = 0x57C620
WINDOW_END = 0x57C7CC
WINDOW_SHA256 = "d15f6ebe4f862da7c50c7dfc08087cf12ab4a90cb38fc3231eb876987a61cc54"
ORIGINAL_SOURCE_WORDS = {
    0x57C620:0x27BDFFC0, 0x57C648:0x8DEE8A50,
    0x57C64C:0x8C8D0008, 0x57C650:0x15AE0008,
    0x57C680:0x8E0F0010, 0x57C68C:0x30AF0001,
    0x57C698:0x00057843, 0x57C6AC:0x8E050050,
    0x57C6C8:0xAE000050, 0x57C6D8:0x91EF54E7,
    0x57C6E8:0x91EF54E0, 0x57C758:0x0260102D,
    0x57C77C:0x31CF0001, 0x57C78C:0x000E7843,
    0x57C798:0x8DEF0000, 0x57C79C:0x000E7082,
    0x57C7A0:0x000E7080, 0x57C7A4:0x01CF7021,
    0x57C7A8:0x8DCE0000, 0x57C7B0:0x00042043,
    0x57C7B4:0x01C0F809, 0x57C7C8:0x8E0E0008
}
DIRECT_SOURCE_CALLS = {
    0x57C6C0:0x30FFD0,
    0x57C730:0x2478EC,
    0x57C750:0x247D84
}

class OriginalUSAELF:
    def __init__(self, path):
        data = path.read_bytes()
        if hashlib.sha256(data).hexdigest() != ORIGINAL_ELF_SHA256:
            raise ValueError("Expected exact owner-original USA SCUS_975.01 SHA-256")
        if data[:7] != b"\x7fELF\x01\x01\x01":
            raise ValueError("Expected MIPS little-endian ELF32")
        if struct.unpack_from("<H",data,18)[0] != 8:
            raise ValueError("Original target must be MIPS")
        phoff=struct.unpack_from("<I",data,28)[0]
        stride,count=struct.unpack_from("<HH",data,42)
        self.data=data
        self.segments=[]
        for n in range(count):
            typ,off,va,_,sz,_,_,_=struct.unpack_from("<8I",data,phoff+n*stride)
            if typ==1 and off+sz<=len(data):
                self.segments.append((va,va+sz,off))
        if not self.segments:
            raise ValueError("No original loadable ELF segments")
    def read(self,va,count):
        for start,end,off in self.segments:
            if start<=va and va+count<=end:
                return self.data[off+va-start:off+va-start+count]
        raise ValueError("Unmapped original VA "+hex(va))
    def word(self,va):
        return struct.unpack("<I",self.read(va,4))[0]

def verify(original_executable):
    elf=OriginalUSAELF(original_executable)
    if hashlib.sha256(elf.read(WINDOW_START,WINDOW_END-WINDOW_START)).hexdigest()!=WINDOW_SHA256:
        raise ValueError("Original controller 0x57C620 function window differs")
    if elf.word(0x6B1834)!=WINDOW_START:
        raise ValueError("Original controller vtable sixth entry not 0x57C620")
    if elf.read(0x708A50,8)!=b"\0"*8:
        raise ValueError("Original source method sentinel has changed")
    if elf.word(0x708A58)!=0x3f800000:
        raise ValueError("Original warning source scalar is not 1.0f")
    for va,opcode in ORIGINAL_SOURCE_WORDS.items():
        if elf.word(va)!=opcode:
            raise ValueError("Original opcode differs at "+hex(va))
    for va,expected in DIRECT_SOURCE_CALLS.items():
        opcode=elf.word(va)
        dest=((va+4)&0xf0000000)|((opcode&0x03ffffff)<<2)
        if opcode>>26!=3 or dest!=expected:
            raise ValueError("Original JAL target differs at "+hex(va))
    return {
        "original_executable_sha256":ORIGINAL_ELF_SHA256,
        "original_function":"0x57C620",
        "original_vtable_entry":"0x6B1834",
        "complete_function_sha256":WINDOW_SHA256,
        "verified_instruction_words":(WINDOW_END-WINDOW_START)//4,
        "verified_individual_opcode_sites":len(ORIGINAL_SOURCE_WORDS),
        "verified_original_direct_call_targets":len(DIRECT_SOURCE_CALLS),
        "original_sentinel_708a50_708a54":"0000000000000000",
        "warning_diagnostic_source_callback_required":True,
        "runtime_playable":False
    }
if __name__=="__main__":
    cli=argparse.ArgumentParser(description=__doc__)
    cli.add_argument("original_executable",type=Path)
    args=cli.parse_args()
    result=verify(args.original_executable)
    print(json.dumps(result,indent=2))
    print("PASS original 0x57C620 source instructions")
