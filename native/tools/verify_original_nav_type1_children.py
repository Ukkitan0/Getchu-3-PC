#!/usr/bin/env python3
"""Verify original USA AE3 0x21B0B0 / 0x21B240 type-1 navigation source lists.

Requires owner-private ELF and woods_c/navigation.pck; emits no original bytes.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path
ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
NAV_SHA='d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0'
REGIONS={
 (0x21b0b0,0x21b120):'3ebcf450644c362e2d1522a61e535f6782e6e218d412f2719f58741ded8bca2e',
 (0x21b14c,0x21b178):'71ddd00c85d4b74aeacd98b5161fcf1c7319b261a253d957effd6abb82053391',
 (0x21b178,0x21b194):'f451cc60bb1ce68ab4813b06891ca5558e9f334dbdea41ff3b2d0ad21d1f353e',
 (0x21b240,0x21b254):'1b027bbaa78c5ad4e633e4f39a90d8388964a30cc14e369e3972b8f1c4153f23',
 (0x21d70c,0x21d7c0):'d382e73c36c0cf01f36afc56da64ccd4564ed9d16285de652865e7002984b2e5',
 (0x21b384,0x21b3f8):'a4d99963163930421f7910b85e88be51dd6c9a5dd0727a0e296742e37e9367c1',
 (0x21d7d8,0x21d884):'07154907e37686d7e98c418420e28f483869d8f4e411eecaed1be6bdb059bbcf'
}
ANCHORS={
0x21b0b4:0xac850000,0x21b0bc:0x00057900,0x21b0d0:0x0c106e2c,
0x21b0d4:0x01e0202d,0x21b0d8:0x1200000b,0x21b108:0xae220000,
0x21b178:0x03e00008,0x21b180:0x8c8f0004,
0x21b184:0x00052900,0x21b188:0x00af2821,0x21b190:0x78a20000,
0x21b240:0x8c8f0004,0x21b244:0x00052900,0x21b248:0x00af2821,
0x21b250:0x7ca60000,
0x21d710:0x24040008,0x21d714:0x96cf0000,0x21d71c:0x8dce001c,
0x21d720:0x000f7840,0x21d724:0x0c106e00,0x21d728:0x01cf9821,
0x21d72c:0x96650000,0x21d734:0x0c086c2c,0x21d738:0x0040202d,
0x21d760:0x00157840,0x21d768:0x026f7821,0x21d770:0x95ef0002,
0x21d77c:0x0280202d,0x21d780:0x000f7900,0x21d788:0x01cf7021,
0x21d7a0:0x0c086c90,0x21d7b0:0x55e0ffec,
0x21b334:0x8e300000,0x21b344:0x0c086c53,0x21b34c:0x0c1068ec}
CALLS={0x21b0d0:0x41b8b0,0x21d724:0x41b800,0x21d734:0x21b0b0,
       0x21d7a0:0x21b240,0x21b344:0x21b14c,0x21b34c:0x41a3b0}
BRANCHES={0x21b0d8:0x21b108,0x21d758:0x21d67c,0x21d7b0:0x21d764,
          0x21d7b8:0x21d680}

def verify(executable,archive):
    exe=Path(executable).read_bytes();pack=Path(archive).read_bytes()
    if hashlib.sha256(exe).hexdigest()!=ELF_SHA or hashlib.sha256(pack).hexdigest()!=NAV_SHA:
        raise ValueError('Expected the original owner-private USA SCUS_975.01 and woods_c/navigation.pck')
    if exe[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',exe,18)[0]!=8:
        raise ValueError('Expected original LE MIPS ELF32')
    ph=struct.unpack_from('<I',exe,28)[0];entsz,n=struct.unpack_from('<HH',exe,42)
    mappings=[]
    for j in range(n):
        typ,off,va,_,filesz,_,_,_=struct.unpack_from('<8I',exe,ph+j*entsz)
        if typ==1 and off+filesz<=len(exe):mappings.append((va,va+filesz,off))
    def read(va,size):
        for a,b,off in mappings:
            if a<=va and va+size<=b:return exe[off+va-a:off+va-a+size]
        raise ValueError('Unmapped executable VA '+hex(va))
    def word(va):return struct.unpack('<I',read(va,4))[0]
    for (begin,end),expected in REGIONS.items():
        if hashlib.sha256(read(begin,end-begin)).hexdigest()!=expected:
            raise ValueError('Original source instruction region differs '+hex(begin))
    for pc,original in ANCHORS.items():
        if word(pc)!=original:
            raise ValueError(f'Original opcode differs at {pc:#x}: {word(pc):08x} expected {original:08x}')
    for pc,target in CALLS.items():
        w=word(pc)
        if w>>26!=3 or ((((pc+4)&0xf0000000)|((w&0x3ffffff)<<2))!=target):
            raise ValueError('Original function edge differs '+hex(pc))
    for pc,target in BRANCHES.items():
        delta=word(pc)&65535
        if delta>=32768:delta-=65536
        if pc+4+4*delta!=target:raise ValueError('Original branch differs '+hex(pc))
    assert word(0x21b0bc)>>26==0 and (word(0x21b0bc)>>6)&31==4
    assert word(0x21b250)>>26==0x1f # retail SQ copy of 16B
    assert word(0x21b0bc)&0x3f==0 # count shift by 4
    assert (word(0x21d720)>>6)&31==1 # selector indexes 16-bit elements
    assert word(0x21d778)&65535==0x10 # original source vertex table pointer
    assert word(0x21d770)&65535==2 # next list entry
    assert word(0x21d780)>>26==0 # index shift by 4
    assert pack[:4]==b'PCK\0' and struct.unpack_from('<II',pack,4)==(64,2)
    actions={0:0,1:0,2:0}
    for i in range(2):
        _,_,offset,size=struct.unpack_from('<4I',pack,64+i*16)
        src=pack[offset:offset+size]
        count=struct.unpack_from('<I',src,0x24)[0]
        desc=struct.unpack_from('<I',src,0x3c)[0]
        for k in range(count):
            at=desc+k*0x1e
            n=struct.unpack_from('<H',src,at+2)[0]
            assert n<=3
            for j in range(n):
                typ=struct.unpack_from('<H',src,at+10+2*j)[0]
                actions[typ]=actions.get(typ,0)+1
    if actions[1]!=0 or actions[2]!=15:
        raise ValueError('Original woods_c action type tally unexpected')
    return {'original_instruction_windows':len(REGIONS),
            'instruction_words':sum((b-a)//4 for a,b in REGIONS),
            'opcode_anchors':len(ANCHORS),'direct_call_edges':len(CALLS),
            'branch_edges':len(BRANCHES),'type1_actions_in_woods_c':actions[1],
            'type2_actions_in_woods_c':actions[2],
            'original_type1_allocation_size_per_vector_bytes':16,
            'original_list_source_offset_field':'stage+0x4c',
            'original_vector_source_offset_field':'stage+0x40',
            'original_child_runtime_allocated':False,'game_playable':False}
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('retail_elf',type=Path);p.add_argument('private_navigation_pck',type=Path)
    a=p.parse_args()
    print('PASS original type-1 navigation child source operations')
    print(json.dumps(verify(a.retail_elf,a.private_navigation_pck),indent=2))
