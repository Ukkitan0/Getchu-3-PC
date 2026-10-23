#!/usr/bin/env python3
"""Pin source original USA 0x36FB5C positive flagged movement path.

Genuine original SCUS_975.01 SHA256 required. This verifier
checks original VU0 and COP1 instructions, branch targets,
status/flag stores and the constant +0x7157C4=0.125f.
Recovery handles only the bit1-set, non-tagged pointer and
exact finite binary32 subset. If bit1 is clear, actual 0x36F8C8 still MUST regenerate VF1;
ordinary untagged regeneration is now recovered. Tagged and repeated
movement/collision paths remain incomplete; no stand-in gameplay.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ELF_SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
WINDOW_START=0x36FB5C
WINDOW_END=0x36FC28
WINDOW_SHA="eba23d30f1f7b8186abebf22a802722e6835be500a5286906420fb02c943c457"
OPCODES={
    0x36FB5C:0x8E0F00A8, # original a0+0xA8 flags
    0x36FB60:0x31EF0002, # flag bit 1
    0x36FB64:0x11E0002C, # if no source bit1: 36FC18
    0x36FB6C:0xDA010090, # BBit032 original child bit1
    0x36FB70:0x4BE1086A, # VMUL.xyzw VF1^2
    0x36FB7C:0xC5E10000, # LWC1 0.125 from 7157C4
    0x36FB80:0x4BA10841, # VADDy.xyw
    0x36FB84:0x4BE10842, # VADDz.xyzw
    0x36FB88:0x482E0800, # DMFC2 VF1.x
    0x36FB90:0x448F0000, # MTC1 f0 squared norm
    0x36FB9C:0x460000C4, # SQRT.S f3,f0
    0x36FBA0:0x46011882, # MUL.S f2=f3*f1
    0x36FBA4:0x4602A036, # C.OLE.S f20,f2
    0x36FBA8:0x45000018, # BC1F to 36FC0C
    0x36FBAC:0xC60000A4, # delay load original status
    0x36FBB8:0x4603A043, # DIV.S f1=f20/f3
    0x36FBBC:0x46010000, # ADD.S f0=f0+f1
    0x36FBC0:0x4480A000, # MTC1 zero,f20
    0x36FBD0:0x46000836, # C.OLE.S threshold1,status
    0x36FBD4:0x45030004, # BC1TL taken if complete
    0x36FBD8:0x46010001, # branch-likely SUB.S status,1
    0x36FBDC:0xE60000A4, # SWC1 next status
    0x36FBE0:0x1000FFDA, # return to 36FB4C with f20 updated
    0x36FBE4:0xAE0000A8, # delay clear original flags
    0x36FBE8:0xAE0000A8, # clear flags completed
    0x36FBEC:0xE60100A4, # store 1.0f status
    0x36FBF0:0x46030002, # return overflow length
    0x36FBF4:0x4600A000, # return remaining + overflow
    0x36FC0C:0x4602A501, # SUB.S remaining f20-f2
    0x36FC10:0x1000FFEC, # loop threshold compare 36FBC4
    0x36FC14:0x46010000, # delay status+0.125
    0x36FC18:0x0C0DBE32, # JAL original 36F8C8
    0x36FC1C:0x0200202D, # genuine child guest addr
    0x36FC20:0x1000FFD3, # back to 36FB70 after VF1 regen
    0x36FC24:0x48A20800, # delay load regenerated VF1
}
def read(data,addr,length):
    phoff=struct.unpack_from("<I",data,28)[0]
    stride,n=struct.unpack_from("<HH",data,42)
    for j in range(n):
        typ,off,start,_,size,_,_,_=struct.unpack_from(
            "<8I",data,phoff+j*stride)
        if typ==1 and start<=addr and addr+length<=start+size:
            return data[off+addr-start:off+addr-start+length]
    raise ValueError("source VA not original file-backed "+hex(addr))
def verify(path):
    data=Path(path).read_bytes()
    if hashlib.sha256(data).hexdigest()!=ELF_SHA:
        raise ValueError("not pinned Ape Escape 3 USA SCUS_975.01")
    if data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0]!=8:
        raise ValueError("source not little-endian MIPS ELF32")
    if hashlib.sha256(read(data,WINDOW_START,WINDOW_END-WINDOW_START)).hexdigest()!=WINDOW_SHA:
        raise ValueError("original flagged movement source window changed")
    for at,word in OPCODES.items():
        if struct.unpack("<I",read(data,at,4))[0]!=word:
            raise ValueError("source instruction mismatch "+hex(at))
    if struct.unpack("<I",read(data,0x7157C4,4))[0]!=0x3E000000:
        raise ValueError("retail movement coefficient not 0.125f")
    targets={
        0x36FB64:0x36FC18,
        0x36FBA8:0x36FC0C,
        0x36FBD4:0x36FBE8,
        0x36FBE0:0x36FB4C,
        0x36FC10:0x36FBC4,
        0x36FC20:0x36FB70,
    }
    for at,target in targets.items():
        w=OPCODES[at];disp=w&0xffff
        if disp>=0x8000:disp-=0x10000
        if at+4+(disp<<2)!=target:
            raise ValueError("original branch target mismatch "+hex(at))
    w=OPCODES[0x36FC18]
    if w>>26!=3 or ((0x36FC1C&0xf0000000)|((w&0x3ffffff)<<2))!=0x36F8C8:
        raise ValueError("retail original source 36F8C8 dependency differs")
    return {
      "original_executable_sha256":ELF_SHA,
      "original_window":"0x36FB5C..0x36FC28",
      "source_instruction_count":(WINDOW_END-WINDOW_START)//4,
      "window_sha256":WINDOW_SHA,
      "pinned_individual_instructions":len(OPCODES),
      "original_source_coefficient":0.125,
      "source_completed_flagged_paths_native":True,
      "requires_original_36f8c8_if_regeneration":True,
      "original_36f8c8_ordinary_untagged_native_recovered":True,
      "original_36f8c8_tagged_branches_native_recovered":False,
      "original_multi_iteration_and_collision_native_recovered":False,
      "retail_game_playable":False
    }
if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("original_elf",type=Path)
    print(json.dumps(verify(parser.parse_args().original_elf),indent=2))
