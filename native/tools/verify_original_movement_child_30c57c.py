#!/usr/bin/env python3
"""Verify complete original USA SCUS_975.01 0x30C57C movement-child constructor.

Uses user-provided private original ELF, never committed to GitHub.
Checks exact 61-word source function, source 0x57C15C call arguments,
allocation size, vtable, 0xB0-byte source scratch transfer, mode
low-byte stores and only genuinely initialized original object fields.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ORIGINAL_ELF_SHA = "5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
CONSTRUCTOR_WINDOW_SHA = "9c9bd89de5dc50b3780c081f91aa5e826d89383b0bea95fb39b78bf35943d4a5"
CALLSITE_WINDOW_SHA = "6aacae3b2047d5a00fbbdefca10af0a64580551e9f700f2500dcb931bc6a8b13"
OPCODES = {
    0x30C57C:0x27BDFFD0, # original 0x30 stack frame
    0x30C594:0x30D000FF, # ANDI source a2,0xFF
    0x30C59C:0x30F100FF, # ANDI source a3,0xFF
    0x30C5A4:0x0C106E00, # JAL game heap alloc 0x41B800
    0x30C5A8:0x240400D0, # allocation argument 0xD0
    0x30C5AC:0x3C0F006B, # original vtable base
    0x30C5B0:0xAC520004, # SW original parent,+0x04
    0x30C5B4:0x25EF0B20, # original vtable 0x006B0B20
    0x30C5B8:0xA0500008, # SB a2 lowbyte,+0x08
    0x30C5BC:0xAC4F0000, # SW vtable,+0x00
    0x30C5C4:0xA0510009, # SB a3 lowbyte,+0x09
    0x30C5C8:0x266D0080, # source scratch end +0x80
    0x30C5CC:0x244F0010, # destination byte offset +0x10
    0x30C5D0:0xDE690000, # LD 0x00 source, repeated 0x40 loop
    0x30C5EC:0xFDEC0018, # SD target+0x18
    0x30C610:0x26730040, # scratch pointer +=0x40
    0x30C614:0x166DFFEE, # loop until scratch +0x80
    0x30C618:0x25EF0040, # destination pointer +=0x40 delay slot
    0x30C61C:0xDE6D0000, # read remaining six doublewords
    0x30C63C:0xDE6D0020,
    0x30C650:0xFDE90028, # SD last 8 bytes source +0xA8
    0x30C658:0xA1C000C0, # SB zero,+0xC0
    0x30C668:0x03E00008, # JR ra
    0x57C150:0x9247001D, # a3 = original controller mode+0x1d
    0x57C154:0x0200282D, # a1 = original scratch s0
    0x57C158:0x02A0202D, # a0 = true original parent
    0x57C15C:0x0C0C315F, # JAL original 0x30C57C
    0x57C160:0x24060001, # original a2=1 in JAL delay slot
}

def source_bytes(data,va,size):
    phoff=struct.unpack_from("<I",data,28)[0]
    width,n=struct.unpack_from("<HH",data,42)
    for i in range(n):
        typ,off,start,_,file_size,_,_,_=struct.unpack_from(
            "<8I",data,phoff+width*i)
        if typ==1 and start<=va and va+size<=start+file_size:
            return data[off+va-start:off+va-start+size]
    raise ValueError("original executable source VA not file-backed "+hex(va))

def verify(path):
    original=path.read_bytes()
    if hashlib.sha256(original).hexdigest()!=ORIGINAL_ELF_SHA:
        raise ValueError("not pinned original Ape Escape 3 USA executable")
    if original[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",original,18)[0]!=8:
        raise ValueError("not original little-endian 32-bit MIPS ELF")
    source=source_bytes(original,0x30C57C,0x30C670-0x30C57C)
    if hashlib.sha256(source).hexdigest()!=CONSTRUCTOR_WINDOW_SHA:
        raise ValueError("original 0x30C57C constructor body differs")
    caller=source_bytes(original,0x57C150,0x57C164-0x57C150)
    if hashlib.sha256(caller).hexdigest()!=CALLSITE_WINDOW_SHA:
        raise ValueError("original movement-child callsite differs")
    for at,want in OPCODES.items():
        (word,)=struct.unpack("<I",source_bytes(original,at,4))
        if word!=want:
            raise ValueError("original child constructor opcode changed "+hex(at))
    return {
        "original_executable_sha256":ORIGINAL_ELF_SHA,
        "original_constructor":"0x30C57C..0x30C670",
        "instruction_words":len(source)//4,
        "full_constructor_sha256":CONSTRUCTOR_WINDOW_SHA,
        "caller":"0x57C15C",
        "guest_allocated_object_bytes":0xD0,
        "guest_vtable_va":"0x006B0B20",
        "original_source_scratch_copy_bytes":0xB0,
        "source_caller_arg2":1,
        "original_written_offsets":"+0x00..+0x09, +0x10..+0xC0",
        "original_uninitialized_offsets":"+0x0A..+0x0F, +0xC1..+0xCF",
        "source_constructor_native_translation":True,
        "retail_vtable_methods_and_game_runtime_required":True,
        "playable_windows_game":False,
    }

if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("original_elf",type=Path)
    print(json.dumps(verify(p.parse_args().original_elf),indent=2))
