#!/usr/bin/env python3
"""Verify original Ape Escape 3 USA sound descriptor 0x1BB9A8."""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ELF_SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
REGION=(0x1bb9a8,0x1bbb18)
REGION_SHA='441da325cbbcf3e04118db2e833a692ed0ed7ad8e11381c6e942ddb4616afb04'
STRING_FIELDS=[
    (0x6d64b0,'noname',0x6d64b8,'name',0x00,0x1bb9d0,0x1bb9e8,0x1bba04,0x1bba0c),
    (0x6d64c0,'M01.mid',0x6d64c8,'midi',0x20,0x1bba08,0x1bba20,0x1bba3c,0x1bba44),
    (0x6d64d0,'system',0x6d64d8,'reverb',0x40,0x1bba40,0x1bba58,0x1bba74,0x1bba7c),
    (0x6d64e0,'M01.bd',0x6d64e8,'vb',0x60,0x1bba78,0x1bba90,0x1bbaac,0x1bbab4),
    (0x6d64f0,'M01.hd',0x6d64f8,'vh',0x80,0x1bbab0,0x1bbac8,0x1bbaf8,0x1bbaf0)
]

def verify(elf):
    binary=Path(elf).read_bytes()
    if hashlib.sha256(binary).hexdigest()!=ELF_SHA:
        raise ValueError('Expected original USA SCUS_975.01')
    if binary[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',binary,18)[0]!=8:
        raise ValueError('Expected MIPS ELF32 little endian')
    phoff=struct.unpack_from('<I',binary,28)[0]
    phentsize,phnum=struct.unpack_from('<HH',binary,42)
    segments=[]
    for idx in range(phnum):
        typ,offset,virt,_,size,_,_,_=struct.unpack_from('<8I',binary,phoff+idx*phentsize)
        if typ==1 and offset+size<=len(binary):segments.append((virt,virt+size,offset))
    def read(va,count):
        for start,end,offset in segments:
            if start<=va and va+count<=end:return binary[offset+va-start:offset+va-start+count]
        raise ValueError(f'Unmapped original ELF virtual address {va:#x}')
    def word(va):return struct.unpack('<I',read(va,4))[0]
    def assert_jal(at,target):
        w=word(at)
        if w>>26!=3 or (((at+4)&0xf0000000)|((w&0x3ffffff)<<2))!=target:
            raise ValueError(f'Original JAL differs at {at:#x}')
    if hashlib.sha256(read(REGION[0],REGION[1]-REGION[0])).hexdigest()!=REGION_SHA:
        raise ValueError('Original audio descriptor instruction window differs')
    field_report=[]
    for default_va,default,key_va,key,offset,string_call,lookup_call,sq_lo,sq_hi in STRING_FIELDS:
        if read(default_va,len(default)+1)!=default.encode('ascii')+b'\0':
            raise ValueError('Original string default differs: '+default)
        if read(key_va,len(key)+1)!=key.encode('ascii')+b'\0':
            raise ValueError('Original source attribute key differs: '+key)
        assert_jal(string_call,0x35ab88)
        assert_jal(lookup_call,0x2bb3bc)
        for addr,src,off in ((sq_lo,15,offset),(sq_hi,14,offset+16)):
            w=word(addr)
            if (w>>26,(w>>21)&31,(w>>16)&31,w&65535)!=(0x1f,16,src,off):
                raise ValueError(f'Original 128-bit field store differs: {addr:#x}')
        field_report.append(dict(key=key,default=default,original_slot_offset=offset))
    if read(0x6d6500,len('volume_scale')+1)!=b'volume_scale\0':
        raise ValueError('Original float attribute key differs')
    if word(0x6d6510)!=0x3f800000:
        raise ValueError('Original float fallback differs')
    assert_jal(0x1bbaf4,0x2bb1d0)
    w=word(0x1bbafc)
    if (w>>26,(w>>21)&31,(w>>16)&31,w&65535)!=(0x39,16,0,0xa0):
        raise ValueError('Original SWC1 volume-scale field differs')
    assert_jal(0x1bb4a0,0x1bb9a8)
    if word(0x1bb488)&0xffff!=0xb0:
        raise ValueError('Original 0xB0 record stride differs')
    return {
        'original_elf_sha256':ELF_SHA,
        'original_audio_descriptor_region':[hex(x) for x in REGION],
        'original_instructions':(REGION[1]-REGION[0])//4,
        'verified_lookup_calls':11,
        'verified_call_edges_including_producer':12,
        'original_string_fields':field_report,
        'original_volume_field':{'key':'volume_scale','default':1.0,'offset':0xa0},
        'source_record_stride':0xb0,
        'navrange_resource_loader_verified':False,
        'serialized_attribute_parser_ported':False
    }
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('original_elf',type=Path)
    result=verify(p.parse_args().original_elf)
    print('PASS: original 0x1BB9A8 audio descriptor source literals, calls and field layout')
    print(json.dumps(result,indent=2))
