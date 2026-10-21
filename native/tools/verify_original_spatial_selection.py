#!/usr/bin/env python3
"""Verify original Ape Escape 3 USA spatial-selection and registry logic.

Uses a private, hash-pinned original game executable. Outputs metadata,
not proprietary original code or a PlayStation machine-code runtime.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct

SHA='5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
OPCODES='''
21d7d8:27bdffe0 21d7ec:8c840000 21d7f0:50800005
21d7f8:0c1068ec 21d800:ae000000 21d804:8e0f0004 21d808:51e00012
21d810:8e0f0014 21d814:19e0000b 21d81c:240f0070
21d820:8e040004 21d824:022f7818 21d828:008f2021 21d82c:0c086ce1
21d834:8e0f0014 21d838:022f782a 21d83c:15e0fff8 21d840:240f0070
21d844:0c0dbb7a 21d84c:ae000004 21d850:8e040010 21d854:50800007
21d85c:0c0dbb7a 21d864:240fffff 21d868:ae000010 21d86c:ae0f000c
21b404:27bdfff0 21b410:8c8f0030 21b418:95ef0002 21b41c:00af282a
21b420:10a00005 21b428:95c50004 21b42c:8c840000 21b430:08087621
21b438:0000102d 21b444:8c8f0030 21b448:00052840 21b454:95e2000a
21b458:00052900 21b45c:00a42821 21b460:03e00008 21b464:78a20040
21b4e0:00052880 21b4e4:00a42821 21b4ec:8ca20004
21b4f0:00052880 21b4f4:00a42821 21b4fc:8ca20010
21b500:03e00008 21b504:8c82001c
21d884:240f0070 21d888:8c820004 21d88c:00af2818 21d890:03e00008 21d894:00451021
21d898:27bdffa0 21d8b0:0000802d 21d8cc:0c138f05 21d8d0:8c840000
21d8d4:944f0000 21d8d8:11e00035 21d8e4:c5f43e7c
21d8f8:00157040 21d8fc:8e840004 21d900:26cf0002 21d908:01cf8021
21d90c:24110070 21d910:960f0000 21d918:01f17818
21d91c:0c086de2 21d920:008f2021 21d924:304200ff 21d928:5040001c
21d930:1260002b 21d93c:01f17818 21d940:0c086d1a
21d974:46010004 21d978:46140034 21d97c:45020007
21d984:960f0000 21d988:46000506 21d98c:8e8e0004
21d994:01cf9821 21d99c:26b50001 21d9a0:02af782a 21d9a4:15e0ffd5
21d9ac:0260802d 21d9b0:0200102d 21da20:46000504
21da24:1000ffdd
21da2c:27bdffc0 21da4c:8c8f0000 21da50:8def0004 21da54:8def0024
21da58:19e00026 21da60:3c0f006e 21da68:c5f43e80
21da78:12200027 21da7c:240f0070 21da88:0c086d1a
21da90:48a20800 21da94:48b41000 21da9c:4be1086a
21dabc:46010004 21dac0:46140034 21dac4:45020005
21dacc:46000506 21dad0:8e6f0004 21dad4:01f08821
21dadc:26520001 21dae8:024f782a 21daec:15e0ffe2
21daf0:0220102d 21db24:0c086d1a 21db58:46000504
'''
CALLS={
    0x21d7f8:0x41a3b0,0x21d82c:0x21b384,
    0x21d844:0x36ede8,0x21d85c:0x36ede8,
    0x21b430:0x21d884,0x21d8cc:0x4e3c14,
    0x21d91c:0x21b788,0x21d940:0x21b468,
    0x21d9ec:0x21b468,0x21da88:0x21b468,
    0x21db24:0x21b468
}
class ELF:
    def __init__(self,path):
        self.content=path.read_bytes()
        if hashlib.sha256(self.content).hexdigest()!=SHA:
            raise ValueError('Exact original USA SCUS_975.01 SHA-256 required')
        if self.content[:7]!=b'\x7fELF\x01\x01\x01':
            raise ValueError('Expected little-endian ELF32')
        ph=struct.unpack_from('<I',self.content,28)[0]
        stride,n=struct.unpack_from('<HH',self.content,42)
        self.load=[]
        for i in range(n):
            typ,off,va,_,size,_,flags,_=struct.unpack_from('<8I',self.content,ph+i*stride)
            if typ==1 and off+size<=len(self.content):
                self.load.append((va,va+size,off))
        if not self.load:raise ValueError('Original ELF has no load segments')
    def word(self,va):
        for lo,hi,off in self.load:
            if lo<=va<=hi-4:
                return struct.unpack_from('<I',self.content,off+va-lo)[0]
        raise ValueError('Unmapped original executable address '+hex(va))

def verify(path):
    elf=ELF(path)
    checks={int(x.split(':')[0],16):int(x.split(':')[1],16)
            for x in OPCODES.split()}
    for pc,word in checks.items():
        got=elf.word(pc)
        if got!=word:
            raise ValueError('Original MIPS instruction mismatch '+hex(pc))
    for pc,expected in CALLS.items():
        w=elf.word(pc)
        result=((pc+4)&0xf0000000)|((w&0x3ffffff)<<2)
        if w>>26 not in (2,3) or result!=expected:
            raise ValueError('Original branch differs at '+hex(pc))
    if elf.word(0x6e3e7c)!=0x7f7fffff or elf.word(0x6e3e80)!=0x7f7fffff:
        raise ValueError('Original spatial distance bound is not FLT_MAX')
    return {
        'format':'ae3/original-spatial-selection-registry/v1',
        'source_sha256':SHA,
        'verified_instruction_sites':len(checks),
        'verified_direct_branch_targets':len(CALLS),
        'spatial_select_grid':'0x21d898',
        'spatial_select_all':'0x21da2c',
        'registry_teardown':'0x21d7d8',
        'source_record_stride':0x70,
        'initial_distance_float_bits':'0x7f7fffff',
        'unported_engine_dependencies':[
            '0x4e3c14 grid-hit construction','0x21b788 candidate eligibility',
            '0x21b468 VU0 position','VU0 distance and sqrt arithmetic',
            '0x21b384 record destructor','0x36ede8 allocation release'],
        'limitation':'Verified control flow and native records; not a playable game'
    }
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('original_elf',type=Path)
    p.add_argument('--out',type=Path)
    a=p.parse_args()
    result=verify(a.original_elf)
    payload=json.dumps(result,indent=2)+'\n'
    if a.out:a.out.parent.mkdir(parents=True,exist_ok=True);a.out.write_text(payload,encoding='utf-8')
    else:print(payload,end='')
    print('PASS original spatial registry checks:',result['verified_instruction_sites'])
if __name__=='__main__':main()
