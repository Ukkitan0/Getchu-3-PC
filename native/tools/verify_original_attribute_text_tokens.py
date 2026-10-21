#!/usr/bin/env python3
"""Authentic USA SCUS_975.01 text -> LF records -> ':' tokens code audit.

All source code/data in the owner-private ELF; no executable bytes emitted.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ELF_SHA = '5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
REGIONS = {
 (0x1f28c4,0x1f2a58):'15c75e3abd6233b95ad07d322f917dc5794946561ab4fbe88183c16221f671a5',
 (0x2ba3ac,0x2ba584):'547d795b26d79e730c3fff92ac0a8da326183ee40a76a89ac5bf361097d86a8f',
 (0x2ba584,0x2ba8d0):'9c6d2f1f86971441ff69c8cf1f8d6680489912ab86a20ce2be7943238bb71d00',
 (0x2bb03c,0x2bb084):'e4e87983feecf0691f5cc5ba6a953b72aaf0939f71719c03b94c1bd05d370594',
}
ANCHORS = {
 0x1f28c8:0xac800000,0x1f28f8:0xac800004,0x1f2908:0xac800008,
 0x1f2950:0x566f001e,0x1f295c:0x8deffff4,
 0x1f29cc:0x1514001a,0x1f29d4:0x8e040004,0x1f29dc:0x108f0010,
 0x1f29fc:0xae0f0004,0x1f2a18:0x1000ffc4,
 0x2ba418:0x8e2f0008,0x2ba424:0x92480000,0x2ba428:0x240f000a,
 0x2ba42c:0x15e8004b,0x2ba4b8:0x15e0ffda,
 0x2ba5f4:0x2406003a,0x2ba660:0x8fae0004,0x2ba6bc:0x240f0003,
 0x2ba700:0x11e000bd,0x2ba7fc:0x8deffffc,
 0x2ba788:0x27a40030,0x2ba8b8:0x8e8f0030,
 0x2bb048:0xade406a8,
}
CALLS = {
 0x1f296c:0x4c5a28,0x1f2a10:0x626d78,0x1f2a48:0x627a78,
 0x2ba4c0:0x2ba3a4,0x2ba5fc:0x1f28c4,0x2ba78c:0x1f28c4,
 0x2bb058:0x2ba218,0x2bb060:0x2ba3ac,0x2bb068:0x2ba584,
}
def verify(path):
    data=Path(path).read_bytes()
    if hashlib.sha256(data).hexdigest()!=ELF_SHA:
        raise ValueError('Incorrect original private USA retail executable')
    if data[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',data,18)[0]!=8:
        raise ValueError('Expected little endian MIPS ELF32')
    phoff=struct.unpack_from('<I',data,28)[0]
    phsize,phcount=struct.unpack_from('<HH',data,42)
    segments=[]
    for i in range(phcount):
        kind,off,va,_,size,_,_,_=struct.unpack_from('<8I',data,phoff+i*phsize)
        if kind==1 and off+size<=len(data):segments.append((va,va+size,off))
    def read(va,count):
        for low,high,off in segments:
            if low<=va and va+count<=high:return data[off+va-low:off+va-low+count]
        raise ValueError('Original address not mapped: '+hex(va))
    def word(addr):return struct.unpack('<I',read(addr,4))[0]
    for (lo,hi),digest in REGIONS.items():
        if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=digest:
            raise ValueError('Retail region mismatch '+hex(lo))
    for a,v in ANCHORS.items():
        if word(a)!=v:raise ValueError('Retail opcode mismatch '+hex(a))
    for a,dest in CALLS.items():
        ins=word(a)
        if ins>>26!=3 or ((ins&0x03ffffff)<<2)!=dest:
            raise ValueError('Original JAL target mismatch '+hex(a))
    def branch(addr):
        offset=word(addr)&65535
        if offset&32768:offset-=65536
        return addr+4+4*offset
    for addr,target in ((0x1f2950,0x1f29cc),(0x1f29cc,0x1f2a38),
                        (0x1f29dc,0x1f2a20),(0x2ba42c,0x2ba55c),
                        (0x2ba4b8,0x2ba424)):
        if branch(addr)!=target:
            raise ValueError('Original loop branch differs '+hex(addr))
    assert word(0x2ba428)&0xffff==10
    assert word(0x2ba5f4)&0xffff==0x3a
    assert word(0x1f29cc)>>26==5 # BNE at delimiter decision
    assert word(0x1f2950)>>26==0x15 # BNEL at EOF check
    return dict(original_elf_sha256=ELF_SHA,
        complete_regions=len(REGIONS),
        original_instruction_words=sum((b-a)//4 for a,b in REGIONS),
        exact_opcode_anchors=len(ANCHORS), original_JAL_edges=len(CALLS),
        delimiter_colon=0x3a,delimiter_line_feed=0x0a,
        source_pipeline=['0x2bb03c','0x2ba218','0x2ba3ac','0x2ba584','0x1f28c4'],
        original_tree_entry_deserialization_complete=False,
        playable_native_windows=False)
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('original_elf',type=Path)
    result=verify(parser.parse_args().original_elf)
    print('PASS: original lexical source-to-attribute parser call graph')
    print(json.dumps(result,indent=2))
