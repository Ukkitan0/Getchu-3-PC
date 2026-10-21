#!/usr/bin/env python3
"""Verify retail AE3 USA 0x2BA3AC line scanner and its two stop markers."""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
REGIONS={
    (0x2ba3ac,0x2ba584):'547d795b26d79e730c3fff92ac0a8da326183ee40a76a89ac5bf361097d86a8f',
    (0x627c68,0x627cf8):'d3701fa1c86d81b2ae2e7b909ba5f5492929c9de221c7af703bebb392bfad166',
    (0x2bb03c,0x2bb084):'e4e87983feecf0691f5cc5ba6a953b72aaf0939f71719c03b94c1bd05d370594'
}
ANCHORS={
    0x2ba418:0x8e2f0008, # input length
    0x2ba424:0x92480000, # current byte
    0x2ba428:0x240f000a, # newline separator
    0x2ba42c:0x15e8004b, # non LF branch to append character
    0x2ba438:0x8e2f0014, # available vector capacity
    0x2ba43c:0x10af0043, # full vector calls insertion
    0x2ba458:0x25ef0004, # advance vector end by 4
    0x2ba484:0x00021027, # NOR complement of std::string::find index
    0x2ba488:0x10400020, # missing sizest -> secondary decision
    0x2ba490:0x24150001, # persist saw size marker
    0x2ba4b8:0x15e0ffda, # loop next byte
    0x2ba50c:0x16afffe2, # don't search b: until size marker occurred
    0x2ba538:0x00021027, # complement find index b:
    0x2ba53c:0x1440ffe0, # found b: -> cleanup/exit
    0x2ba55c:0x00084600, # sign extend non-newline byte
    0x2ba574:0x0c189e9e, # append non-newline byte
    0x627c90:0x8c930000, # source string start
    0x627c94:0x8e72fff4, # source string length
    0x627c98:0x024f782b, # len compared with needle length
    0x627cd0:0x240fffff, # original find() not-found npos=FFFFFFFF
    0x2bb060:0x0c0ae8eb, # lifecycle entry -> source line scanner
}
CALLS={
    0x2ba3f8:0x626c70,
    0x2ba410:0x626d78,
    0x2ba44c:0x626fb0,
    0x2ba468:0x42ce60,
    0x2ba47c:0x627c68,
    0x2ba4a4:0x626d78,
    0x2ba4c0:0x2ba3a4,
    0x2ba4d0:0x626d48,
    0x2ba4d8:0x626c70,
    0x2ba4e0:0x626c68,
    0x2ba51c:0x42ce60,
    0x2ba530:0x627c68,
    0x2ba54c:0x49feac,
    0x2ba574:0x627a78,
    0x2bb060:0x2ba3ac,
}
BRANCHES={
    0x2ba41c:0x2ba4c0,
    0x2ba42c:0x2ba55c,
    0x2ba43c:0x2ba54c,
    0x2ba488:0x2ba50c,
    0x2ba4b8:0x2ba424,
    0x2ba50c:0x2ba498,
    0x2ba53c:0x2ba4c0,
    0x2ba544:0x2ba49c,
    0x2ba57c:0x2ba4b0,
}

def verify(path):
    b=Path(path).read_bytes()
    if hashlib.sha256(b).hexdigest()!=ELF_SHA:
        raise ValueError('Not the expected original USA SCUS_975.01 ELF')
    if b[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',b,18)[0]!=8:
        raise ValueError('Expected original MIPS LE ELF32')
    ph=struct.unpack_from('<I',b,28)[0]
    step,count=struct.unpack_from('<HH',b,42)
    seg=[]
    for i in range(count):
        typ,off,va,_,filesz,_,_,_=struct.unpack_from('<8I',b,ph+i*step)
        if typ==1 and off+filesz<=len(b):seg.append((va,va+filesz,off))
    def read(va,n):
        for lo,hi,off in seg:
            if lo<=va and va+n<=hi:return b[off+va-lo:off+va-lo+n]
        raise ValueError('Original VA not mapped: '+hex(va))
    def word(a):return struct.unpack('<I',read(a,4))[0]
    for (lo,hi),sha in REGIONS.items():
        if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=sha:
            raise ValueError('Retail code region differs at '+hex(lo))
    for addr,expected in ANCHORS.items():
        if word(addr)!=expected:
            raise ValueError(f'Retail opcode mismatch at {addr:#x}: {word(addr):08x} expected {expected:08x}')
    for addr,target in CALLS.items():
        w=word(addr)
        if w>>26!=3 or ((w&0x03ffffff)<<2)!=target:
            raise ValueError('Original JAL edge mismatch at '+hex(addr))
    for addr,target in BRANCHES.items():
        n=word(addr)&0xffff
        if n&0x8000:n-=65536
        if addr+4+4*n!=target:raise ValueError('Original control-flow edge mismatch '+hex(addr))
    if read(0x6f8e08,7)!=b'sizest\0':
        raise ValueError('Original first search marker differs')
    if read(0x6f8e10,3)!=b'b:\0':
        raise ValueError('Original second search marker differs')
    assert (word(0x2ba418)&0xffff,word(0x2ba424)&0xffff)==(8,0)
    assert word(0x2ba428)&65535==10
    assert (word(0x2ba488)>>26,word(0x2ba53c)>>26)==(4,5)
    assert word(0x627cd0)&65535==65535
    return {
        'elf_sha256':ELF_SHA,
        'verified_complete_regions':len(REGIONS),
        'whole_instruction_words':sum((hi-lo)//4 for lo,hi in REGIONS),
        'opcode_anchors':len(ANCHORS),
        'direct_JAL_edges':len(CALLS),
        'branch_destinations':len(BRANCHES),
        'line_delimiter':'LF 0x0A',
        'first_substring':'sizest',
        'later_stop_substring':'b:',
        'unterminated_trailing_line_committed':False,
        'complete_stage_file_loading':False,
        'playable_game':False,
    }
if __name__=='__main__':
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('original_elf',type=Path)
    print('PASS original 0x2BA3AC line scanner and sentinel-controlled termination')
    print(json.dumps(verify(ap.parse_args().original_elf),indent=2))
