#!/usr/bin/env python3
"""Check the authentic retail MIPS instructions behind source-record polygons.

Reads an owner-private USA SCUS_975.01, prints metadata only. Does not
emulate the PlayStation 2 or publish copyrighted original executable bytes.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

PIN = '5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
WINDOWS = {
    (0x21B468, 0x21B4AC): '3b99ca72e88c9213b2edde8e979927ce9a6c2b0e4252a1047e373f41029e08da',
    (0x21B788, 0x21B7C8): '7cab91f4e9d1bfafdcf462a06df08af3455f0d9d741bedd27ed73b1e6b820d1b',
    (0x21D8F8, 0x21D924): '23aca4c63cc7f439c3bb3cd30b81588cd7e5d9aee537c78beed9c9a55375b7e0',
    (0x21DA74, 0x21DA90): 'fcf493462e7901ad8f6f7d10e14c1d711eb75fa70a525c0b827b714718ef9b37',
}
ANCHORS = {
    0x21B46C:0x8C8E0030, # lw t6,0x30(a0) header pointer
    0x21B478:0x95CF0016, # lhu t7,0x16(t6) vertex count
    0x21B488:0x24840040, # a0 +=0x40 vertex data
    0x21B49C:0x24840010, # a0 +=0x10 per vertex
    0x21B788:0x8C8B0030, # lw t3,0x30(a0)
    0x21B794:0x956E0016, # lhu t6,0x16(t3)
    0x21B79C:0x01E47821, # pointer+16*count
    0x21B7A8:0x24840040, # a0 +=0x40 data
    0x21D900:0x26CF0002, # pointer+2 hit-list entries
    0x21D90C:0x24110070, # exact original record stride
    0x21D910:0x960F0000, # original u16 hit index
    0x21D918:0x01F17818, # multiply index *0x70
    0x21D920:0x008F2021, # a0 = registry base+record offset
    0x21DA7C:0x240F0070, # original full-registry stride
    0x21DA80:0x8E640004,
    0x21DA84:0x024F8018,
    0x21DA88:0x0C086D1A, # call original VU mean
}
def verify(path:Path):
    data=path.read_bytes()
    if hashlib.sha256(data).hexdigest()!=PIN:
        raise ValueError('Wrong original USA SCUS_975.01 SHA256')
    if data[:7] != b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',data,18)[0]!=8:
        raise ValueError('Expected original MIPS ELF32 little-endian')
    ph=struct.unpack_from('<I',data,28)[0]
    stride,n=struct.unpack_from('<HH',data,42)
    segments=[]
    for i in range(n):
        typ,off,va,_,size,_,_,_=struct.unpack_from('<8I',data,ph+i*stride)
        if typ==1 and off+size<=len(data):segments.append((va,va+size,off))
    def read(va,nbytes):
        for lo,hi,off in segments:
            if lo<=va and va+nbytes<=hi:return data[off+va-lo:off+va-lo+nbytes]
        raise ValueError('Unmapped game VA '+hex(va))
    for (start,end),digest in WINDOWS.items():
        if hashlib.sha256(read(start,end-start)).hexdigest()!=digest:
            raise ValueError('Code region changed at '+hex(start))
    for pc,expected in ANCHORS.items():
        if struct.unpack('<I',read(pc,4))[0]!=expected:
            raise ValueError('Retail instruction mismatch '+hex(pc))
    return {
        'format':'ae3/authentic-original-spatial-record-layout/v1',
        'source_sha256':PIN,
        'original_record_stride':0x70,
        'original_vertex_data_offset':0x40,
        'original_header_pointer_offset':0x30,
        'original_vertex_count_offset_in_header':0x16,
        'original_vector_stride':0x10,
        'verified_regions':len(WINDOWS),
        'verified_original_instruction_words':sum((end-start)//4 for start,end in WINDOWS),
        'verified_independent_opcode_sites':len(ANCHORS),
        'limitation':'Only known 0x70-byte source-record layouts with one to three vertices are decoded. Unresolved runtime header VA needs the actual game-owned allocations.',
    }
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('original_elf',type=Path)
    parser.add_argument('--out',type=Path)
    a=parser.parse_args()
    report=verify(a.original_elf)
    output=json.dumps(report,indent=2)+'\n'
    if a.out:
        a.out.parent.mkdir(parents=True,exist_ok=True)
        a.out.write_text(output,encoding='utf-8')
    else:print(output,end='')
    print('PASS original ELF: %d original instructions verified' % report['verified_original_instruction_words'])
