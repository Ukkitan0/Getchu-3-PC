#!/usr/bin/env python3
"""Verify four original Ape Escape 3 USA controller functions.

The 382-word 0x57BF5C..0x57C554 region contains FOUR separate entrypoints,
not one 382-instruction function: 0x57BF5C, 0x57C27C, 0x57C388 and 0x57C48C.
This audits all four windows and every original JAL edge independently.
Original executable bytes are owner-private and never committed.
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
# Original prologue and JR/restore boundaries independently verified.
WINDOWS = (
    (0x57BF5C,0x57C27C,"c625b3ad27bba0b2658f037bc4c5f53dd11096896fe28408eb5fe922e43ae4d4"),
    (0x57C27C,0x57C388,"c276349407171b78e83ba5af50aa92c980ced0f638cdf333d670fd3805790b4f"),
    (0x57C388,0x57C48C,"c37ced8f115b217f5c79114344cbec86e3745c755bacebc3ff63c87b5614a4fb"),
    (0x57C48C,0x57C554,"889e3c33b087048da2eaca86df93c03bb62b4dfbe76baafab3be537540bf609a"),
)
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
# Across FOUR independent original function bodies: 8+4+4+2 JAL sites.
# Never model the entries as one 382-instruction actor method.
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
    # Source 0x57C27C publishes the packed original method pair
    # 0x708A48 -> {0x57C388, 0}. This is the NEXT genuine controller
    # function, not an external replacement steering method.
    original_next_method=load_segment(data,0x708A48,8)
    if original_next_method!=struct.pack("<II",0x57C388,0):
        raise ValueError("source 0x708A48 controller update pair changed")
    # Distinct source function prologues; the previous 382-word
    # region audit mistakenly combined these as one method.
    for pc,opcode in {
        0x57BF5C:0x27BDFED0,
        0x57C27C:0x27BDFFC0,
        0x57C388:0x27BDFFA0,
        0x57C48C:0x27BDFFE0,
    }.items():
        if struct.unpack("<I",load_segment(data,pc,4))[0]!=opcode:
            raise ValueError("original controller entry changed "+hex(pc))
    # Both source 0x57C48C and 0x57C554 share the exact MMI
    # PCEQW -> PPACB -> NOR -> SLTIU 128-bit equality, followed
    # by branch-delay return polarity. Distinguish these from any
    # prior native test expectation that inverted 1 and child+0x14.
    for pc,op in {
        0x57C500:0x11E00009,  # BEQZ variant-zero -> word14
        0x57C510:0x71EE78A8,  # original PCEQW
        0x57C514:0x700F7DC8,  # original PPACB
        0x57C528:0x240D0001,  # variant-zero selects source LW
        0x57C538:0x8E020014,  # word14 return
        0x57C5CC:0x11E00009,
        0x57C5DC:0x71EE78A8,
        0x57C5E0:0x700F7DC8,
        0x57C5F4:0x240D0001,
        0x57C604:0x8E020014,
    }.items():
        if struct.unpack("<I",load_segment(data,pc,4))[0]!=op:
            raise ValueError("original packed-vector return branch changed "+hex(pc))
    total_window = load_segment(data,START,END-START)
    if hashlib.sha256(total_window).hexdigest() != SOURCE_SHA256:
        raise ValueError("combined original four-function region differs")
    actual = {}
    per_function = []
    for lo,hi,pinned in WINDOWS:
        window=load_segment(data,lo,hi-lo)
        if hashlib.sha256(window).hexdigest()!=pinned:
            raise ValueError("original separate controller function differs "+hex(lo))
        call_count=0
        for off in range(0,len(window),4):
            op=struct.unpack_from("<I",window,off)[0]
            if op>>26 in (2,3):
                pc=lo+off
                actual[pc]=((pc+4)&0xf0000000)|((op&0x3ffffff)<<2)
                if op>>26!=3:
                    raise ValueError("unexpected source J at "+hex(pc))
                call_count+=1
        per_function.append({"entry":hex(lo),"end_exclusive":hex(hi),
            "instruction_words":len(window)//4,"jal_sites":call_count,
            "window_sha256":pinned})
    if actual != CALLS:
        missing = {hex(k):hex(v) for k,v in actual.items() if CALLS.get(k)!=v}
        raise ValueError("movement source edge mismatch: "+repr(missing))
    return {
        "original_functions":per_function,
        "combined_source_window":"0x57BF5C..0x57C554",
        "elf_sha256":PIN,
        "combined_sha256":SOURCE_SHA256,
        "source_instruction_words":len(total_window)//4,
        "direct_jal_sites":len(CALLS),
        "route_update_call_at_57c3d0":"0x57C7CC (belongs to 0x57C388)",
        "complete_57c27c_translated":True,
        "movement_owner_native_recovery_complete":False,
        "playable_windows_game":False,
    }

if __name__ == "__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("original_elf",type=Path)
    args=parser.parse_args()
    print(json.dumps(verify(args.original_elf),indent=2))
