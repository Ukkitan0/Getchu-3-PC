#!/usr/bin/env python3
"""Verify complete original USA 36F83C bit0 VF1/VF4 regeneration.

The original private SCUS_975.01 ELF (not committed) is required.
All 35 source words, 8 exact shared VU0 instructions, 3 PPACW
words, original bit0 ORI and 9 alternate tag branches are pinned.
Only the untagged ordinary exact source branch is translated.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ELF_SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
START,END=0x36F83C,0x36F8C8
WINDOW_SHA="83dfb2648a7f6a120429ba3bf25d508ca4540b06a6cb98363d2130203934e399"
HEX="""
C48000A4 3C0F3F80 D8880000 46000042 D8870010
440C0000 71EC74C8 8C8F00A0 46000802 D8860020
D9E10000 D9E20010 D9E50020 D9E30030 440D0800
440C0000 71AC7CC8 71CF74C8 48AE2000 8C8F00A8
4BE409BC 35EF0001 AC8F00A8 48AE0800 D8840030
4BE110BD 4BE128BE 4BE1184B 4BE141BC 4BE138BD
4BE130BE 4BE1204B F8810080 03E00008 48220800
"""
WORDS={START+4*i:int(x,16) for i,x in enumerate(HEX.split())}
VU_PAIR=(
 (0x36F88C,0x36F918),(0x36F8A0,0x36F92C),
 (0x36F8A4,0x36F930),(0x36F8A8,0x36F934),
 (0x36F8AC,0x36F938),(0x36F8B0,0x36F93C),
 (0x36F8B4,0x36F940),(0x36F8B8,0x36F944),
)
PPACW_PAIR=((0x36F854,0x36F8E0),
            (0x36F87C,0x36F908),
            (0x36F880,0x36F90C))
TAG_BRANCHES={
    0x36F844:0x36F848,0x36F84C:0x36F890,
    0x36F860:0x36F8E4,0x36F864:0x36F868,
    0x36F868:0x36F8AC,0x36F86C:0x36F8F0,
    0x36F870:0x36F934,0x36F89C:0x36F960,
    0x36F8BC:0x36FAC0,
}
def read(data,va,length):
    phoff=struct.unpack_from("<I",data,28)[0]
    stride,num=struct.unpack_from("<HH",data,42)
    for i in range(num):
        typ,off,base,_,size,_,_,_=struct.unpack_from(
            "<8I",data,phoff+i*stride)
        if typ==1 and base<=va and va+length<=base+size:
            return data[off+va-base:off+va-base+length]
    raise ValueError("source VA not original file-backed "+hex(va))
def word(data,pc):return struct.unpack("<I",read(data,pc,4))[0]
def verify(path):
    data=Path(path).read_bytes()
    if hashlib.sha256(data).hexdigest()!=ELF_SHA:
        raise ValueError("wrong USA SCUS_975.01 original")
    if data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0]!=8:
        raise ValueError("not original MIPS32 little-endian ELF")
    if len(WORDS)!=35 or hashlib.sha256(read(data,START,END-START)).hexdigest()!=WINDOW_SHA:
        raise ValueError("36F83C original 35-word source fingerprint changed")
    for pc,opcode in WORDS.items():
        if word(data,pc)!=opcode:
            raise ValueError("source original 36F83C opcode changed "+hex(pc))
    for first,second in VU_PAIR+PPACW_PAIR:
        if word(data,first)!=word(data,second):
            raise ValueError("original 36F83C/36F8C8 source arithmetic differed "+hex(first))
    for pc,target in TAG_BRANCHES.items():
        disp=WORDS[pc]&65535
        if disp>=32768:disp-=65536
        if pc+4+(disp<<2)!=target:
            raise ValueError("original source tagged branch target altered "+hex(pc))
    if word(data,0x36F890)!=0x35EF0001:
        raise ValueError("original flag0 ORI changed")
    if word(data,0x36F894)!=0xAC8F00A8:
        raise ValueError("original flag0 store changed")
    if word(data,0x36F8C4)!=0x48220800:
        raise ValueError("original returned VF1 changed")
    return {
        "retail_elf_sha256":ELF_SHA,
        "source_window":"0x36F83C..0x36F8C8",
        "source_original_words":35,
        "source_sha256":WINDOW_SHA,
        "identical_vu0_accumulator_instructions":len(VU_PAIR),
        "identical_packed_word_ops":len(PPACW_PAIR),
        "tagged_address_branch_targets_verified":len(TAG_BRANCHES),
        "source_original_flag":"scratch +0xA8 ORI 1",
        "original_36f8c8_sibling_differs":"scratch +0xA8 ORI 2",
        "native_full_actor_gameplay":False,
        "playable_windows_port":False}
if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("original_elf",type=Path)
    print(json.dumps(verify(parser.parse_args().original_elf),indent=2))
