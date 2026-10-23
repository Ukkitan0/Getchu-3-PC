#!/usr/bin/env python3
"""Verify the original USA 276234 full EE 128-bit LQ/SQ vector copy.

Private ELF is SHA-256 verified; only original code hashes,
addresses and narrow opcode fields are included in this repo.
The native CTest can run without an original executable.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ELF_SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
START,END=0x276234,0x276240
WINDOW_SHA="0f7121db3eab577e6cd5103201965a66f1a1d805730f691c4979b625e021c034"

def retail_word(data,addr):
    phoff=struct.unpack_from("<I",data,28)[0]
    step,count=struct.unpack_from("<HH",data,42)
    for i in range(count):
        kind,off,base,_,length,_,_,_=struct.unpack_from("<8I",data,phoff+step*i)
        if kind==1 and base<=addr and addr+4<=base+length:
            return data[off+addr-base:off+addr-base+4]
    raise ValueError("source original address not available "+hex(addr))

def verify_retail(path):
    data=path.read_bytes()
    if hashlib.sha256(data).hexdigest()!=ELF_SHA:
        raise ValueError("not original USA SCUS_975.01")
    if data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0]!=8:
        raise ValueError("expected original MIPS little-endian ELF32")
    words=[struct.unpack("<I",retail_word(data,pc))[0]
           for pc in range(START,END,4)]
    window=b"".join(struct.pack("<I",w) for w in words)
    if hashlib.sha256(window).hexdigest()!=WINDOW_SHA:
        raise ValueError("retail 276234 source 3-word window changed")
    # Source LQ t7,0(a1), return, JR delay slot SQ t7,0x40(a0).
    lq,jr,sq=words
    if ((lq>>26,(lq>>21)&31,(lq>>16)&31,lq&0xFFFF)
        != (30,5,15,0)):
        raise ValueError("source 276234 is not LQ t7,0(a1)")
    if jr != 0x03E00008:
        raise ValueError("source 276238 not JR ra")
    if ((sq>>26,(sq>>21)&31,(sq>>16)&31,sq&0xFFFF)
        != (31,4,15,0x40)):
        raise ValueError("source 27623C is not SQ t7,0x40(a0)")
    direct=[]
    for pc in (0x30C114,0x30C14C):
        word=struct.unpack("<I",retail_word(data,pc))[0]
        target=((pc+4)&0xF0000000)|((word&0x3FFFFFF)<<2)
        if word>>26!=3 or target!=START:
            raise ValueError("real 30C0A0 source not calling 276234 at "+hex(pc))
        direct.append(hex(pc))
    return {
        "retail_verified":True,
        "elf_sha256":ELF_SHA,
        "source_window":"0x276234..0x276240",
        "window_sha256":WINDOW_SHA,
        "source_instruction_count":3,
        "original_30c0a0_direct_call_sites":direct,
    }

def verify_native(root):
    header=(root/"include"/"ae3"/"original_movement_vector_copy_276234.hpp").read_text(encoding="utf-8")
    tests=(root/"tests"/"original_movement_vector_copy_276234_test.cpp").read_text(encoding="utf-8")
    cmake=(root/"unreal_bridge"/"CMakeLists.txt").read_text(encoding="utf-8")
    for item in ("originalMovementVectorCopy_276234(",
                 "originalA0GuestVA+0x40ull",
                 "store_original_128bit_at_guest("):
        if item not in header:raise AssertionError("missing original 276234 "+item)
    if "OriginalMovement276234Source128BitMemoryCopy" not in cmake:
        raise AssertionError("original 276234 native regression not registered")
    for item in ("0xfedcba98u","0xffffffff81234040ull",
                 "0x340004ull","std::runtime_error"):
        if item not in tests:raise AssertionError("missing 128-bit source test "+item)
    return {
        "native_source_276234_128bit_copy_connected":True,
        "standalone_ctest_registered":True,
        "original_30c0a0_entire_manager_translated":False,
        "original_windows_game_playable_verified":False,
    }

if __name__=="__main__":
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--native-root",type=Path,
        default=Path(__file__).resolve().parents[1])
    ap.add_argument("--original-elf",type=Path)
    args=ap.parse_args()
    result=verify_native(args.native_root)
    result["retail_verified"]=False
    if args.original_elf:result.update(verify_retail(args.original_elf))
    print(json.dumps(result,indent=2))
