#!/usr/bin/env python3
"""Pin original SCUS_975.01 movement child vtable slot5/slot1.

Audits the real vtable at 0x6B0B20, the complete 33-word flag/
early-return section at 0x557E30, the 8-word original slot1
deleting destructor at 0x556A0C, and the 9-word allocator-release
routine at 0x41A3B0. This does NOT translate the source gameplay
continuations at 0x557EB4 and later.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ORIGINAL_SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
WINDOWS=(
    (0x557E30,0x557EB4,"c19b233ea83358e4af3e6e4e1c17f17e32b27641ae21055c38a051d60785e5e5"),
    (0x556A0C,0x556A2C,"d4c56f66712db7e4411af1dff39c27cd44f4de90eb229dfca65d2258633d9114"),
    (0x41A3B0,0x41A3D4,"3cffb07b4b1336059345d14c95001d5bd89ffedb85dd42edfc81b3cd1a6a037d"),
)
OPCODES={
    0x557E34:0x30AF0001, # ANDI a1 bit0
    0x557E60:0x8C910004, # LW real parent VA at child+4
    0x557E64:0x15E00006, # BNEZ bit0 -> early return1
    0x557E68:0x8E340014, # DELAY SLOT LW real parent+0x14
    0x557E6C:0x00057843, # SRA a1 one
    0x557E70:0x31EF0001, # ANDI a1 bit1
    0x557E74:0x31EF00FF, # original source low-byte canonicalization
    0x557E78:0x51E0000E, # BEQZL bit1->remaining gameplay path
    0x557E7C:0x908F00C0, # BRANCH-LIKELY DELAY SLOT LBU child+0xC0
    0x557E80:0x240E0001, # early result 1
    0x557E88:0x01C0102D, # original return status v0=1
    0x557EAC:0x03E00008, # JR ra
    0x556A0C:0x27BDFFF0, # deleting destructor
    0x556A10:0x3C0F006B, # base vtable 0x6B0C40
    0x556A18:0x25EF0C40,
    0x556A1C:0xAC8F0000, # source vtable pointer replaced
    0x556A24:0x081068EC, # tail J 0x41A3B0 allocator free
    0x41A3B8:0x10800004, # null-object free bypass
    0x41A3C0:0x0C040574, # real allocator release 0x1015D0
}
def read(data,va,count):
    offset=struct.unpack_from("<I",data,28)[0]
    size,n=struct.unpack_from("<HH",data,42)
    for i in range(n):
        typ,fileoff,base,_,filesz,_,_,_=struct.unpack_from(
            "<8I",data,offset+i*size)
        if typ==1 and base<=va and va+count<=base+filesz:
            return data[fileoff+va-base:fileoff+va-base+count]
    raise ValueError("original source ELF address unavailable "+hex(va))
def verify(path):
    data=Path(path).read_bytes()
    if hashlib.sha256(data).hexdigest()!=ORIGINAL_SHA:
        raise ValueError("wrong original USA SCUS_975.01 executable")
    if data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0]!=8:
        raise ValueError("expected little-endian MIPS ELF32")
    for lo,hi,pin in WINDOWS:
        if hashlib.sha256(read(data,lo,hi-lo)).hexdigest()!=pin:
            raise ValueError("original movement-child method changed "+hex(lo))
    methods=struct.unpack("<6I",read(data,0x6B0B20,0x18))
    if methods!=(0x5569FC,0x556A0C,0x557E18,0x557E28,0x557E20,0x557E30):
        raise ValueError("original child vtable 0x6B0B20 methods changed")
    for va,pin in OPCODES.items():
        if struct.unpack("<I",read(data,va,4))[0]!=pin:
            raise ValueError("original movement child opcode changed "+hex(va))
    return {
        "original_executable_sha256":ORIGINAL_SHA,
        "original_movement_child_vtable":"0x6B0B20",
        "slot1_deleting_destructor":"0x556A0C",
        "slot5_original_update":"0x557E30",
        "source_early_update_status":1,
        "source_early_arg_mask":"a1 & 3",
        "required_delay_slot_read":"parent+0x14 at 0x557E68",
        "conditional_child_flag_read":"child+0xC0 at 0x557E7C",
        "native_source_gameplay_tail_recovered":False,
        "original_runtime_playable":False,
        "verified_source_windows":len(WINDOWS),
        "pinned_instruction_sites":len(OPCODES)}
if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("original_elf",type=Path)
    print(json.dumps(verify(p.parse_args().original_elf),indent=2))
