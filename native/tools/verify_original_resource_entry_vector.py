#!/usr/bin/env python3
"""Audit authentic USA Ape Escape 3 16-byte resource vector and PCK fixture.

Private inputs only. No original game code or assets copied to repository.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import struct
from pathlib import Path
ELF_SHA = '5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
PCK_SHA = '185db8cfd710760d9c715e0d1cce8ac4ecf6b18201673ce00bc6815078ae4615'
REGIONS = {
 (0x26d120,0x26d138): 'fc2d9b311b0daa5714080c19c0aab726f214600f658e5f27b8a58da8cbb7cd5d',
 (0x26d138,0x26d1b0): '342bc3c1078652be0153882d5e9ea84506858d58ac156875ccdced5547a9675d',
 (0x26d1b0,0x26d214): '30a4ecf5ec53fa7024d040df377756f56121fcef5e2ab56a4a7fd75cf9001e5d',
 (0x26d214,0x26d24c): '648002f376aaf708815a744769851335aa5ba82f445ab485990b58ca2be6090b',
 (0x26d24c,0x26d35c): 'dcb06b243e56d8f481c5af26b6c6b6c843b76833b4599a660c00518f8c716a3e',
 (0x35fcb0,0x35fd80): '3c8eee3f1ce66ad50443f7cb5a005ed610434b4925502f952a193ef0047c682e',
 (0x4b4350,0x4b4424): 'f48e4e06082ad96b220cc4c3a01f62ed6c5358b26a14426be4fc4a6d3667f49d',
 (0x42ce60,0x42ce90): 'c96ea5733147b062798665ad16aa53e7b3043256c51ce8157df42bc62e8d20ad',
 (0x42d148,0x42d164): '6be812d80f3ff307137a6ee2caf456a48186802d33cb2dfd15a029832b8e8a06',
}
ANCHORS = {
 0x26d120:0x8c8f0000,0x26d124:0x8dee0000,0x26d128:0x8de20004,
 0x26d12c:0x004e1023,0x26d134:0x00021103,
 0x26d148:0x8c8c0000,0x26d14c:0x8d8e0004,0x26d154:0x8d8f0008,
 0x26d168:0x8daf0000,0x26d170:0x8daf0004,
 0x26d178:0x8daf0008,0x26d180:0x8daf000c,0x26d190:0xad8f0004,
 0x26d1a0:0x0c145bc2,
 0x26d1b0:0x8c840000,0x26d1f4:0x018f7823,0x26d208:0xac8e0004,
 0x26d214:0x8c8f0000,0x26d218:0x00052900,
 0x26d21c:0x8de20000,0x26d224:0x00451021,
 0x26d228:0x8c8f0000,0x26d22c:0x8dee0008,
 0x26d230:0xac8f0004,0x26d238:0xac8f0008,
 0x26d240:0x01ee7821,
 0x26d280:0x0c09b46c,0x26d28c:0x19e00027,
 0x26d29c:0x8eaf0008,0x26d2a4:0x8e8e0004,
 0x26d2a8:0x0c10b398,0x26d2bc:0x0c10b452,
 0x26d2c4:0x14400014,0x26d2cc:0x81ee0000,
 0x26d2d0:0x11c00003,0x26d2d4:0x240f0020,
 0x26d2d8:0x55cf0010,0x26d2e0:0x8eaf0008,
 0x26d2e8:0x8e8b0000,0x26d2f0:0x8e8c0008,
 0x26d2f4:0x8eae0000,0x26d2f8:0x01eb7821,
 0x26d2fc:0x8e8d000c,0x26d300:0x01cc7021,
 0x26d304:0xafb10004,0x26d308:0xafaf0000,
 0x26d30c:0xafae0008,0x26d310:0x0c09b44e,
 0x26d314:0xafad000c,0x26d324:0x15e0ffdd,
 0x26d328:0x26940010,0x26d32c:0x0c09b448,
 0x42ce60:0x30820007,0x42d148:0x14c00003,
 0x35fcbc:0x0c09b7ce,0x35fcc4:0x0c09b448,
 0x35fd18:0x8c440000,0x35fd20:0x0c10b113,
 0x35fd5c:0x8e050008,0x35fd64:0x0c06ed8f,0x35fd68:0x8c46000c,
 0x4b4354:0x3c0f006a,0x4b4358:0x25ef7d08,
 0x4b4364:0xac8f0000,0x4b43c0:0x25ef7d08
}
CALLS={0x26d1a0:0x516f08,
       0x26d280:0x26d1b0,0x26d2a8:0x42ce60,0x26d2bc:0x42d148,
       0x26d310:0x26d138,0x26d32c:0x26d120,
       0x35fcbc:0x26df38,0x35fcc4:0x26d120,
       0x35fd20:0x42c44c,0x35fd64:0x1bb63c,
       0x4b4368:0x18a634,0x4b4370:0x1a22b4,
       0x4b4380:0x4a7418}

def verify_elf(path: Path):
    data=path.read_bytes()
    if hashlib.sha256(data).hexdigest()!=ELF_SHA:raise ValueError('Not original private USA SCUS_975.01')
    if data[:7]!=b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',data,18)[0]!=8:
        raise ValueError('Expected ELF32 MIPS little endian')
    poff=struct.unpack_from('<I',data,28)[0]
    step,n=struct.unpack_from('<HH',data,42)
    seg=[]
    for i in range(n):
        kind,off,va,_,length,_,_,_=struct.unpack_from('<8I',data,poff+i*step)
        if kind==1 and off+length<=len(data):seg.append((va,va+length,off))
    def read(va,length):
        for start,end,off in seg:
            if start<=va and va+length<=end:return data[off+va-start:off+va-start+length]
        raise ValueError(f'Unmapped original VA: {va:#x}')
    def word(va):return struct.unpack('<I',read(va,4))[0]
    for (lo,hi),digest in REGIONS.items():
        if hashlib.sha256(read(lo,hi-lo)).hexdigest()!=digest:
            raise ValueError(f'Original region mismatch at {lo:#x}')
    for pc,opcode in ANCHORS.items():
        if word(pc)!=opcode:raise ValueError(f'Original instruction mismatch at {pc:#x}')
    for pc,target in CALLS.items():
        w=word(pc)
        if (w>>26)!=3 or (((pc+4)&0xf0000000)|((w&0x03ffffff)<<2))!=target:
            raise ValueError(f'Original call target mismatch at {pc:#x}')
    # 16 byte vector: sourceEnd-sourceBegin>>4, index <<4,
    # and four source words copied before end pointer advances by 16.
    assert word(0x26d134)&0x3f==3
    assert word(0x26d218)&0x3f==0
    assert (word(0x26d168)&65535,word(0x26d170)&65535,
            word(0x26d178)&65535,word(0x26d180)&65535)==(0,4,8,12)
    assert (word(0x35fd18)&65535,word(0x35fd5c)&65535,
            word(0x35fd68)&65535)==(0,8,12)
    assert word(0x26d190)&65535==4
    # Producer's four source words at +0 / +4 / +8 / +C.
    assert (word(0x26d308)&65535,word(0x26d304)&65535,
            word(0x26d30c)&65535,word(0x26d314)&65535)==(0,4,8,12)
    # C-string type matching requires exact prefix followed by NUL
    # or original ASCII 0x20, not arbitrary suffixes.
    assert word(0x26d2d4)&65535==0x20
    assert word(0x26d2e8)&65535==0
    assert word(0x26d2a4)&65535==4
    assert word(0x26d2f0)&65535==8
    # Critical trap: bytes b'PCK\0' at VA 0x6A7D08 are not a
    # retail PCK signature string. They are the little-endian
    # function pointer 0x004B4350, stored as vtable data!
    assert read(0x6a7d08,4)==b'PCK\0'
    assert word(0x6a7d08)==0x004b4350
    assert word(0x6a7d0c)==0x004b43b8
    assert word(0x4b4364)>>26==43  # SW vtable pointer at object+0
    assert word(0x4b4358)&0xffff==0x7d08
    return {'complete_instruction_windows':len(REGIONS),
            'original_instruction_words':sum((end-start)//4 for start,end in REGIONS),
            'exact_opcode_anchors':len(ANCHORS),
            'verified_direct_calls':len(CALLS),
            'original_entry_bytes':16,
            'read_name_offset':0,
            'read_payload_pointer_offset':8,
            'read_payload_length_offset':12,
            'source_entry_word_at_04':'type string pointer',
            'source_PCK_entry_table_offset':0x10,
            'source_PCK_header_count_offset':0x0c,
            'type_match':'requested prefix followed by NUL or ASCII space',
            'elf_PCK_bytes_are_vtable_pointer':True,
            'original_PCK_file_loader_reverse_engineered':False}

def verify_pck(path: Path):
    raw=path.read_bytes()
    if hashlib.sha256(raw).hexdigest()!=PCK_SHA:raise ValueError('Not the original private woods_c/exdb.pck')
    assert raw[:4]==b'PCK\0'
    table,count=struct.unpack_from('<II',raw,4)
    assert (table,count,len(raw))==(64,2,5728)
    rows=[struct.unpack_from('<4I',raw,table+16*i) for i in range(count)]
    assert rows==[(42,37,96,5120),(16,37,5216,512)]
    # Names/types are absolute offsets into the original source
    # byte string pool. No retail assets are emitted or embedded.
    def name(pos):return raw[pos:raw.index(b'\0',pos)].decode('ascii')
    assert [(name(a),name(t)) for a,t,_,_ in rows]==[
        ('monkeyprofile.exdb','exdb'),('MainCameraParam.exdb','exdb')]
    assert raw[96:100]==b'EDB\n'
    assert raw[5216:5220]==b'EDB\n'
    return {'pck_sha256':PCK_SHA,'original_resource_entries':count,
            'source_package_offsets':[row[2] for row in rows],
            'source_entry_lengths':[row[3] for row in rows]}

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('original_elf',type=Path)
    p.add_argument('private_woods_c_exdb_pck',type=Path)
    args=p.parse_args()
    result={'source':verify_elf(args.original_elf),
            'private_asset':verify_pck(args.private_woods_c_exdb_pck)}
    print('PASS authentic retail resource vector and PCK resource binding')
    print(json.dumps(result,indent=2))
