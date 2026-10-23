#!/usr/bin/env python3
"""Audit source-original Ape Escape 3 USA nonterminal movement metric.

Pins original 0x317B34 complete 26-word function, genuine
0x36FB18 early-return gate (17 words), and the original
0x557F30..0x557F50 call prefix (9 words). Only source
opcode-verified paths are implemented: VU0/VF1 squared norm,
SQRT.S/ADD.S/MIN.S for exact ordinary operands, and two
36FB18 non-collision returns. The larger 36FB5C+/557F54+
gameplay remains unrecovered. Original ELF stays private.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

EXE_SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
SOURCE_WINDOWS={
 (0x317B34,0x317B9C):"66e99b9cf7fadebdbb0a4befc31cdfce5aeefa3beba5c08787351c2ea9b09b4c",
 (0x36FB18,0x36FB5C):"053051e501659048bcc73ce6ad95247f21f761436775fe59382283661eb506e3",
 (0x557F30,0x557F54):"1d6be51ae1fb85db3ac8f83bec9031bd1c7115621ca00b550263d984d8d41d71",
}
WORDS={
 0x317B40:0x15CF0003, # BNE parent+0x40 !=1
 0x317B44:0x30A500FF, # mode masked in branch delay
 0x317B50:0x10A00010, # mode zero -> source +0x3B4
 0x317B58:0xD8810060, # EE BBit032 tagged parent branch
 0x317B5C:0xC5A20364, # original resource table f2 offset
 0x317B60:0x4BE1086A, # VMUL VF1.xyzw,VF1,VF1
 0x317B64:0xC5A10368, # original resource f1 cap
 0x317B68:0x4BA10841, # VADDy.xyw VF1,VF1,VF1
 0x317B6C:0x4BE10842, # VADDz.xyzw VF1,VF1,VF1
 0x317B70:0x482F0800, # DMFC2 VF1.x to GPR
 0x317B78:0x448F0000, # MTC1 source f0
 0x317B84:0x46000004, # SQRT.S f0,f0
 0x317B88:0x46020000, # ADD.S f0,f0,f2
 0x317B8C:0x03E00008, # JR RA
 0x317B90:0x46010029, # MIN.S f0,f0,f1 in return DELAY
 0x317B94:0x03E00008, # source zero-mode fast JR RA
 0x317B98:0xC5A003B4, # branch-delay LWC1 source float
 0x36FB24:0x25CE57C0, # source original constant 0x7157C0
 0x36FB3C:0xC48100A4, # original child+0xB4 status
 0x36FB40:0x46010036, # C.OLE.S threshold <= status
 0x36FB44:0x4501002C, # BC1T to 0x36FBF8
 0x36FB48:0x46006006, # MOVE f0=f12 in delay slot
 0x36FB4C:0x44800000, # MTC1 +0.0 into f0
 0x36FB50:0x46140034, # C.OLT.S +0 < original f20=f12
 0x36FB54:0x45020029, # BC1FL to 0x36FBFC
 0x36FB58:0xDFB00000, # delayed restore only on branch
 0x557F30:0x92050009, # source child mode+0x09
 0x557F34:0x0220202D, # genuine original parent
 0x557F38:0x0C0C5ECD, # JAL 0x317B34
 0x557F3C:0x26120010, # delay child+0x10 source address
 0x557F40:0xC62C0070, # LWC1 f12 original parent+0x70
 0x557F48:0x46000506, # source f20=f0
 0x557F4C:0x0C0DBEC6, # JAL 0x36FB18
 0x557F50:0x460C0302, # delay MUL.S f12=f0*f12
}
def original_bytes(data,va,size):
    phoff=struct.unpack_from("<I",data,28)[0]
    stride,n=struct.unpack_from("<HH",data,42)
    for i in range(n):
        typ,offset,base,_,file_len,_,_,_=struct.unpack_from(
            "<8I",data,phoff+i*stride)
        if typ==1 and base<=va and va+size<=base+file_len:
            return data[offset+va-base:offset+va-base+size]
    raise ValueError("source original retail VA not ELF-backed "+hex(va))

def verify(original_path):
    data=Path(original_path).read_bytes()
    if hashlib.sha256(data).hexdigest()!=EXE_SHA:
        raise ValueError("not pinned Ape Escape 3 USA SCUS_975.01")
    if data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0]!=8:
        raise ValueError("not original MIPS little-endian ELF32")
    for (start,end),digest in SOURCE_WINDOWS.items():
        actual=hashlib.sha256(original_bytes(data,start,end-start)).hexdigest()
        if actual!=digest:
            raise ValueError("original complete source window changed "+hex(start))
    for at,expected in WORDS.items():
        actual=struct.unpack("<I",original_bytes(data,at,4))[0]
        if actual!=expected:
            raise ValueError("original source instruction changed "+hex(at))

    branches={0x317B40:0x317B50,0x317B50:0x317B94,
              0x36FB44:0x36FBF8,0x36FB54:0x36FBFC}
    for at,target in branches.items():
        disp=WORDS[at]&0xffff
        if disp>=0x8000:disp-=65536
        if at+4+disp*4!=target:
            raise ValueError("source retail branch target changed "+hex(at))

    for at,target in {0x557F38:0x317B34,0x557F4C:0x36FB18}.items():
        word=WORDS[at]
        actual=((at+4)&0xf0000000)|((word&0x03ffffff)<<2)
        if (word>>26)!=3 or actual!=target:
            raise ValueError("source JAL/callee changed "+hex(at))
    if struct.unpack("<I",original_bytes(data,0x7157C0,4))[0]!=0x3F800000:
        raise ValueError("source threshold original 0x7157C0 not 1.0f")
    return {
      "source_elf_sha256":EXE_SHA,
      "complete_windows_verified":len(SOURCE_WINDOWS),
      "independently_pinned_opcode_sites":len(WORDS),
      "real_vu0_VMUL_VADDy_VADDz":True,
      "real_COP1_SQRT_ADD_MIN":True,
      "original_parent_scale_float_offset":"+0x70",
      "original_movement_status_float_offset":"child+0xB4",
      "unrecovered":"36FB5C+ positive movement collision/animation and 557F54+",
      "exact_ordinary_arithmetic_only":True,
      "native_game_playable":False}
if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("original_elf",type=Path)
    print(json.dumps(verify(p.parse_args().original_elf),indent=2))
