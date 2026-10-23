#!/usr/bin/env python3
"""Verify real USA normal child terminal update: 0x36F954 -> 0x557E30.

Source ELF is private and never committed. Verifies the complete
28-instruction original 0x36F954 first-return region, original
0x557EB4 caller/branch-delay instructions, threshold and argument
constants. TRUE terminal means child slot5 returns original v0=2;
the other source paths are NOT translated or assumed to be zero.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ELF_PIN="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
WINDOWS={
    (0x36F954,0x36F9C4):
    "674fae1d67260580966da93648ee600a3c16b706c713eeff15374a519a5bbc66",
    (0x557EB4,0x557ED8):
    "679c219dcbf330b0c75dc53ab409783d96787535e07e5168c0b527d50ccad993",
}
SOURCE_OPS={
    0x36F958:0x44801000, # COP1 f2 = 0.0
    0x36F96C:0xC48100A4, # original a0+0xA4 (child+0xB4)
    0x36F970:0x46020836, # C.OLE.S status <= 0.0
    0x36F974:0x45000004, # BC1F bypass first gate
    0x36F978:0x46006506, # original source f12 retained as f20
    0x36F97C:0x46026034, # C.OLT.S f12 < 0.0
    0x36F980:0x4501000B, # BC1T original status=1 exit
    0x36F984:0x24020001, # delay slot return status 1
    0x36F98C:0x25CE57B4, # original threshold VA 0x7157B4
    0x36F990:0xC5C00000, # LWC1 source threshold 1.0
    0x36F994:0x46010036, # C.OLE.S 1.0 <= status
    0x36F998:0x4502000B, # BC1FL into other game cases
    0x36F99C:0x8E0F00A8, # branch-likely load source +0xA8
    0x36F9A0:0x46141034, # C.OLT.S zero < source f12
    0x36F9A4:0x45020008, # BC1FL into other game cases
    0x36F9A8:0x8E0F00A8, # delay load only on other path
    0x36F9AC:0x24020001, # known terminal source return =1
    0x36F9BC:0x03E00008, # return
    0x557EB4:0x55E0001F, # child +0xC0 nonzero => other path
    0x557EB8:0x92050009, # branch-likely delay: child mode +9
    0x557EC0:0x24840010, # a0=child+0x10
    0x557EC4:0x25AD4B04, # original source f12 constant 0x704B04
    0x557EC8:0x0C0DBE55, # actual JAL 0x36F954
    0x557ECC:0xC5AC0000, # JAL delay LWC1 f12=2.0
    0x557ED0:0x1440FFEC, # return status2 when source returns nonzero
    0x557ED4:0x240E0002, # delay slot status=2
}

def source_bytes(data,va,length):
    phoff=struct.unpack_from("<I",data,28)[0]
    width,n=struct.unpack_from("<HH",data,42)
    for i in range(n):
        kind,offset,base,_,filesz,_,_,_=struct.unpack_from(
            "<8I",data,phoff+i*width)
        if kind==1 and base<=va and va+length<=base+filesz:
            return data[offset+va-base:offset+va-base+length]
    raise ValueError("original source VA absent "+hex(va))

def verify(path):
    data=Path(path).read_bytes()
    if hashlib.sha256(data).hexdigest()!=ELF_PIN:
        raise ValueError("not original Ape Escape 3 USA SCUS_975.01")
    if data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0]!=8:
        raise ValueError("source not little-endian ELF32 MIPS")
    for (start,end),digest in WINDOWS.items():
        if hashlib.sha256(source_bytes(data,start,end-start)).hexdigest()!=digest:
            raise ValueError("original function window differs "+hex(start))
    for pc,op in SOURCE_OPS.items():
        actual=struct.unpack("<I",source_bytes(data,pc,4))[0]
        if actual!=op:
            raise ValueError("original opcode mismatch at "+hex(pc))
    if struct.unpack("<I",source_bytes(data,0x704B04,4))[0]!=0x40000000:
        raise ValueError("original callsite f12 is not +2.0")
    if struct.unpack("<I",source_bytes(data,0x7157B4,4))[0]!=0x3F800000:
        raise ValueError("original helper threshold is not +1.0")
    return {
        "original_elf_sha256":ELF_PIN,
        "source_windows":len(WINDOWS),
        "original_instruction_sites":len(SOURCE_OPS),
        "caller":"0x557EC8",
        "callee":"0x36F954",
        "callee_source_field":"child+0xB4",
        "source_terminal_threshold":1.0,
        "source_original_caller_f12":2.0,
        "source_child_update_result":2,
        "source_actor_gameplay_below_threshold_unrecovered":True,
        "native_game_playable":False,
    }
if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("original_elf",type=Path)
    print(json.dumps(verify(p.parse_args().original_elf),indent=2))
