#!/usr/bin/env python3
"""Verify actual USA SCUS_975.01 startup and 143 native constructor addresses.

Reads owner-private executable; emits audit metadata only, never game bytes.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

EXPECTED_SHA = "5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
CTOR_BODY_SHA = "01e600ce72db3b1290949e9f8f772559faa937eb8b034f4e6bae5688115d7b1b"
CTOR_TABLE_SHA = "a3c5739dcec7d153ee3e72a04b14046ce4e9366d68537258cab6bd168eac99ba"
FIRST_CTOR_SHA = "75a521c6fd605abeba2de1d1b6880f78ec824dbfe3e4e89a4db567ac669437d9"
FIRST_CTOR_DEP_SHA = "07cac5712617a817c73f96b849d99560b9bdfefd2b02f23ea94360020694a809"
SOURCE_NOOP_SHA = "6d64edf91449c1b17746c1ef18afa2eb25c70bdf1322ab3df5a2630993b7e2f1"
THIRD_CTOR_SOURCE_SHA = "a7ea32a26e18a2f6baf7e5341a5869caf968c5673a8818cd585473555005b68a"
THIRD_CTOR_LITERAL_SHA = "409f7ce3cbaeb6670123aa8e26359e36f118ec8d784f405ac78f0657ffd4e446"
FOURTH_CTOR_SHA = "769e555620bccceda273b52b0b90d2a24e8872ecaae788a126566f26d703c0fd"
FOURTH_ENTRY_SHA = "20552c8eb949f74f127d90eaef73b7f76e9878f3c332efc605a97d443fa9a341"
FOURTH_LITERAL_SHA = "d355cd4bb05ebf17c914cfb28d4cd2cdb2e2bb10a6959869af5cd051d1abc0f0"
FIFTH_WRAPPER_SHA = "60f74f34bd5b454ca0933867b130b26308d06094067024ecaad34bfc83a8ad46"
FIFTH_CALL_SHA = "22f7ce43f757ec3635a967cdffc7f06c715a3fb8f773654b7fd2a7283f633b20"
SIXTH_WRAPPER_SHA = "ac1a87f54e8c6a62b046f07c871394582e8670565030e3cc391f7f3dec1b5a48"
SIXTH_CLEAR_SHA = "147ec31a450535311c9f83fe65361ec99ac3359a1cf6eeb6c4aa963c50966be4"
SIXTH_RESET_SHA = "70fe2f3808fc889ce02fee60156d9357c9df5f9a525fd1a71bc1a72c78e22b50"
NUMERIC_CTOR_SHA = "035a60ed83dc1ceb0893ae3d0ab2949656327f914812efeafe76a7dec6fefd4e"
NUMERIC_INIT_SHA = "ad45d81525c06e107fa271f6b8f51e678f64e81c25dc267dcad5451e4abbb800"
MATH_SQRT_WRAPPER_SHA = "d448df6f0659e171eabf325fdd8d52171a72e1fe13dbea6e14971dc99952d961"
MATH_MULTIPLY_SHA = "20c3dba4cc2eae7fd3cffd8c89650db4fc6958dcadf5381fcb3fb9075068c633"


class OriginalElf:
    def __init__(self, raw):
        if hashlib.sha256(raw).hexdigest() != EXPECTED_SHA:
            raise ValueError("Not the verified Ape Escape 3 USA SCUS_975.01")
        if raw[:7] != b'\x7fELF\x01\x01\x01' or struct.unpack_from('<H',raw,18)[0] != 8:
            raise ValueError("Expected original little-endian MIPS ELF32")
        if struct.unpack_from('<I',raw,24)[0] != 0x100134:
            raise ValueError("Unexpected original program entrypoint")
        self.raw=raw
        phoff=struct.unpack_from('<I',raw,28)[0]
        entsize,count=struct.unpack_from('<HH',raw,42)
        self.segments=[]
        if entsize<32 or phoff+entsize*count>len(raw):raise ValueError('Invalid ELF program headers')
        for i in range(count):
            typ,off,va,_,size,_,flags,_=struct.unpack_from('<8I',raw,phoff+i*entsize)
            if typ==1 and off+size<=len(raw):self.segments.append((va,va+size,off,flags))
        if not self.segments:raise ValueError('Missing original ELF PT_LOAD')
    def read(self,va,size):
        for a,b,off,_ in self.segments:
            if a<=va and va+size<=b:return self.raw[off+va-a:off+va-a+size]
        raise ValueError(f'Original ELF VA not backed by file data: {va:#x}')
    def word(self,va):return struct.unpack('<I',self.read(va,4))[0]

def verify(path):
    elf=OriginalElf(path.read_bytes())
    def sha(start,size):return hashlib.sha256(elf.read(start,size)).hexdigest()
    if sha(0x428ab8,0x58)!=CTOR_BODY_SHA:
        raise ValueError('Original 0x428AB8 constructor dispatch opcode mismatch')
    if sha(0x62e660,0x240)!=CTOR_TABLE_SHA:
        raise ValueError('Original 143-entry static constructor table mismatch')
    if sha(0x41a390,0x1c)!=FIRST_CTOR_SHA or sha(0x41a088,0x304)!=FIRST_CTOR_DEP_SHA:
        raise ValueError('Native original first constructor/12-counter initializer opcode mismatch')
    if sha(0x4173b0,0x8)!=SOURCE_NOOP_SHA:
        raise ValueError('Source-native 0x4173B0 has ceased to be an actual no-op')
    for addr,n,digest in (
        (0x417c58,0x1c,NUMERIC_CTOR_SHA),
        (0x417b50,0x108,NUMERIC_INIT_SHA),
        (0x41d6b8,0x14,MATH_SQRT_WRAPPER_SHA),
        (0x4240d8,0x54,MATH_MULTIPLY_SHA),
    ):
        if sha(addr,n)!=digest:
            raise ValueError(f'Original retail constructor numeric dependency changed at {addr:#x}')
    numeric_edges={
        0x417b84:0x4173b0, 0x417ba8:0x4173b0,
        0x417bcc:0x41d6b8, 0x417bdc:0x41d6b8,
        0x417be8:0x4240d8, 0x417c14:0x41d6b8,
        0x417c24:0x41d6b8, 0x417c30:0x4240d8,
    }
    for site,target in numeric_edges.items():
        op=elf.word(site)
        observed=((site+4)&0xf0000000)|((op&0x3ffffff)<<2)
        if op>>26!=3 or observed!=target:
            raise ValueError(f'Original second constructor call dependency changed at {site:#x}')
    for site,target in {0x417c6c:0x417b50,0x41d6c4:0x41efd0}.items():
        op=elf.word(site)
        observed=((site+4)&0xf0000000)|((op&0x3ffffff)<<2)
        if op>>26!=2 or observed!=target:
            raise ValueError(f'Original second constructor trampoline changed at {site:#x}')
    for va,value in ((0x417bc8,0x24041009),(0x417bd4,0x24040403),
                     (0x417c10,0x24041009),(0x417c1c,0x24040201)):
        if elf.word(va)!=value:
            raise ValueError(f'Original constructor hardcoded float argument changed at {va:#x}')
    if sha(0x3a6b34,0x19c)!=THIRD_CTOR_SOURCE_SHA:
        raise ValueError('Original 0x3A6CD0 static resource writes no longer match verified source')
    if sha(0x720040,0xd0)!=THIRD_CTOR_LITERAL_SHA:
        raise ValueError('Original static literal payload for third constructor differs')
    if elf.word(0x3a6ce4)>>26!=2 or (elf.word(0x3a6ce4)&0x3ffffff)<<2!=0x3a6b34:
        raise ValueError('Original static resource constructor trampoline mismatch')
    if sha(0x39fa10,0x240)!=FOURTH_CTOR_SHA or sha(0x39fc78,0x1c)!=FOURTH_ENTRY_SHA:
        raise ValueError('Original fourth constructor opcode fingerprint changed')
    if sha(0x71f600,0x260)!=FOURTH_LITERAL_SHA:
        raise ValueError('Original 38 native qword source records changed')
    fourth_jump=elf.word(0x39fc8c)
    if fourth_jump>>26!=2 or ((fourth_jump&0x3ffffff)<<2)!=0x39fa10:
        raise ValueError('Fourth constructor original trampoline changed')
    fourth=[elf.word(pc) for pc in range(0x39fa10,0x39fc78,4)]
    if sum(op>>26==30 for op in fourth)!=42 or sum(op>>26==31 for op in fourth)!=41:
        raise ValueError('Original fourth constructor source quadword load/store count changed')
    for va, length, digest in (
        (0x391600, 0x1c, FIFTH_WRAPPER_SHA),
        (0x391148, 0x3c, FIFTH_CALL_SHA),
        (0x38abfc, 0x1c, SIXTH_WRAPPER_SHA),
        (0x38aba0, 0x5c, SIXTH_CLEAR_SHA),
        (0x38aa64, 0x14, SIXTH_RESET_SHA),
    ):
        if sha(va,length)!=digest:
            raise ValueError(f'Original fifth/sixth source instruction mismatch at {va:#x}')
    for site,target,kind in (
        (0x391614,0x391148,2),
        (0x391168,0x39117c,3),
        (0x38ac10,0x38aba0,2),
        (0x38abd4,0x38aa64,3),
    ):
        op=elf.word(site)
        dst=((site+4)&0xf0000000)|((op&0x3ffffff)<<2)
        if op>>26!=kind or dst!=target:
            raise ValueError(f'Original fifth/sixth call/jump changed at {site:#x}')
    if elf.word(0x39117c)!=0x03e00008 or elf.word(0x391180)!=0:
        raise ValueError('Original fifth constructor callee is no longer no-op')
    if elf.word(0x62e660)!=0xffffffff or elf.word(0x62e8a0)!=0:
        raise ValueError('Original constructor table sentinel/boundary mismatch')
    functions=[elf.word(0x62e89c-i*4) for i in range(143)]
    if functions[0]!=0x41a390 or functions[-1]!=0x10526c:
        raise ValueError('Original constructor table boundary addresses changed')
    if any(a<=b or (b & 3) for a,b in zip(functions,functions[1:])):
        raise ValueError('Original constructor dispatch order is invalid')
    edges={0x1002c4:0x1036d8,0x10031c:0x43fe58,0x100324:0x435d60,
           0x100354:0x428b80,0x10036c:0x1037e8,
           0x628218:0x1000a0,0x628220:0x428ab8}
    for site,target in edges.items():
        opcode=elf.word(site)
        observed=((site+4)&0xf0000000)|((opcode&0x3ffffff)<<2)
        if opcode>>26 != 3 or observed!=target:
            raise ValueError(f'Original startup JAL mismatch {site:#x}->{target:#x}')
    opcode=elf.word(0x100374)
    if opcode>>26 != 2 or (opcode & 0x3ffffff)<<2!=0x428cb8:
        raise ValueError('Original startup termination JMP mismatch')
    return {'schema':'ae3/original-windows-bootstrap-elf-audit/v1',
            'usa_elf_sha256':EXPECTED_SHA,'original_entry':'0x100134',
            'source_constructor_loop':'0x428ab8','ctor_table_first':'0x62e89c',
            'ctor_sentinel':'0x62e660','native_constructor_count':len(functions),
            'first_constructor':f'{functions[0]:#x}',
            'last_constructor':f'{functions[-1]:#x}',
            'verified_startup_calls':len(edges),
            'native_first_original_constructor':"0x41a390",
            'first_original_constructor_global_counters':12,
            'native_second_original_constructor':"0x417c58",
            'original_numeric_call_edges':len(numeric_edges)+2,
            'original_numeric_source_inputs':[10,16,8],
            'third_constructor_source_literals':13,
            'third_constructor_source_type_words':[1,1,1,2,1,1,4,3,2],
            'third_constructor_source_data_required':True,
            'fourth_constructor_source_data_required':True,
            'fourth_constructor_native_qwords':38,
            'original_fifth_native_noop':"0x391600",
            'original_sixth_native_reset_records':2,
            'original_sixth_preserved_padding_offsets':[5,6,7],
            'next_unrecovered_original_constructor':"0x38aa18",
            'original_callbacks_not_implemented':True,
            'original_windows_gameplay_booted':False}

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('private_original_elf',type=Path)
    parser.add_argument('--out',type=Path)
    args=parser.parse_args()
    report=verify(args.private_original_elf)
    result=json.dumps(report,indent=2)+'\n'
    if args.out:args.out.write_text(result,encoding='utf8')
    else:print(result,end='')
    print('ORIGINAL_WINDOWS_STARTUP_ELF_VERIFIED',report['native_constructor_count'])
if __name__=='__main__':main()
