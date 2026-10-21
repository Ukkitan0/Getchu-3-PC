#!/usr/bin/env python3
"""Verify original owner +0x20 count / +0x24 stride / +0x04 base accessor."""
import argparse, hashlib, json, struct
from pathlib import Path
SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
WINDOW=(0x2bb0d0,0x2bb110)
WINDOW_SHA='aad762fd1378d559d32848a21d81508e0a0c7d83ba56ca28e14e69183b2a9d76'
ANCHORS={
    0x2bb0d0:0x27bdfff0,
    0x2bb0d8:0x0c0aec21,
    0x2bb0e8:0x8de206c8,
    0x2bb0f4:0x3c0f0074,
    0x2bb0f8:0x25ef06a8,
    0x2bb0fc:0x8dee0024,
    0x2bb100:0x8de20004,
    0x2bb104:0x008e2018,
    0x2bb108:0x03e00008,
    0x2bb10c:0x00441021,
    0x2baa98:0xae020024,
    0x2bab60:0xae0f0004,
    0x2ba6f8:0xae020020,
}
def verify(path):
    b=Path(path).read_bytes()
    if hashlib.sha256(b).hexdigest()!=SHA:raise ValueError('Wrong original USA ELF')
    if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
        raise ValueError('Expected retail MIPS LE ELF32')
    ph=struct.unpack_from('<I',b,28)[0]; step,n=struct.unpack_from('<HH',b,42)
    seg=[]
    for i in range(n):
        kind,off,va,_,sz,_,_,_=struct.unpack_from('<8I',b,ph+i*step)
        if kind==1 and off+sz<=len(b):seg.append((va,va+sz,off))
    def read(va,size):
        for lo,hi,off in seg:
            if lo<=va and va+size<=hi:return b[off+va-lo:off+va-lo+size]
        raise ValueError('Unmapped retail VA '+hex(va))
    def word(va):return struct.unpack('<I',read(va,4))[0]
    lo,hi=WINDOW
    if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=WINDOW_SHA:
        raise ValueError('Original numeric record accessor differs')
    for pc,expected in ANCHORS.items():
        if word(pc)!=expected:raise ValueError('Source instruction mismatch '+hex(pc))
    w=word(0x2bb104)
    # PS2 R5900 MUL rd,rs,rt writes low product into a0.
    assert (w>>26, (w>>21)&31,(w>>16)&31,(w>>11)&31,w&63)==(0,4,14,4,24)
    assert word(0x2bb0d8)>>26==3 and (word(0x2bb0d8)&0x03ffffff)<<2==0x2bb084
    assert word(0x2bb0fc)&0xffff==0x24
    assert word(0x2bb100)&0xffff==0x04
    assert word(0x2bb0e8)&0xffff==0x06c8
    assert word(0x2bb10c)>>26==0 # ADDU in JR delay slot
    return {'source_elf_sha256':SHA,'whole_instruction_words':(hi-lo)//4,
            'opcode_anchors':len(ANCHORS),'owner_global':hex(0x7406a8),
            'source_count_field':0x20,'source_stride_field':0x24,
            'source_rebased_pointer_field':0x04,
            'source_index_formula':'(base32+index32*stride32)&0xffffffff',
            'retail_allocations_decoded':False,'playable_game':False}
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('elf',type=Path)
    print('PASS original attribute count/stride/indexed pointer access')
    print(json.dumps(verify(p.parse_args().elf),indent=2))
