#!/usr/bin/env python3
"""Pin the actual USA retail parser 0x2BA584..0x2BABDC and decoded record layout."""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ELF_SHA = '5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
WHOLE = (0x2BA584,0x2BABDC,
         'c59bec02a0167117306e2622dfe2aefd7882e7a92d3acee879dfcaffce0ee32b')
# Independently chosen exact opcodes distributed across the original parser.
ANCHORS={
 0x2ba5b0:0x8c8f0010,0x2ba5d8:0x8e05000c,0x2ba5e8:0x24a50004,
 0x2ba5f4:0x2406003a,0x2ba5fc:0x0c07ca31,
 0x2ba604:0x8fa40010,0x2ba650:0x0240202d,
 0x2ba694:0x8d8f0008,0x2ba6a4:0x0c10a312,0x2ba6a8:0x8d840008,
 0x2ba6c8:0xae02001c,0x2ba6f0:0x0c10a312,0x2ba6f4:0x8da4000c,
 0x2ba6f8:0xae020020,0x2ba6fc:0x8e0f001c,0x2ba700:0x11e000bd,
 0x2ba714:0x24040034,0x2ba718:0x0c106e00,0x2ba738:0xac4d0004,
 0x2ba770:0x8e85000c,0x2ba78c:0x0c07ca31,
 0x2ba7d8:0x8fb00030,0x2ba814:0x91ef0000,0x2ba818:0xa22f0000,
 0x2ba858:0x0c189dc6,0x2ba874:0x01ee782b,
 0x2ba8a8:0x0c10a312,0x2ba8ac:0x8d840004,0x2ba8b0:0xae020008,
 0x2ba8b4:0x8e85002c,0x2ba8bc:0x10af00c4,
 0x2ba8cc:0xacaf0000,0x2ba9c0:0xadee0014,
 0x2ba9f8:0x8e050010,0x2baa10:0x24a5fff8,
 0x2baa24:0x0c07ca31,0x2baa90:0x0c10a312,
 0x2baa98:0xae020024,0x2baac4:0x2665fffc,
 0x2baad8:0x0c07ca31,0x2bab40:0x0c10a312,
 0x2bab44:0x8da40004,0x2bab54:0x01e27821,0x2bab60:0xae0f0004,
}
CALLS={
 0x2ba5fc:0x1f28c4,
 0x2ba654:0x627718,
 0x2ba6a4:0x428c48,
 0x2ba6f0:0x428c48,
 0x2ba718:0x41b800,
 0x2ba78c:0x1f28c4,
 0x2ba858:0x627718,
 0x2ba8a8:0x428c48,
 0x2ba8f0:0x541564,
 0x2ba994:0x5415e4,
 0x2ba9c4:0x4c59cc,
 0x2baa24:0x1f28c4,
 0x2baa90:0x428c48,
 0x2baad8:0x1f28c4,
 0x2bab40:0x428c48,
 0x2babd0:0x541408,
}
def verify(path):
    binary=Path(path).read_bytes()
    if hashlib.sha256(binary).hexdigest()!=ELF_SHA:
        raise ValueError('Wrong USA original retail SCUS_975.01')
    if binary[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',binary,18)[0]!=8:
        raise ValueError('Expected original little-endian MIPS ELF32')
    phoff=struct.unpack_from('<I',binary,28)[0]
    step,n=struct.unpack_from('<HH',binary,42)
    segments=[]
    for i in range(n):
        kind,offset,va,_,size,_,_,_=struct.unpack_from('<8I',binary,phoff+i*step)
        if kind==1 and offset+size<=len(binary):
            segments.append((va,va+size,offset))
    def read(va,size):
        for lo,hi,off in segments:
            if lo<=va and va+size<=hi:
                return binary[off+va-lo:off+va-lo+size]
        raise ValueError('Unmapped original ELF address '+hex(va))
    def word(va):return struct.unpack('<I',read(va,4))[0]
    start,end,digest=WHOLE
    if hashlib.sha256(read(start,end-start)).hexdigest()!=digest:
        raise ValueError('Original complete 0x2BA584 parser has changed')
    for pc,expected in ANCHORS.items():
        if word(pc)!=expected:
            raise ValueError(f'Original opcode mismatch {pc:#x}')
    for pc,target in CALLS.items():
        w=word(pc)
        if w>>26!=3 or (((pc+4)&0xf0000000)|((w&0x03ffffff)<<2))!=target:
            raise ValueError(f'Original JAL mismatch at {pc:#x}')
    def branch(pc):
        n=word(pc)&0xffff
        if n>=0x8000:n-=0x10000
        return pc+4+4*n
    for pc,target in ((0x2ba700,0x2ba9f8),
                      (0x2ba8bc,0x2babd0),
                      (0x2ba9ec,0x2ba718)):
        if branch(pc)!=target:
            raise ValueError(f'Original record loop branch differs {pc:#x}')
    assert word(0x2ba5f4)&0xffff==ord(':')
    assert word(0x2ba714)&0xffff==0x34
    # New record byte/word stores precisely one field per offset.
    for va,opcode,offset in ((0x2ba818,0x28,0),
                             (0x2ba8b0,0x2b,8),
                             (0x2ba6c8,0x2b,0x1c),
                             (0x2ba6f8,0x2b,0x20)):
        w=word(va)
        if w>>26!=opcode or w&0xffff!=offset:raise ValueError('Source field store mismatch '+hex(va))
    assert word(0x2ba7d8)&0xffff==0x30
    assert word(0x2ba8ac)&0xffff==0x4 # source child field 1
    assert word(0x2ba6a8)&0xffff==0x8 # source header field 0
    assert word(0x2ba6f4)&0xffff==0xc # source header field 3
    assert word(0x2ba9c0)&0xffff==0x14 # map stores child pointer, not the numeric offset
    return {
      'original_elf_sha256':ELF_SHA,
      'original_parser_region':[hex(start),hex(end)],
      'whole_instruction_words':(end-start)//4,
      'independent_opcode_anchors':len(ANCHORS),
      'verified_JAL_edges':len(CALLS),
      'original_header_child_count_offset':0x1c,
      'original_header_fourth_field_offset':0x20,
      'original_child_allocation_bytes':0x34,
      'original_child_leading_character_offset':0x0,
      'original_child_string_handle_offset':0x4,
      'original_child_numeric_offset_field':0x8,
      'original_key_tree_node_value_field':0x14,
      'original_key_tree_node_value_is_child_pointer':True,
      'full_original_source_materialization_complete':False,
      'original_game_playable':False}
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('original_elf',type=Path)
    result=verify(parser.parse_args().original_elf)
    print('PASS: original record header, child allocation, three child fields and map value pointer')
    print(json.dumps(result,indent=2))
