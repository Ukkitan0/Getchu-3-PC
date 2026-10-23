#!/usr/bin/env python3
"""Verify source USA 0x317B34 two native non-VU float return branches.

Pins the complete 26-instruction routine and its calling JAL at
0x557F38 against the exact private retail SCUS_975.01 ELF.
The nonzero-mode VU0 SQRT branch is *not* implemented by the
two fast paths and must never yield a fabricated resource float.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path
PIN="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
FUNCTION_SHA="66e99b9cf7fadebdbb0a4befc31cdfce5aeefa3beba5c08787351c2ea9b09b4c"
WORDS={
    0x317B34:0x8C8D0008, # original parent+0x08 resource pointer
    0x317B38:0x8C8E0040, # original parent+0x40 type
    0x317B3C:0x240F0001,
    0x317B40:0x15CF0003, # BNE !=1 into mode branch
    0x317B44:0x30A500FF, # original branch delay: a1&0xff
    0x317B48:0x03E00008, # JR ra source type1
    0x317B4C:0xC5A00374, # JR delay: LWC1 original table+0x374
    0x317B50:0x10A00010, # BEQZ mode -> source 317B94
    0x317B54:0x00000000,
    0x317B58:0xD8810060, # original tagged-pointer alternate branch
    0x317B5C:0xC5A20364, # VU0 FP source table+0x364
    0x317B60:0x4BE1086A, # VMUL source data
    0x317B64:0xC5A10368,
    0x317B68:0x4BA10841,
    0x317B6C:0x4BE10842,
    0x317B70:0x482F0800,
    0x317B74:0x000F7800,
    0x317B78:0x448F0000,
    0x317B7C:0x00000000,
    0x317B80:0x00000000,
    0x317B84:0x46000004, # SQRT.S source normal metric
    0x317B88:0x46020000, # ADD.S source offset
    0x317B8C:0x03E00008,
    0x317B90:0x46010029, # source VU0 delay-slot
    0x317B94:0x03E00008, # JR ra source mode==0
    0x317B98:0xC5A003B4, # JR delay LWC1 original table+0x3B4
    0x557F38:0x0C0C5ECD, # real original 0x317B34 callsite
}
def get(data,va,n):
    phoff=struct.unpack_from("<I",data,28)[0]
    width,num=struct.unpack_from("<HH",data,42)
    for i in range(num):
        kind,off,base,_,count,_,_,_=struct.unpack_from(
            "<8I",data,phoff+i*width)
        if kind==1 and base<=va and va+n<=base+count:
            return data[off+va-base:off+va-base+n]
    raise ValueError("source VA not mapped in original ELF "+hex(va))
def verify(path):
    data=Path(path).read_bytes()
    if hashlib.sha256(data).hexdigest()!=PIN:
        raise ValueError("not SHA-256-pinned original USA SCUS_975.01")
    if data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0]!=8:
        raise ValueError("source is not original MIPS little-endian ELF32")
    if hashlib.sha256(get(data,0x317B34,0x68)).hexdigest()!=FUNCTION_SHA:
        raise ValueError("original 317B34 function body differs")
    for addr,expected in WORDS.items():
        if struct.unpack("<I",get(data,addr,4))[0]!=expected:
            raise ValueError("original source opcode differs "+hex(addr))
    for branch,target in {0x317B40:0x317B50,0x317B50:0x317B94}.items():
        word=WORDS[branch]
        signed=word&0xffff
        if signed>=0x8000:signed-=65536
        if branch+4+(signed<<2)!=target:
            raise ValueError("original resource float branch changed "+hex(branch))
    return {
        "original_elf_sha256":PIN,
        "source_method":"0x317B34",
        "source_words":26,
        "source_complete_window_sha256":FUNCTION_SHA,
        "individually_pinned_sites":len(WORDS),
        "native_complete_branch_one":"parent+0x40==1 -> f0=(parent+0x08)+0x374",
        "native_complete_branch_two":"mode_low8==0 -> f0=(parent+0x08)+0x3B4",
        "remaining_native_gap":"mode_low8!=0 and parent+0x40!=1: original VU0 SQRT",
        "original_game_playable":False,
    }
if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("original_elf",type=Path)
    print(json.dumps(verify(p.parse_args().original_elf),indent=2))
