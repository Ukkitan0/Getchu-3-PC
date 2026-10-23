#!/usr/bin/env python3
"""Verify retail 0x557FC8..0x558030 post-movement source control path.

Original USA SCUS_975.01 is private and NEVER committed.
Pins 26 original code words and three real 128-bit MMI3 POR
register moves. Confirms genuine 30C0A0/276538/36FC28
JALs, original debug gate and alternate 558084 bit0 branch.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ELF_SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
START,END=0x557FC8,0x558030
WINDOW_SHA="aecf9edaedeeea8a27a5987bed707c47e9210d57ca411f95bfd1503fec0df6b1"
HEX="""
8E0F00B8 31EF0001 31EF00FF 11E0002B 00000000
7A020090 70022CA9 0C0C3028 0220202D 0040A82D
0280202D 0C09D94E 70132CA9 3C0F0063 91EF54E7
11E00004 0000702D 3C0F0063 91EF54E0 2DEE0001
31CF00FF 11E00004 3C0D0070 0C0DBF0A 0240202D
3C0D0070
"""
WORDS={START+4*i:int(h,16) for i,h in enumerate(HEX.split())}
EXTRA={
    0x558084:0x0C0DBE0F, # real 36F83C JAL
    0x558088:0x0240202D, # JAL delay original child scratch
    0x55808C:0x1000FFD5, # branch to 557FE4 after flag0
    0x558090:0x70022CA9, # branch delay original POR a1,v0
}
def read(data,va,n):
    ph=struct.unpack_from("<I",data,28)[0]
    stride,count=struct.unpack_from("<HH",data,42)
    for i in range(count):
        kind,off,base,_,length,_,_,_=struct.unpack_from(
            "<8I",data,ph+i*stride)
        if kind==1 and base<=va and va+n<=base+length:
            return data[off+va-base:off+va-base+n]
    raise ValueError("original retail VA not in source ELF "+hex(va))
def verify(path):
    data=Path(path).read_bytes()
    if hashlib.sha256(data).hexdigest()!=ELF_SHA:
        raise ValueError("incorrect original USA SCUS_975.01")
    if data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0]!=8:
        raise ValueError("source is not little-endian MIPS32 ELF")
    if len(WORDS)!=26 or hashlib.sha256(read(data,START,END-START)).hexdigest()!=WINDOW_SHA:
        raise ValueError("retail 557FC8 original window does not match")
    for at,opcode in {**WORDS,**EXTRA}.items():
        if struct.unpack("<I",read(data,at,4))[0]!=opcode:
            raise ValueError("retail post-tail source instruction mismatch "+hex(at))
    for at,target in {
        0x557FD4:0x558084, 0x558004:0x558018,
        0x55801C:0x558030, 0x55808C:0x557FE4,
    }.items():
        w={**WORDS,**EXTRA}[at]
        disp=w&65535
        if disp>=32768:disp-=65536
        if at+4+(disp<<2)!=target:
            raise ValueError("original branch target changed "+hex(at))
    for at,target in {
        0x557FE4:0x30C0A0, 0x557FF4:0x276538,
        0x558024:0x36FC28, 0x558084:0x36F83C,
    }.items():
        w={**WORDS,**EXTRA}[at]
        if w>>26!=3 or (((at+4)&0xf0000000)|((w&0x03ffffff)<<2))!=target:
            raise ValueError("actual original JAL destination changed "+hex(at))
    for at,rt in ((0x557FE0,2),(0x557FF8,19),(0x558090,2)):
        w={**WORDS,**EXTRA}[at]
        # MMI function0x29=M MI3; index18=POR.
        if not (w>>26==0x1c and w&63==0x29 and (w>>6)&31==18
                and (w>>21)&31==0 and (w>>16)&31==rt
                and (w>>11)&31==5):
            raise ValueError("retail 128-bit POR original argument differs "+hex(at))
    return {
        "original_elf_sha256":ELF_SHA,
        "source_window":"0x557FC8..0x558030",
        "full_window_sha256":WINDOW_SHA,
        "original_window_instruction_words":len(WORDS),
        "original_direct_jals":["0x36F83C","0x30C0A0","0x276538","0x36FC28?"],
        "verified_ee_por_128bit_sites":3,
        "source_diag":"read 0x6354E0 only if 0x6354E7 is nonzero",
        "native_post_movement_gameplay_requires_actual_owner_methods":True,
        "complete_playable_game":False}
if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("original_elf",type=Path)
    print(json.dumps(verify(p.parse_args().original_elf),indent=2))
