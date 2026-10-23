#!/usr/bin/env python3
"""Retail source audit for 0x57C554 type-2 actor child update.
The user-owned Ape Escape 3 USA ELF is never committed or redistributed.
"""
from pathlib import Path
import argparse,hashlib,json,struct

PIN="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
WINDOW=(0x57C554,0x57C620)
SOURCE_SHA="4ca5bcd1a364bee1fb22edfde1b29073296b0531f3ddaab1396577cfadfa4e7a"
ANCHORS={
    0x57C554:0x27BDFFE0,0x57C570:0x8E0F0010,
    0x57C574:0x15E0000D,0x57C580:0x9207001D,
    0x57C588:0x8E060050,0x57C598:0x8C8F0000,
    0x57C59C:0x8DEF0004,0x57C5A0:0x01E0F809,
    0x57C5A8:0xAE110010,0x57C5AC:0x26040010,
    0x57C5C0:0x50400011,0x57C5C8:0x920F0030,
    0x57C5CC:0x11E00009,0x57C5D4:0x7A0F0040,
    0x57C5D8:0x7A0E0020,0x57C5DC:0x71EE78A8,
    0x57C5E0:0x700F7DC8,0x57C5E8:0x2DEF0001,
    0x57C5EC:0x11E00003,0x57C5F4:0x240D0001,
    0x57C5FC:0x11E00002,0x57C600:0x24020001,
    0x57C604:0x8E020014
}
JALS={0x57C584:0x30C360,0x57C5B8:0x3161B8}

def verify(path):
    data=Path(path).read_bytes()
    if hashlib.sha256(data).hexdigest()!=PIN:
        raise ValueError("Expected exact owner-original USA SCUS_975.01")
    if data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0]!=8:
        raise ValueError("Expected little-endian MIPS ELF32")
    ph=struct.unpack_from("<I",data,28)[0]
    stride,count=struct.unpack_from("<HH",data,42)
    segments=[]
    for i in range(count):
        typ,offset,va,_,length,_,_,_=struct.unpack_from("<8I",data,ph+i*stride)
        if typ==1 and offset+length<=len(data):
            segments.append((va,va+length,offset))
    def read(va,length):
        for first,last,offset in segments:
            if first<=va and va+length<=last:
                return data[offset+va-first:offset+va-first+length]
        raise ValueError("Original source VA not mapped "+hex(va))
    def word(va):return struct.unpack("<I",read(va,4))[0]
    first,last=WINDOW
    if hashlib.sha256(read(first,last-first)).hexdigest()!=SOURCE_SHA:
        raise ValueError("Complete original 0x57C554 function digest differs")
    for va,opcode in ANCHORS.items():
        if word(va)!=opcode:
            raise ValueError("Original 0x57C554 opcode differs "+hex(va))
    for va,target in JALS.items():
        ins=word(va)
        actual=((va+4)&0xf0000000)|((ins&0x03ffffff)<<2)
        if ins>>26!=3 or actual!=target:
            raise ValueError("Source original JAL target differs "+hex(va))
    return {
        "source":"Ape Escape 3 USA SCUS_975.01",
        "original_elf_sha256":PIN,
        "function":"0x57C554",
        "complete_function_sha256":SOURCE_SHA,
        "instructions":(last-first)//4,
        "opcode_anchors":len(ANCHORS),
        "original_call_targets":[hex(x) for x in JALS.values()],
        "original_owner_update_runtime_verified":False}
if __name__=="__main__":
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("original_elf",type=Path)
    args=ap.parse_args()
    print(json.dumps(verify(args.original_elf),indent=2))
    print("PASS original 0x57C554 type2 controller source")
