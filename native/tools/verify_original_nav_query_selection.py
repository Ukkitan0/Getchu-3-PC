#!/usr/bin/env python3
"""Verify retail USA NavGraph radius query, 256-slot owner, and path backtracking.

Requires exact privately owned original ELF and woods_c/navigation.pck.
No original game binary or resource payload is redistributed.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
NAV_SHA='d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0'
REGIONS={
 (0x2207d4,0x220924):'918b4add42ccd947f0cee5324d55ad9b93956bd23d2202b998b380f7aac3571e',
 (0x220994,0x220a4c):'417764b1fd072cd69df9fc4242f721521fc731498f766d53542cb45dcc392483',
 (0x220aa4,0x220c00):'a04c1c570197cea97e26e1c65278d2802c23b0e4c8a3cfe970207fb27a6dddc5',
 (0x21afb8,0x21b040):'7eb90f6245ffa39cd0e3b4e474b412f53011bb7d400860292b48aebe61158f78',
}
ANCHORS={
 0x22081c:0x24043800,0x220820:0x0c106e2c,0x220830:0x3c0e006e,
 0x220848:0xe5a00034,0x22084c:0xadaf0030,0x220850:0x158ffff8,
 0x220858:0xaea20000,0x220888:0x24070001,
 0x220890:0x0c0882a9,0x220898:0x0c088300,0x2208a0:0x0c088357,
 0x220998:0xac800004,0x2209b4:0x50af000c,0x2209c4:0x10a00007,
 0x2209cc:0x240f0038,0x2209e8:0x0c106e2c,0x2209ec:0x00042080,
 0x220a0c:0x000f7880,0x220a14:0x240f0038,
 0x220a24:0x8dee0004,0x220a28:0x8de50030,0x220a2c:0xac4e0000,
 0x220aa8:0x310800ff,0x220ad8:0x15000004,
 0x220ae4:0x0c086bee,0x220af0:0x0c086c05,
 0x220af8:0x5440002e,0x220b00:0x8e440008,
 0x220b0c:0x12800005,0x220b14:0x0c087e27,
 0x220b24:0x0c086d1a,0x220b58:0x46000004,
 0x220b5c:0x46140034,0x220b60:0x45000013,
 0x220b68:0x8e4e000c,0x220b88:0xae4e000c,
 0x220b8c:0x0c086c0e,0x220b94:0x0c086cfe,
 0x220ba8:0x1440000a,0x220bd4:0x0c086d01,
 0x220bf0:0x0c0882a9,
 0x21b014:0x8c8f0004,0x21b018:0x8c8e000c,
 0x21b02c:0xac800008,0x21b038:0x03e00008,
}
CALLS={
 0x220820:0x41b8b0,0x220890:0x220aa4,0x220898:0x220c00,
 0x2208a0:0x220d5c,0x2209e8:0x41b8b0,
 0x220ae4:0x21afb8,0x220af0:0x21b014,0x220b14:0x21f89c,
 0x220b24:0x21b468,0x220b8c:0x21b038,0x220b94:0x21b3f8,
 0x220bd4:0x21b404,0x220bf0:0x220aa4,
}
BRANCHES={
 0x220850:0x220834,0x2208a8:0x220934,0x2209b4:0x2209e8,
 0x2209c4:0x2209e4,0x2209fc:0x220a38,0x220a30:0x220a14,
 0x220ad8:0x220aec,0x220af8:0x220bb4,
 0x220b04:0x220b24,0x220b0c:0x220b24,
 0x220b1c:0x220bb4,0x220ba8:0x220bd4,
}

def audit(executable,original_package):
    blob=Path(executable).read_bytes()
    pck=Path(original_package).read_bytes()
    if hashlib.sha256(blob).hexdigest()!=ELF_SHA or hashlib.sha256(pck).hexdigest()!=NAV_SHA:
        raise ValueError('Wrong privately owned original USA ELF or navigation.pck')
    if blob[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',blob,18)[0]!=8:
        raise ValueError('Expected little-endian original retail ELF32 for MIPS')
    ph=struct.unpack_from('<I',blob,28)[0]
    stride,count=struct.unpack_from('<HH',blob,42)
    seg=[]
    for i in range(count):
        typ,off,va,_,size,_,_,_=struct.unpack_from('<8I',blob,ph+i*stride)
        if typ==1 and off+size<=len(blob):seg.append((va,va+size,off))
    def read(va,length):
        for lo,hi,off in seg:
            if lo<=va and va+length<=hi:return blob[off+va-lo:off+va-lo+length]
        raise ValueError('Original source VA unmapped: '+hex(va))
    def word(pc):return struct.unpack('<I',read(pc,4))[0]
    for (lo,hi),expected in REGIONS.items():
        if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=expected:
            raise ValueError('Original source-region digest failed at '+hex(lo))
    for pc,expected in ANCHORS.items():
        if word(pc)!=expected:raise ValueError(f'Original opcode mismatch at {pc:#x}: {word(pc):08x} != {expected:08x}')
    for pc,target in CALLS.items():
        w=word(pc)
        if w>>26!=3 or (((pc+4)&0xf0000000)|((w&0x3ffffff)<<2))!=target:
            raise ValueError('Original caller edge differs at '+hex(pc))
    for pc,target in BRANCHES.items():
        d=word(pc)&65535
        if d>=32768:d-=65536
        if pc+4+4*d!=target:raise ValueError('Original branch destination differs at '+hex(pc))
    # COP1 BC1F 0x220B60 goes to no-insertion epilogue at 0x220BB0.
    assert word(0x220b60)>>26==0x11 and ((word(0x220b60)>>16)&31)==0x00
    assert 0x220b60+4+4*(word(0x220b60)&0xffff)==0x220bb0
    assert (word(0x22081c)&65535)==0x3800
    assert (word(0x2209cc)&65535)==0x38
    assert (word(0x220a24)&65535,word(0x220a28)&65535)==(4,0x30)
    assert (word(0x220b68)&65535,word(0x220b88)&65535)==(12,12)
    assert word(0x220b58)==0x46000004 and word(0x220b5c)==0x46140034
    assert (word(0x220aa8)&65535)==255
    assert pck[:4]==b'PCK\0' and struct.unpack_from('<II',pck,4)==(64,2)
    counts=[];edges=[]
    for i in range(2):
        name,typ,off,size=struct.unpack_from('<4I',pck,64+i*16)
        assert pck[typ:pck.index(0,typ)]==b'nav'
        src=pck[off:off+size]
        n=struct.unpack_from('<I',src,0x24)[0]
        h=struct.unpack_from('<I',src,0x3c)[0]
        degrees=[struct.unpack_from('<H',src,h+j*30+2)[0] for j in range(n)]
        if any(x>3 for x in degrees):raise ValueError('Invalid source three-neighbor limit')
        counts.append(n);edges.append(sum(degrees))
    assert counts==[39,20] and edges==[95,42]
    return {'source_regions':len(REGIONS),
            'original_instruction_words':sum((hi-lo)//4 for lo,hi in REGIONS),
            'exact_opcode_anchors':len(ANCHORS),
            'direct_call_edges':len(CALLS),'branch_targets':len(BRANCHES),
            'source_query_workspace_bytes':0x3800,
            'source_search_record_bytes':0x38,
            'max_source_query_nodes':256,
            'centroid_gate':'distance_xyz < radius',
            'source_radius_graphs':counts,
            'source_directed_neighbors':edges,
            'stage_actor_runtime_recovered':False,'playable_native_windows_game':False}
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('original_elf',type=Path)
    p.add_argument('retail_navigation_pck',type=Path)
    args=p.parse_args()
    result=audit(args.original_elf,args.retail_navigation_pck)
    print('PASS: retail original 0x220AA4 radius selection and 0x220994 predecessor extraction')
    print(json.dumps(result,indent=2))
