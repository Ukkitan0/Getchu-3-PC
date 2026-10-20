#!/usr/bin/env python3
"""Recover REAL Lua API registration names from SCUS_975.01 executable.

Looks for the actual native-register helper call at 0x255BD8 preceded
by MIPS LUI $a1 and delay-slot ADDIU $a1,$a1,offset. Reads original
string literals from ELF PT_LOAD, no PS2 CPU execution or emulation.
Owned ELF is supplied locally and NEVER checked into source control.
"""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import struct

US_SHA256="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
TEXT_START=0x100000
TEXT_BYTES=0x528258
REGISTRY_HELPER=0x255bd8

def inspect(blob:bytes):
    sha=hashlib.sha256(blob).hexdigest()
    if sha!=US_SHA256:
        raise ValueError("Unknown game ELF: only verified USA SCUS_975.01 accepted")
    if blob[:4]!=b"\x7fELF" or blob[4:6]!=b"\x01\x01":
        raise ValueError("Not original MIPS ELF32 little endian")
    phoff=struct.unpack_from("<I",blob,28)[0]
    entrysize,count=struct.unpack_from("<HH",blob,42)
    if entrysize<32 or phoff+entrysize*count>len(blob):
        raise ValueError("Invalid ELF program headers")
    segments=[]
    for i in range(count):
        p=struct.unpack_from("<8I",blob,phoff+i*entrysize)
        if p[0]==1:segments.append(p)
    def read_va(va:int,n:int):
        for _,off,addr,_,filesize,_,_,_ in segments:
            if addr<=va and va+n<=addr+filesize:
                return blob[off+va-addr:off+va-addr+n]
        return None
    code=read_va(TEXT_START,TEXT_BYTES)
    if code is None:
        raise ValueError("Verified original ELF missing .text virtual span")
    items=[]
    for i in range(0,len(code)-8,4):
        word=struct.unpack_from("<I",code,i)[0]
        if word>>26!=3:continue  # MIPS JAL
        pc=TEXT_START+i
        target=((pc+4)&0xf0000000)|((word&0x3ffffff)<<2)
        if target!=REGISTRY_HELPER:continue
        delay=struct.unpack_from("<I",code,i+4)[0]
        # immediate low half of the string address in the delay slot
        if delay>>26!=9 or ((delay>>21)&31)!=5 or ((delay>>16)&31)!=5:
            continue
        immediate=delay&65535
        if immediate>=0x8000:immediate-=65536
        hi=None
        for offset in range(i-4,max(0,i-64),-4):
            ins=struct.unpack_from("<I",code,offset)[0]
            if ins>>26==15 and ((ins>>16)&31)==5:
                hi=ins&65535
                break
        if hi is None:continue
        string_address=(hi<<16)+immediate
        source=read_va(string_address,128)
        if source is None:continue
        label=source.split(b"\0",1)[0]
        if not label or len(label)>100 or not all(32<=c<127 for c in label):
            continue
        items.append({"name":label.decode("ascii"),
                      "register_jal":f"0x{pc:08X}",
                      "game_string_address":f"0x{string_address:08X}",
                      "game_register_helper":f"0x{REGISTRY_HELPER:08X}"})
    return {"original_verified_elf_sha256":sha,
            "source":"Original SCUS_975.01 code and string literals, not a guess",
            "api_names_found":len(items),"registrations":items,
            "limits":"Only captures this specific registration helper/path. Other game native Lua callbacks require further reverse engineering."}

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("original_elf",type=Path)
    p.add_argument("--out",type=Path,
                   help="Save owner-private JSON analysis of retail registration sites")
    args=p.parse_args()
    result=inspect(args.original_elf.read_bytes())
    print("ORIGINAL_GAME_LUA_REGISTRATIONS",len(result["registrations"]))
    for r in result["registrations"]:
        print(r["register_jal"],r["name"],r["game_string_address"])
    if args.out:
        args.out.parent.mkdir(parents=True,exist_ok=True)
        args.out.write_text(json.dumps(result,indent=2)+"\n")
    return 0
if __name__=="__main__":
    raise SystemExit(main())
