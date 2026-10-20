#!/usr/bin/env python3
"""Check decompiled original Monkey vtable methods against owner's real USA ELF.

Metadata only; game code and assets stay private. This is source verification,
not a PS2 interpreter or any form of hardware emulation.
"""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import struct

EXPECTED_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
EXPECTED={
    # Two accessors: this+4 -> actor+0x60 -> 0x08 and 0x38, respectively.
    0x571648:[0x8c8f0004,0x8def0060,0x03e00008,0x8de20008],
    0x571658:[0x8c8f0004,0x8def0060,0x03e00008,0x8de20038],
    # Setter: this+4 -> actor+0x60 -> byte 0x50 = 1.
    0x5716c4:[0x8c8f0004,0x8dee0060,0x240f0001,0x03e00008,0xa1cf0050],
    # Tail-called floating-point conversion, plus original data source offsets.
    0x571688:[0x27bdfff0,0xffbf0000,0x8c8f0004,0xdfbf0000,
              0x8def0060,0x8de40020,0x080c5a4a,0x27bd0010],
    0x316928:[0x8c8f0000,0x8def0000,0xc5e0037c,0x3c0f0071,
              0x25ef8278,0xc5e10000,0x03e00008,0x46010002],
}
AUX=[0x571648,0x571658,0x571668,0x571688,0x5716a8,
     0x5716c4,0x5716d8,0x57173c,0x57178c]

def validate(path:Path)->dict:
    data=path.read_bytes()
    if hashlib.sha256(data).hexdigest()!=EXPECTED_SHA:
        raise ValueError('Expected exact original USA SCUS_975.01 ELF SHA-256')
    if data[:7]!=b'\x7fELF\x01\x01\x01':
        raise ValueError('Expected original little-endian 32-bit ELF')
    phoff=struct.unpack_from('<I',data,28)[0]
    entsize,count=struct.unpack_from('<HH',data,42)
    mappings=[]
    for i in range(count):
        kind,off,va,_,size,_,flags,_=struct.unpack_from('<IIIIIIII',data,phoff+i*entsize)
        if kind==1 and off+size<=len(data):mappings.append((va,va+size,off))
    if not mappings:raise ValueError('Original ELF has no loadable segments')
    def word(va:int)->int:
        for low,high,offset in mappings:
            if low<=va<=high-4:
                return struct.unpack_from('<I',data,offset+va-low)[0]
        raise ValueError('Unmapped original ELF address: '+hex(va))
    for address,instructions in EXPECTED.items():
        observed=[word(address+4*i) for i in range(len(instructions))]
        if observed!=instructions:
            raise ValueError('Original ELF code changed at '+hex(address))
    if [word(0x6B14D0+4*i) for i in range(9)]!=AUX:
        raise ValueError('Original Monkey secondary vtable was not recovered correctly')
    # The original Monkey main virtual slot 6 (0x571AA4) includes the
    # exact 8-entry index advance at +0x2f0. This is a method fragment,
    # not the full game event or animation handler.
    if word(0x6B1498+6*4)!=0x571AA4:
        raise ValueError('Monkey main vtable entry 6 differs')
    for address,opcode in {0x571B88:0x8e2d02f0,0x571BA0:0x39af0007,
                           0x571BA8:0x25ad0001,0x571BAC:0x000f680a,
                           0x571BB4:0xae2d02f0}.items():
        if word(address)!=opcode:
            raise ValueError('Original Monkey slot cursor code differs at '+hex(address))
    if word(0x708278)!=0x3C8EFA34:
        raise ValueError('Original Monkey angle conversion constant differs')
    return {
        'format':'ae3/verified-retail-monkey-secondary-accessors/v1',
        'source_sha256':EXPECTED_SHA,
        'original_secondary_vtable':'0x6b14d0',
        'original_getter_word08':'0x571648',
        'original_getter_word38':'0x571658',
        'original_set_flag50':'0x5716c4',
        'original_angle_method':'0x571688',
        'original_angle_callee':'0x316928',
        'original_slot_method':'0x571aa4',
        'slot_field_offset':'0x2f0',
        'original_slot_increment_sites':['0x571b88','0x571ba0','0x571ba8','0x571bac','0x571bb4'],
        'angle_source_float_offset':'0x37c',
        'angle_factor_global':'0x708278',
        'angle_factor_ieee754_hex':'0x3c8efa34',
        'angle_factor_value':struct.unpack('<f',struct.pack('<I',0x3C8EFA34))[0],
        'confidence':'Exact instructions and vtable verified against original retail ELF',
        'unknowns':'Original subobject ownership, remaining method bodies and full gameplay lifecycle',
    }

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('original_elf',type=Path)
    parser.add_argument('--out',type=Path)
    args=parser.parse_args()
    report=validate(args.original_elf)
    formatted=json.dumps(report,indent=2)+'\n'
    if args.out:
        args.out.parent.mkdir(parents=True,exist_ok=True)
        args.out.write_text(formatted,encoding='utf-8')
    else:print(formatted,end='')
    print('PASS: original Monkey secondary vtable and 4 behavior methods match retail ELF')
if __name__=='__main__':main()
