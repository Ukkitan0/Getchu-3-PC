#!/usr/bin/env python3
"""Audit native 0x57C98C against original Ape Escape 3 USA executable.

Reads user-owned ELF only, never emits or republishes original game bytes.
"""
import argparse, hashlib, json, struct
from pathlib import Path
ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
ORIGINAL={
 0x57c98c:0x0c086d16,0x57c99c:0x0c086d16,0x57c9b0:0x0c086d16,
 0x57c9bc:0x0c0dbd35,0x57c9e0:0x0c0dbd52,
 0x57c9d4:0xc5f40000,0x57c9dc:0x4600a501,
 0x57c9f8:0xc5e10000,0x57ca0c:0xc5e20000,
 0x57ca28:0x4be11858,0x57ca40:0x4be20898,
 0x57ca54:0x4be20898,0x57ca5c:0x4be118e8,
 0x57ca60:0x1000ffb9,0x57ca64:0x4be21868,
 0x708a64:0x3f800000,0x708a68:0x3f4ccccd,0x708a6c:0x3d888888
}
CALLS={0x57c98c:0x21b458,0x57c99c:0x21b458,0x57c9b0:0x21b458,
       0x57c9bc:0x36f4d4,0x57c9e0:0x36f548}
VU={0x57ca28:(0x18,1,3,1),0x57ca40:(0x18,2,1,2),
    0x57ca54:(0x18,2,1,2),0x57ca5c:(0x28,3,3,1),
    0x57ca64:(0x28,1,3,2)}
def verify(path):
    b=Path(path).read_bytes()
    if hashlib.sha256(b).hexdigest()!=ELF_SHA:
        raise ValueError('Wrong original retail USA SCUS_975.01 SHA256')
    if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
        raise ValueError('Expected original ELF32 little-endian MIPS')
    ph=struct.unpack_from('<I',b,28)[0]
    step,n=struct.unpack_from('<HH',b,42)
    segments=[]
    for i in range(n):
        kind,off,va,_,length,_,_,_=struct.unpack_from('<8I',b,ph+i*step)
        if kind==1 and off+length<=len(b):segments.append((va,va+length,off))
    def word(a):
        for lo,hi,off in segments:
            if lo<=a and a+4<=hi:return struct.unpack_from('<I',b,off+a-lo)[0]
        raise ValueError(f'Original retail VA is unmapped: {a:#x}')
    for a,expected in ORIGINAL.items():
        if word(a)!=expected:raise ValueError(f'Original instruction mismatch: {a:#x}')
    for a,target in CALLS.items():
        opcode=word(a)
        if opcode>>26!=3 or (((a+4)&0xf0000000)|((opcode&0x3ffffff)<<2))!=target:
            raise ValueError(f'Original JAL mismatch: {a:#x}')
    for a,(op,fd,fs,ft) in VU.items():
        opcode=word(a)
        fields=(opcode&63,(opcode>>6)&31,(opcode>>11)&31,(opcode>>16)&31)
        if fields!=(op,fd,fs,ft):
            raise ValueError(f'Original VU0 encoded opcode mismatch: {a:#x}')
    return {'retail_ELF_SHA256':ELF_SHA,'original_source_instructions':55,
            'source_vertex_accessor_calls':3,'source_game_RNG_calls':2,
            'source_VU0_mul_add_operations':5,'constant_1_bits':'0x3F800000',
            'constant_0_8_bits':'0x3F4CCCCD','constant_offset_bits':'0x3D888888',
            'recovered_original_branch':'0x57C98C..0x57CA64',
            'native_windows_game_booted':False}
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('original_elf',type=Path)
    args=parser.parse_args()
    print(json.dumps(verify(args.original_elf),indent=2))
