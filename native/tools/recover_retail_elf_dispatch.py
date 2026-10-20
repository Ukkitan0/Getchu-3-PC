#!/usr/bin/env python3
"""Static analysis of authentic AE3 USA ELF Lua registrations and entity dispatch.

No hardware emulation, translated guest instructions, extracted original code
bytes, or fabricated engine behavior. Output consists of verified metadata.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct

PIN = "5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"

def signed16(x):
    x &= 0xffff
    return x - 0x10000 if x & 0x8000 else x

def jal(pc, word):
    return ((pc+4)&0xf0000000)|((word&0x3ffffff)<<2) if word>>26==3 else None

class RetailElf:
    def __init__(self, path):
        self.data=path.read_bytes()
        if hashlib.sha256(self.data).hexdigest()!=PIN:
            raise ValueError("Expected exact original Ape Escape 3 USA SCUS_975.01 ELF")
        if self.data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",self.data,18)[0]!=8:
            raise ValueError("Expected little-endian MIPS ELF")
        phoff=struct.unpack_from("<I",self.data,28)[0]
        stride,count=struct.unpack_from("<HH",self.data,42)
        self.loads=[]
        for i in range(count):
            kind,off,va,_,size,_,flags,_=struct.unpack_from("<IIIIIIII",self.data,phoff+i*stride)
            if kind==1 and off+size<=len(self.data):
                self.loads.append((va,va+size,off,flags))
        if not self.loads:raise ValueError("Missing original ELF load segment")
    def offset(self,va):
        for start,end,off,_ in self.loads:
            if start<=va<=end-4:return off+va-start
        raise ValueError("Unmapped original executable address: 0x%x"%va)
    def word(self,va):return struct.unpack_from("<I",self.data,self.offset(va))[0]
    def string(self,va):
        try:
            off=self.offset(va)
            end=self.data.index(0,off,off+128)
            b=self.data[off:end]
            if b and all(32<=x<127 for x in b):return b.decode("ascii")
        except (ValueError,UnicodeDecodeError):pass
        return None
    def a1_ptr(self,pc):
        low=self.word(pc+4)
        if low>>26!=9 or (low>>21)&31!=5 or (low>>16)&31!=5:return None
        for n in range(1,12):
            high=self.word(pc-4*n)
            if high>>26==15 and (high>>16)&31==5:
                return ((high&0xffff)<<16)+signed16(low)
        return None

def analyze(elf):
    registrations=[]
    for lo,hi,_,flags in elf.loads:
        if not(flags&1):continue
        for pc in range(lo,hi-60,4):
            if jal(pc,elf.word(pc))!=0x3e5f90:continue
            nameva=elf.a1_ptr(pc)
            name=elf.string(nameva) if nameva is not None else None
            if not name:continue
            for step in range(3,14):
                bind=pc+4*step
                if jal(bind,elf.word(bind))!=0x3e60b8:continue
                target=elf.a1_ptr(bind)
                if target is not None and 0x100000<=target<0x600000:
                    registrations.append({"name":name,"register_site":hex(pc),
                                          "name_va":hex(nameva),
                                          "native_callback":hex(target),"bind_site":hex(bind)})
                break
    def comparisons(start,end):
        result=[]
        for pc in range(start,end,4):
            if jal(pc,elf.word(pc))!=0x42c44c:continue
            ptr=elf.a1_ptr(pc)
            name=elf.string(ptr) if ptr is not None else None
            if name:result.append({"name":name,"compare_jal":hex(pc),"name_va":hex(ptr)})
        return result
    result={"format":"getchu3/original-elf-lua-direct-dispatch/v1",
            "elf_sha256":PIN,
            "lua_registration_pairs":registrations,
            "new_ent_type_comparisons":comparisons(0x21498c,0x216f00),
            "c_ctrl_opcode_comparisons":comparisons(0x219810,0x219ba0),
            "warning":"These are verified name/target or string-comparison sites. Constructor semantics and runtime validity still require reverse engineering. No original game bytes."}
    core={"new_ent":0x21498c,"c_ctrl":0x219810,
          "c_order":0x217e38,"lua_param":0x219e4c}
    mapped={item["name"]:int(item["native_callback"],16) for item in registrations}
    if any(mapped.get(name)!=addr for name,addr in core.items()):
        raise ValueError("Original game core Lua functions did not match expected addresses")
    return result

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("original_elf",type=Path)
    ap.add_argument("--out",type=Path)
    a=ap.parse_args()
    data=analyze(RetailElf(a.original_elf))
    if a.out:
        a.out.parent.mkdir(parents=True,exist_ok=True)
        a.out.write_text(json.dumps(data,indent=2)+"\n",encoding="utf-8")
    else:print(json.dumps(data,indent=2))
    for key in ("lua_registration_pairs","new_ent_type_comparisons","c_ctrl_opcode_comparisons"):
        print(key+": "+str(len(data[key])))

if __name__=="__main__":main()
