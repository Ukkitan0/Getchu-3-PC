#!/usr/bin/env python3
"""Verify original Ape Escape 3 USA 0x57BF5C source movement owner.

This verifies one COMPLETE source function window and EVERY direct J/JAL
edge. It intentionally does not pretend the original movement implementation
has been translated or that a native/Unreal game is playable.
The original executable is owner-private and is never stored in Git.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

PIN = "5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
START = 0x57BF5C
END = 0x57C554
SOURCE_SHA256 = "f1190b818486c965de496c36b5e946d36e0ed1f89f7721d207b90b80b0c7f0c7"
# These are the genuine source JAL sites, not an imagined game call graph.
CALLS = {
    0x57C034:0x316174, 0x57C074:0x21B854,
    0x57C128:0x441F40, 0x57C130:0x441F40,
    0x57C148:0x36F5F4, 0x57C15C:0x30C57C,
    0x57C190:0x3161B8, 0x57C264:0x21DB64,
    0x57C2A8:0x3176AC, 0x57C2C4:0x30C1D4,
    0x57C2FC:0x3161B8, 0x57C338:0x316174,
    0x57C3D0:0x57C7CC, 0x57C3EC:0x30C21C,
    0x57C424:0x3161B8, 0x57C45C:0x316174,
    0x57C4B8:0x30C520, 0x57C4EC:0x3161B8,
}
# The exact retail window contains 18 JAL sites. Enumerate every site;
# no source edge is filled by guessed native controller behavior.
def load_segment(data, address, count):
    phoff = struct.unpack_from("<I", data, 28)[0]
    size, number = struct.unpack_from("<HH", data, 42)
    for index in range(number):
        typ, fileoff, va, _, filesz, _, _, _ = struct.unpack_from(
            "<8I", data, phoff + index * size)
        if typ == 1 and va <= address and address + count <= va + filesz:
            return data[fileoff + address - va:fileoff + address - va + count]
    raise ValueError("unmapped original source VA " + hex(address))

def verify(path):
    data = path.read_bytes()
    if hashlib.sha256(data).hexdigest() != PIN:
        raise ValueError("not the original hash-pinned USA SCUS_975.01")
    if data[:7] != b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0] != 8:
        raise ValueError("not original MIPS32 little-endian ELF")
    window = load_segment(data,START,END-START)
    if hashlib.sha256(window).hexdigest() != SOURCE_SHA256:
        raise ValueError("source 0x57BF5C original function byte window differs")
    actual = {}
    for off in range(0,len(window),4):
        op = struct.unpack_from("<I",window,off)[0]
        if op >> 26 in (2,3):
            pc = START+off
            actual[pc] = ((pc+4)&0xf0000000)|((op&0x3ffffff)<<2)
            if op >> 26 != 3:
                raise ValueError("unexpected original direct J (not JAL) at "+hex(pc))
    if actual != CALLS:
        missing = {hex(k):hex(v) for k,v in actual.items() if CALLS.get(k)!=v}
        raise ValueError("movement source edge mismatch: "+repr(missing))
    return {
        "original_function":"0x57BF5C",
        "source_window_end_exclusive":"0x57C554",
        "elf_sha256":PIN,
        "source_window_sha256":SOURCE_SHA256,
        "source_instruction_words":len(window)//4,
        "direct_jal_sites":len(CALLS),
        "route_update_call_at_57c3d0":"0x57C7CC",
        "movement_owner_native_recovery_complete":False,
        "playable_windows_game":False,
    }

if __name__ == "__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("original_elf",type=Path)
    args=parser.parse_args()
    print(json.dumps(verify(args.original_elf),indent=2))
