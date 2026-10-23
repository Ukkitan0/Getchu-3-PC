#!/usr/bin/env python3
"""Audit original USA 0x36F5F4 movement VU0 routine and its REAL callsite.

Pins the full local entry window, genuine VU0 vector reduction
opcodes, scalar data constants, return stores and five cross-entry
EE BBit132 branches. The verified caller at 0x57C148 supplies
source stack+0x20, t0=0 and three original COP2 vectors. The two
VF1.x/VF2.x squared-distance reductions are natively translated
for ordinary FP inputs; the rest of the VU register/flag pipeline
and alternative entry paths are NOT recovered.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

RETAIL_SHA256 = "5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
ENTRY = 0x36F5F4
RETURN_END = 0x36F6DC
WINDOW_SHA256 = "1a3f90814dc34de36195e7d7718d1b0d906e365020117633f9f8967218df3de8"
CALLSITE_SHA256 = "6ce5da0f7c29c442276734d4ef87501b483e7c3f242c3cbf61fe721022b81112"

# Source 0x36F608..0x36F62C is the genuine COP2 upper-pipeline
# vector length-squared reduction, not a guessed 3D distance:
# VSUB -> VMUL -> VADDy.xyw -> VADDz.xyzw for BOTH source pairs.
VU_MACRO_SHA256 = "b24dbd9ee6c08d78397e803f4e25b1591b95f2b735215ae7f115e66dffed1840"
VU_SCALAR_OPS = {
    # pc: (word, fd, fs, ft, destination_mask, low6_opcode)
    0x36F608:(0x4BE3206C,1,4,3,0xF,0x2C), # VSUB VF1=VF4-VF3
    0x36F610:(0x4BE330AC,2,6,3,0xF,0x2C), # VSUB VF2=VF6-VF3
    0x36F618:(0x4BE1086A,1,1,1,0xF,0x2A), # VMUL VF1*=VF1
    0x36F61C:(0x4BE210AA,2,2,2,0xF,0x2A), # VMUL VF2*=VF2
    0x36F620:(0x4BA10841,1,1,1,0xD,0x01), # VADDy VF1.xyw
    0x36F624:(0x4BA21081,2,2,2,0xD,0x01), # VADDy VF2.xyw
    0x36F628:(0x4BE10842,1,1,1,0xF,0x02), # VADDz VF1.xyzw
    0x36F62C:(0x4BE21082,2,2,2,0xF,0x02), # VADDz VF2.xyzw
}

# Genuine MIPS words, read from owner-private SCUS_975.01, not examples.
WORDS = {
    0x36F5F4:0x48A51800, # a1 -> COP2
    0x36F5FC:0x48A62000, # a2 -> COP2
    0x36F604:0x48A73000, # a3 -> COP2
    0x36F614:0x48A72800, # reuse original a3 -> VF5
    0x36F638:0x448F0800, # MTC1 first VF1.x -> COP1 f1
    0x36F60C:0xC5C00000, # LWC1 f0, original +0x7157AC
    0x36F630:0x482E0800, # DMFC2 VF1.x => COP1 first norm
    0x36F63C:0x482E1000, # DMFC2 VF2.x => later COP1 f1

    0x36F640:0x46000834, # C.OLT.S f1,f0
    0x36F648:0x45000005, # conditional original normalisation branch
    0x36F660:0x0000682D, # t5 = 0
    0x36F64C:0x3C0E3F00, # LUI source scalar 0.5f in t6
    0x36F650:0x48AE0800, # QMTC2 t6 -> VF1
    0x36F654:0x4BE11898, # VMULx VF2=VF3*VF1.x
    0x36F658:0x4BE13058, # VMULx VF1=VF6*VF1.x
    0x36F65C:0x4BE11128, # VADD.xyzw VF4=VF2+VF1
    0x36F664:0x448F0800, # MTC1 second VU scalar -> f1
    0x36F668:0x71AD74C8, # PPACW zero t5 -> t6
    0x36F66C:0x71CE74C8, # PPACW t6 -> t6, remains zero
    0x36F674:0x48AE3800, # QMTC2 zeroed t6 -> VF7
    0x36F67C:0x4BE320EC, # VSUB VF3=VF4-VF3
    0x36F684:0x4BE4286C, # VSUB VF1=VF5-VF4
    0x36F68C:0x4BE538AC, # VSUB VF2=VF7-VF5
    0x36F678:0x240B0002, # t3 = 2
    0x36F680:0x0008580A, # MOVZ t3,zero,t0 => 0 at retail caller
    0x36F690:0x000B61C0, # t4 = t3 << 7 (128-byte source records)
    0x36F694:0x25EF4DD0, # +0x4DD0 to 0x740000 -> 0x744DD0
    0x36F698:0x46000834, # C.OLT.S f1,1.0f, unordered=false
    0x36F6A0:0xF8840010, # BBit132 a0 bit36 -> source 0x36F6E4
    0x36F6A4:0xF8860020, # BBit132 a0 bit38 -> source 0x36F728
    0x36F6A8:0xAC8000A8, # SW zero, [source a0]+0xA8
    0x36F6AC:0xAC8D00A4, # SW t5(=0), [a0]+0xA4
    0x36F6B0:0xAC8C00A0, # SW t4(source guest VA), [a0]+0xA0
    0x36F6B4:0xF8830040, # BBit132 a0 bit35 -> source 0x36F7B8
    0x36F6B8:0xF8810050, # BBit132 a0 bit33 -> source 0x36F7FC
    0x36F6BC:0xF8820060, # BBit132 a0 bit34 -> source 0x36F840
    0x36F6C4:0xAC8B00AC, # SW t3(=0 at caller), [a0]+0xAC
    0x36F6C8:0x45000002, # BC1F skip optional SWC1
    0x36F6D0:0xE48000A4, # SWC1 f0(=1.0f), [a0]+0xA4
    0x36F6D4:0x03E00008, # return
    0x57C120:0x27B00020, # s0 = original SP+0x20
    0x57C138:0x0200202D, # a0 = s0 after two 0x441F40 no-ops
    0x57C13C:0x0000402D, # t0 = zero
    0x57C148:0x0C0DBD7D, # JAL source 0x36F5F4
    0x57C14C:0x70133CA9, # source packing in JAL delay slot
}
GUEST_RECORD_TABLE = 0x744DD0
SOURCE_CONSTANT_1 = 0x7157AC
BRANCH_EXITS = {
    0x36F6A0:0x36F6E4,
    0x36F6A4:0x36F728,
    0x36F6B4:0x36F7B8,
    0x36F6B8:0x36F7FC,
    0x36F6BC:0x36F840,
}

def segments(data):
    phoff=struct.unpack_from("<I",data,28)[0]
    entsize,n=struct.unpack_from("<HH",data,42)
    return [struct.unpack_from("<8I",data,phoff+i*entsize)
            for i in range(n)]

def original_bytes(data,va,count):
    for typ,off,start,_,filesz,_,_,_ in segments(data):
        if typ==1 and start<=va and va+count<=start+filesz:
            return data[off+va-start:off+va-start+count]
    raise ValueError("source ELF VA is not file-backed "+hex(va))

def verify(original):
    data=Path(original).read_bytes()
    if hashlib.sha256(data).hexdigest()!=RETAIL_SHA256:
        raise ValueError("not pinned original USA retail executable")
    if data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0]!=8:
        raise ValueError("not original little-endian MIPS ELF32")
    code=original_bytes(data,ENTRY,RETURN_END-ENTRY)
    if hashlib.sha256(code).hexdigest()!=WINDOW_SHA256:
        raise ValueError("original 0x36F5F4 entry window changed")
    if hashlib.sha256(original_bytes(data,0x57C120,0x44)).hexdigest()!=CALLSITE_SHA256:
        raise ValueError("original 0x57C148 callsite operands changed")
    # The source opcode fields prove the lane/mask/operand sequence.
    if hashlib.sha256(original_bytes(data,0x36F608,0x28)).hexdigest()!=VU_MACRO_SHA256:
        raise ValueError("source original 36F608 VU0 scalar sequence changed")
    for pc,(word,fd,fs,ft,mask,op) in VU_SCALAR_OPS.items():
        actual=struct.unpack("<I",original_bytes(data,pc,4))[0]
        decoded=((actual>>6)&31,(actual>>11)&31,
                 (actual>>16)&31,(actual>>21)&15,actual&63)
        if actual!=word or decoded!=(fd,fs,ft,mask,op):
            raise ValueError("source VU0 scalar operation or operand changed "+hex(pc))
    for pc,expected in WORDS.items():
        (actual,)=struct.unpack("<I",original_bytes(data,pc,4))
        if actual!=expected:
            raise ValueError("retail 0x36F5F4 source opcode changed "+hex(pc))
    if struct.unpack("<I",original_bytes(data,SOURCE_CONSTANT_1,4))[0]!=0x3F800000:
        raise ValueError("source 0x7157AC is not 1.0f")
    guest_bss=False
    for typ,off,start,_,filesz,memsz,_,_ in segments(data):
        if typ==1 and start+filesz<=GUEST_RECORD_TABLE<start+memsz:
            guest_bss=True
    if not guest_bss:
        raise ValueError("0x744DD0 must remain original runtime BSS, not file data")
    for at,target in BRANCH_EXITS.items():
        originalWord=WORDS[at]
        # All five are original MIPS64 BBit132, which examines
        # a0 high-32 bits (bit >=32), not host pointer bits.
        offset=originalWord&0xffff
        if offset>=0x8000:offset-=0x10000
        actual=at+4+(offset<<2)
        if actual!=target:
            raise ValueError("source BBit132 target changed "+hex(at))

    return {
        "retail_elf_sha256":RETAIL_SHA256,
        "source_entry":"0x36F5F4",
        "entry_to_first_return":"0x36F5F4..0x36F6DC",
        "entry_window_words":len(code)//4,
        "entry_window_sha256":WINDOW_SHA256,
        "callsite":"0x57C148",
        "callsite_t0":0,
        "source_guest_bss_record_pointer":"0x744DD0",
        "source_record_stride":128,
        "direct_tail_stores":["+0xA8=0","+0xA4=0 or 1.0f",
                              "+0xA0=0x744DD0","+0xAC=0"],
        "cross_entry_source_branches":{
            hex(k):hex(v) for k,v in BRANCH_EXITS.items()},
        "native_source_vu0_xyz_reduction_recovered":True,
        "native_source_vu0_reduction_input_policy":"finite normal/zero lanes only",
        "original_vu0_scalar_opcodes_verified":len(VU_SCALAR_OPS),
        "original_vu0_followup_vector_opcode_sites":12,
        "native_source_vu0_vector_midpoint_branch_recovered":True,
        "native_tail_only_recovered":False,
        "full_original_vu0_arithmetic_recovered":False,
        "playable_windows_game":False,
    }

if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("original_elf",type=Path)
    print(json.dumps(verify(p.parse_args().original_elf),indent=2))
