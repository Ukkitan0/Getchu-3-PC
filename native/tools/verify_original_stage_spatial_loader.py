#!/usr/bin/env python3
"""Hash-pin and audit source-code recovery of original AE3 stage registry.

Reads the owner-private SCUS_975.01 ELF; outputs only addresses, hashes,
counts and runtime requirements. No commercial code or executable copied.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

PIN='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
WINDOWS={
 (0x21d46c,0x21d6dc):'a95070408be86d5c0acb58fc0cedc60ea9401fec40045b4430d89e2151e44e66',
 (0x21d6dc,0x21d7d8):'346ef7f055832b57cd99f8e6cdb1c6c829fe8b949d09b94947724b0a9d808006',
 (0x21b254,0x21b288):'96ac4774523e88cae5d550e71c1d2e4b77ad138d15520ea401997b4090610c62',
}
OPCODES={
 0x21d4ac:0x8e040024,0x21d4b4:0x8e0a0038,0x21d4c0:0x8e0d003c,
 0x21d4c4:0x8e0c0040,0x21d4dc:0x8e0f0044,0x21d4d4:0x8e0b004c,
 0x21d4f4:0xac4f0014,0x21d4fc:0xac4c0010,0x21d504:0xac4e0018,
 0x21d50c:0xac4b001c,0x21d524:0x8e0f0024,0x21d594:0x8d6d000c,
 0x21d5a4:0xadf20030,0x21d5a8:0x964f0016,
 0x21d5dc:0x95ef0018,0x21d5e8:0x014f7821,
 0x21d5f0:0x71ae74c8,0x21d5f4:0x8ded0000,
 0x21d5f8:0x8def0004,0x21d5fc:0x71ed7cc8,
 0x21d600:0x71cf74c8,0x21d604:0x7d6e0000,
 0x21d610:0x964e0000,0x21d618:0x11cf0069,
 0x21d63c:0xadaf001c,0x21d658:0x95ee000a,
 0x21d660:0x11cf002a,0x21d674:0x11cf0019,
 0x21d704:0x1000ffdd,0x21d790:0x71af34c8,
 0x21b254:0xac850000,0x21b284:0xac8f0024,
}
CALLS={
 0x21d4a4:0x41b800,0x21d518:0x36edac,0x21d544:0x21b254,
 0x21d724:0x41b800,0x21d734:0x21b0b0,0x21d7a0:0x21b240,
}
def verify(path:Path):
    data=path.read_bytes()
    if hashlib.sha256(data).hexdigest()!=PIN:
        raise ValueError('Wrong original USA Ape Escape 3 ELF hash')
    if data[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',data,18)[0]!=8:
        raise ValueError('Expected little-endian MIPS ELF32')
    ph=struct.unpack_from('<I',data,28)[0]
    stride,count=struct.unpack_from('<HH',data,42)
    segments=[]
    for i in range(count):
        kind,off,va,_,size,_,_=struct.unpack_from('<8I',data,ph+i*stride)
        if kind==1 and off+size<=len(data): segments.append((va,va+size,off))
    def read(va,n):
        for a,b,off in segments:
            if a<=va and va+n<=b:return data[off+va-a:off+va-a+n]
        raise ValueError('Original ELF VA is unmapped: '+hex(va))
    def word(va):return struct.unpack('<I',read(va,4))[0]
    for (a,b),sha in WINDOWS.items():
        if hashlib.sha256(read(a,b-a)).hexdigest()!=sha:
            raise ValueError('Original code section changed at '+hex(a))
    for pc,w in OPCODES.items():
        if word(pc)!=w:
            raise ValueError('Retail opcode mismatch at '+hex(pc))
    for pc,target in CALLS.items():
        w=word(pc)
        decoded=((pc+4)&0xf0000000)|((w&0x3ffffff)<<2)
        if (w>>26 not in (2,3)) or decoded!=target:
            raise ValueError('Original J/JAL mismatch at '+hex(pc))
    return dict(format='ae3/original-stage-to-spatial-record-constructor/v1',
       source_sha256=PIN,
       original_entry='0x21d46c',
       original_record_initializer='0x21b254',
       verified_code_regions=len(WINDOWS),
       verified_original_instruction_words=sum((b-a)//4 for a,b in WINDOWS),
       independently_verified_opcodes=len(OPCODES),
       original_call_targets=len(CALLS),
       source_header_record_count_offset='0x24',
       source_header_relative_pointer_offsets=[hex(x) for x in range(0x38,0x50,4)],
       source_record_descriptor_size='0x1e',
       source_runtime_record_stride='0x70',
       source_vertex_table_stride='0x10',
       source_polygon_header_vertex_count_offset='0x16',
       source_polygon_header_vertex_indices_offset='0x18',
       original_more_complex_actions=['type 1: 0x21d70c','type 2: 0x21d6dc'],
       unresolved_original_dependencies=['0x36edac spatial allocator',
          '0x21b0b0 and 0x21b240 action-object management',
          '0x21d70c/0x21d6dc action instantiation',
          'original stage allocation lifetime and asset stream extraction'],
       original_game_native_windows_boot=False)
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('original_elf',type=Path)
    p.add_argument('--out',type=Path)
    args=p.parse_args()
    report=verify(args.original_elf)
    payload=json.dumps(report,indent=2)+'\n'
    if args.out:args.out.write_text(payload,encoding='utf-8')
    else: print(payload,end='')
    print('PASS %d original instruction words, %d call targets' % (
      report['verified_original_instruction_words'],report['original_call_targets']))
