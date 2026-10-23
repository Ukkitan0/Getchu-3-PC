#!/usr/bin/env python3
"""Verify recovered original AE3 USA 0x316174/0x3161B8 controller updates.

SHA-pin BOTH complete original MIPS function windows, the exact
0x3161B8 -> 0x316174 source JAL, original +0x10/+0x14
child/status ownership, virtual slots, 0x696C98 nesting counter and
low-byte a2/a3 flag behavior. Original ELF stays owner-private.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

PIN="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
WINDOWS={
    (0x316174,0x3161B8):"4f5bf06a0806755a5ab50762e1af2357af0a401f63e1a8b4e7d8bea432584103",
    (0x3161B8,0x31622C):"bc602d09c1633a68857d2ca1c7b1bb2133a3c8e75bd125b876230bc7f0e25cab",
}
OPCODES={
    0x316178:0x8C8E0000,  # load genuine vtable
    0x316188:0x8E0F6C98,  # read original nesting depth
    0x31618C:0x8DC20014,  # virtual slot5 at +0x14
    0x316190:0x25EF0001,  # nesting ++
    0x316194:0x0040F809,  # JALR genuine slot5
    0x316198:0xAE0F6C98,  # store nesting in delay slot
    0x31619C:0x8E0F6C98,  # RELOAD after source vtable update
    0x3161A4:0x25EFFFFF,  # nesting --
    0x3161A8:0xAE0F6C98,  # store nesting --
    0x3161BC:0x30C600FF,  # original a2 low8 only
    0x3161C4:0x34AF0001,  # first original a1 flag
    0x3161CC:0x01E6280B,  # MOVN a1 if a2 lowbyte
    0x3161D4:0x34AF0002,  # second original a1 flag
    0x3161DC:0x8C840000,  # controller child pointer
    0x3161E0:0x0C0C585D,  # JAL 0x316174
    0x3161E4:0x01E7280B,  # delayed MOVN for a3 lowbyte
    0x3161E8:0xAE020004,  # child owner status +0x14
    0x3161EC:0x1040000A,  # update result zero gate
    0x316200:0x8DEF0000,  # child virtual table pointer
    0x316208:0x01E0F809,  # child vtable slot1 destroying JALR
    0x316210:0xAE000000,  # clear original child+0x10
    0x316214:0x240F0001,  # original completed return ONE
    0x31621C:0x01E0102D,  # original return v0
}
def read(data,va,count):
    phoff=struct.unpack_from("<I",data,28)[0]
    stride,n=struct.unpack_from("<HH",data,42)
    for i in range(n):
        typ,offset,base,_,sz,_,_,_=struct.unpack_from(
            "<8I",data,phoff+i*stride)
        if typ==1 and base<=va and va+count<=base+sz:
            return data[offset+va-base:offset+va-base+count]
    raise ValueError("original ELF VA absent "+hex(va))
def verify(path):
    data=path.read_bytes()
    if hashlib.sha256(data).hexdigest()!=PIN:
        raise ValueError("not the pinned original USA SCUS_975.01")
    if data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0]!=8:
        raise ValueError("not MIPS little-endian ELF32")
    for (start,end),digest in WINDOWS.items():
        if hashlib.sha256(read(data,start,end-start)).hexdigest()!=digest:
            raise ValueError("original 0x316174/0x3161B8 function changed "+hex(start))
    for pc,expected in OPCODES.items():
        if struct.unpack("<I",read(data,pc,4))[0]!=expected:
            raise ValueError("original child update instruction changed "+hex(pc))
    return {"original_elf_sha256":PIN,
        "original_functions":[hex(x[0]) for x in WINDOWS],
        "complete_source_instruction_words":sum((y-x)//4 for x,y in WINDOWS),
        "independent_instruction_sites":len(OPCODES),
        "virtual_slot5":5,"deleting_virtual_slot":1,
        "original_global_depth":"0x696C98",
        "source_child_field":"+0x10","source_status_field":"+0x14",
        "original_jal":"0x3161E0 -> 0x316174",
        "owner_virtual_method_recovery_required":True,
        "native_game_playable":False}
if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("original_elf",type=Path)
    print(json.dumps(verify(parser.parse_args().original_elf),indent=2))
