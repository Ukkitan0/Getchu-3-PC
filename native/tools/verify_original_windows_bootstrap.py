#!/usr/bin/env python3
"""Verify actual USA SCUS_975.01 startup and 143 native constructor addresses.

Reads owner-private executable; emits audit metadata only, never game bytes.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

EXPECTED_SHA = "5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
CTOR_BODY_SHA = "01e600ce72db3b1290949e9f8f772559faa937eb8b034f4e6bae5688115d7b1b"
CTOR_TABLE_SHA = "a3c5739dcec7d153ee3e72a04b14046ce4e9366d68537258cab6bd168eac99ba"

class OriginalElf:
    def __init__(self, raw):
        if hashlib.sha256(raw).hexdigest() != EXPECTED_SHA:
            raise ValueError("Not the verified Ape Escape 3 USA SCUS_975.01")
        if raw[:7] != b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',raw,18)[0] != 8:
            raise ValueError("Expected original little-endian MIPS ELF32")
        if struct.unpack_from('<I',raw,24)[0] != 0x100134:
            raise ValueError("Unexpected original program entrypoint")
        self.raw=raw
        phoff=struct.unpack_from('<I',raw,28)[0]
        entsize,count=struct.unpack_from('<HH',raw,42)
        self.segments=[]
        if entsize<32 or phoff+entsize*count>len(raw):raise ValueError('Invalid ELF program headers')
        for i in range(count):
            typ,off,va,_,size,_,flags,_=struct.unpack_from('<8I',raw,phoff+i*entsize)
            if typ==1 and off+size<=len(raw):self.segments.append((va,va+size,off,flags))
        if not self.segments:raise ValueError('Missing original ELF PT_LOAD')
    def read(self,va,size):
        for a,b,off,_ in self.segments:
            if a<=va and va+size<=b:return self.raw[off+va-a:off+va-a+size]
        raise ValueError(f'Original ELF VA not backed by file data: {va:#x}')
    def word(self,va):return struct.unpack('<I',self.read(va,4))[0]

def verify(path):
    elf=OriginalElf(path.read_bytes())
    def sha(start,size):return hashlib.sha256(elf.read(start,size)).hexdigest()
    if sha(0x428ab8,0x58)!=CTOR_BODY_SHA:
        raise ValueError('Original 0x428AB8 constructor dispatch opcode mismatch')
    if sha(0x62e660,0x240)!=CTOR_TABLE_SHA:
        raise ValueError('Original 143-entry static constructor table mismatch')
    if elf.word(0x62e660)!=0xffffffff or elf.word(0x62e8a0)!=0:
        raise ValueError('Original constructor table sentinel/boundary mismatch')
    functions=[elf.word(0x62e89c-i*4) for i in range(143)]
    if functions[0]!=0x41a390 or functions[-1]!=0x10526c:
        raise ValueError('Original constructor table boundary addresses changed')
    if any(a<=b or (b & 3) for a,b in zip(functions,functions[1:])):
        raise ValueError('Original constructor dispatch order is invalid')
    edges={0x1002c4:0x1036d8,0x10031c:0x43fe58,0x100324:0x435d60,
           0x100354:0x428b80,0x10036c:0x1037e8,
           0x628218:0x1000a0,0x628220:0x428ab8}
    for site,target in edges.items():
        opcode=elf.word(site)
        observed=((site+4)&0xf0000000)|((opcode&0x3ffffff)<<2)
        if opcode>>26 != 3 or observed!=target:
            raise ValueError(f'Original startup JAL mismatch {site:#x}->{target:#x}')
    opcode=elf.word(0x100374)
    if opcode>>26 != 2 or (opcode & 0x3ffffff)<<2!=0x428cb8:
        raise ValueError('Original startup termination JMP mismatch')
    return {'schema':'ae3/original-windows-bootstrap-elf-audit/v1',
            'usa_elf_sha256':EXPECTED_SHA,'original_entry':'0x100134',
            'source_constructor_loop':'0x428ab8','ctor_table_first':'0x62e89c',
            'ctor_sentinel':'0x62e660','native_constructor_count':len(functions),
            'first_constructor':f'{functions[0]:#x}',
            'last_constructor':f'{functions[-1]:#x}',
            'verified_startup_calls':len(edges),
            'original_callbacks_not_implemented':True,
            'original_windows_gameplay_booted':False}

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('private_original_elf',type=Path)
    parser.add_argument('--out',type=Path)
    args=parser.parse_args()
    report=verify(args.private_original_elf)
    result=json.dumps(report,indent=2)+'\n'
    if args.out:args.out.write_text(result,encoding='utf8')
    else:print(result,end='')
    print('ORIGINAL_WINDOWS_STARTUP_ELF_VERIFIED',report['native_constructor_count'])
if __name__=='__main__':main()
