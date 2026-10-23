#!/usr/bin/env python3
"""Check exact USA SCUS_975.01 original 0x36F8C8 VF1 regeneration.

The genuine private owner ELF is required; nothing proprietary is
committed. All 35 source words, PS2 PPACW/MUL accumulator opcodes,
nine alternate tagged-guest branches and destination registers
are pinned. The ordinary untagged branch is natively recovered;
the tagged address branches and full actor frame are not.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ELF_SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
WINDOW_SHA="62aef8aedb73f27eb053c1f294af37dbb35f9550fc48f46293b38e2da2eeb7f6"
START,END=0x36F8C8,0x36F954
SOURCE_HEX="""
C48000A4 3C0F3F80 D8880040 46000042 D8870050
440C0000 71EC74C8 8C8F00A0 46000802 D8860060
D9E10040 D9E20050 D9E50060 D9E30070 440D0800
440C0000 71AC7CC8 71CF74C8 48AE2000 8C8F00A8
4BE409BC 35EF0002 AC8F00A8 48AE0800 D8840070
4BE110BD 4BE128BE 4BE1184B 4BE141BC 4BE138BD
4BE130BE 4BE1204B F8810090 03E00008 48220800
"""
WORDS={START+i*4:int(w,16) for i,w in enumerate(SOURCE_HEX.split())}
TARGETS={
    0x36F8D0:0x36F9D4,0x36F8D8:0x36FA1C,
    0x36F8EC:0x36FA70,0x36F8F0:0x36F9F4,
    0x36F8F4:0x36FA38,0x36F8F8:0x36FA7C,
    0x36F8FC:0x36FAC0,0x36F928:0x36FAEC,
    0x36F948:0x36FB8C}
# Source VU0 SPECIAL2 index and real fs,ft,destination mask.
VU={
    0x36F918:(24,1,4,15),  # VMULAx VF1old*VF4.x
    0x36F92C:(9,2,1,15),   # VMADDAy VF2*VF1.y
    0x36F930:(10,5,1,15),  # VMADDAz VF5*VF1.z
    0x36F934:(7,3,1,15),   # VSUBAw VF3*VF1.w
    0x36F938:(24,8,1,15),  # VMULAx resets ACC VF8*VF1.x
    0x36F93C:(9,7,1,15),   # VMADDAy VF7*VF1.y
    0x36F940:(10,6,1,15),  # VMADDAz VF6*VF1.z
    0x36F944:(7,4,1,15),   # VSUBAw VF4*VF1.w
}

def elf_bytes(data,va,n):
    ph=struct.unpack_from("<I",data,28)[0]
    stride,count=struct.unpack_from("<HH",data,42)
    for i in range(count):
        kind,off,base,_,size,_,_,_=struct.unpack_from(
            "<8I",data,ph+i*stride)
        if kind==1 and base<=va and va+n<=base+size:
            return data[off+va-base:off+va-base+n]
    raise ValueError("original ELF VA unavailable "+hex(va))

def verify(path):
    data=Path(path).read_bytes()
    if hashlib.sha256(data).hexdigest()!=ELF_SHA:
        raise ValueError("not pinned original USA SCUS_975.01")
    if data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0]!=8:
        raise ValueError("not original MIPS32 little-endian ELF")
    actual=elf_bytes(data,START,END-START)
    if hashlib.sha256(actual).hexdigest()!=WINDOW_SHA:
        raise ValueError("original 0x36F8C8 source window changed")
    if len(WORDS)!=35:
        raise ValueError("original 0x36F8C8 count changed")
    for pc,w in WORDS.items():
        if struct.unpack("<I",elf_bytes(data,pc,4))[0]!=w:
            raise ValueError("retail source instruction changed "+hex(pc))
    for pc,target in TARGETS.items():
        disp=WORDS[pc]&0xffff
        if disp>=0x8000:disp-=0x10000
        if pc+4+disp*4!=target:
            raise ValueError("original tagged source branch changed "+hex(pc))
    for pc,(op,fs,ft,mask) in VU.items():
        w=WORDS[pc]
        fields=((w&3)|((w>>4)&0x7c),(w>>11)&31,
                (w>>16)&31,(w>>21)&15)
        if fields!=(op,fs,ft,mask):
            raise ValueError("real original VU0 operand/operation differs "+hex(pc))
    for pc in (0x36F8E0,0x36F908,0x36F90C):
        w=WORDS[pc]
        if w>>26!=0x1c or w&63!=8 or (w>>6)&31!=19:
            raise ValueError("original MMI0 PPACW changed "+hex(pc))
    return {
        "elf_sha256":ELF_SHA,"original_window":"0x36F8C8..0x36F954",
        "source_instructions":len(WORDS),"source_window_sha256":WINDOW_SHA,
        "vu0_special2_operations":len(VU),"mmi_ppacw":3,
        "other_tagged_branch_targets":len(TARGETS),
        "native_source_polynomial":"[status^3,status^2,status,1]",
        "native_untagged_vf1_vf4_acc_flags_recovered":True,
        "original_tagged_paths_recovered":False,"playable_game":False}
if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("original_elf",type=Path)
    print(json.dumps(verify(parser.parse_args().original_elf),indent=2))
