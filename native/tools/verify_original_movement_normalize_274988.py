#!/usr/bin/env python3
"""Verify the original USA Ape Escape 3 0x274988 VU0 normalizer.

Retail mode needs the owner-private SCUS_975.01 ELF; SHA-256
checks the entire executable, 32-word source window and both
genuine float table constants. Only metadata is committed.
Without --original-elf, check native source/CMake integration,
but do NOT claim original binary verification or CTest success.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ELF_SHA="5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
WINDOW_SHA="9190ac5270ec67c8803b7b4b08c8276b95c459c4386be2890748b5c79b9474f5"
START,END=0x274988,0x274A08
WORDS={
    0x274988:0x78AF0000, # 128-bit LQ t7,0(a1)
    0x274990:0x71CF7488, # PEXTLW t6,t6,t7
    0x274994:0x71EF7BA9, # PCPYUD t7,t7,t7
    0x274998:0x71EE7389, # PCPYLD t6,t7,t6
    0x27499C:0x48AE1000, # DMTC2 t6,VF2
    0x2749A0:0x4BE2106A, # VMUL.xyzw VF2,VF2,VF2
    0x2749A4:0x4BA10841, # VADDy.xyw VF1,VF2,VF2
    0x2749A8:0x4BE10842, # VADDz.xyzw VF1,VF1,VF2
    0x2749AC:0x482D0800, # DMFC2 VF1.x
    0x2749B4:0x448F0000, # MTC1 squared length
    0x2749C8:0x46000044, # SQRT.S
    0x2749CC:0xC5A00000, # LWC1 original epsilon
    0x2749D0:0x46010034, # C.OLT.S epsilon, length
    0x2749D4:0x4500000A, # BC1F skip normalization
    0x2749E0:0xC5E00000, # LWC1 original 1.0
    0x2749EC:0x46010083, # DIV.S 1/length
    0x2749F0:0x440D1000, # MFC1 scalar reciprocal
    0x2749F4:0x48AD0800, # DMTC2 reciprocal into VF1
    0x2749F8:0x4BE11058, # VMULx.xyzw VF1,VF2,VF1.x
    0x2749FC:0xF8810000, # SQ VF1,0(a0)
    0x274A00:0x03E00008, # JR ra
}

def source_range(data:bytes, addr:int, length:int)->bytes:
    if data[:7]!=b"\x7fELF\x01\x01\x01" or struct.unpack_from("<H",data,18)[0]!=8:
        raise ValueError("expected the original MIPS32 little-endian ELF")
    phoff=struct.unpack_from("<I",data,28)[0]
    stride,count=struct.unpack_from("<HH",data,42)
    if stride<32 or phoff+stride*count>len(data):
        raise ValueError("ELF program header table invalid")
    for idx in range(count):
        typ,off,va,_,filesz,_,_,_=struct.unpack_from("<8I",data,phoff+idx*stride)
        if typ==1 and va<=addr and addr+length<=va+filesz:
            foff=off+addr-va
            if foff+length>len(data):break
            return data[foff:foff+length]
    raise ValueError("source address not file-backed in original "+hex(addr))

def verify_retail(path:Path)->dict:
    data=path.read_bytes()
    if hashlib.sha256(data).hexdigest()!=ELF_SHA:
        raise ValueError("not the pinned original USA SCUS_975.01")
    raw=source_range(data,START,END-START)
    if hashlib.sha256(raw).hexdigest()!=WINDOW_SHA:
        raise ValueError("original 274988 source window differs")
    for pc,expected in WORDS.items():
        actual=struct.unpack_from("<I",raw,pc-START)[0]
        if actual!=expected:
            raise ValueError(f"original opcode mismatch at {pc:#x}")
    if len(raw)//4!=32:
        raise AssertionError("retail normalization instruction count changed")
    # BC1F at 2749D4 targets source 274A00 (JR) when
    # epsilon >= length. Its architectural delay slot executes.
    inst=WORDS[0x2749D4]
    if (inst>>26)!=0x11 or ((inst>>21)&31)!=8 or ((inst>>16)&31)!=0:
        raise AssertionError("original normalization branch is no longer BC1F")
    disp=inst&0xFFFF
    if disp>=0x8000:disp-=0x10000
    if 0x2749D4+4+disp*4!=0x274A00:
        raise AssertionError("original near-zero branch does not return directly")
    # This is the source-critical distinction: near-zero
    # 2749D4 BC1F goes to JR, bypassing 2749FC SQ VF1 to
    # parent+0x50. The output memory is NOT always written.
    if WORDS[0x2749FC]!=0xF8810000:
        raise AssertionError("original normalized branch 2749FC SQ changed")
    # The original data table is source evidence; no original
    # constants are inferred from approximate host math.
    epsilon=struct.unpack("<I",source_range(data,0x6ED7C0,4))[0]
    one=struct.unpack("<I",source_range(data,0x6ED7C4,4))[0]
    if (epsilon,one)!=(0x2EDBE6FF,0x3F800000):
        raise ValueError("original normalization epsilon/one constants differ")
    return {
        "elf_sha256":ELF_SHA,
        "original_window":"0x274988..0x274A08",
        "window_sha256":WINDOW_SHA,
        "instruction_count":32,
        "pinned_opcode_sites":len(WORDS),
        "source_epsilon_raw":hex(epsilon),
        "source_one_raw":hex(one),
        "original_source_verified":True
    }

def verify_native(root:Path)->dict:
    header=(root/"include"/"ae3"/"original_movement_normalize_274988.hpp").read_text(encoding="utf-8")
    impl=(root/"src"/"original_movement_normalize_274988.cpp").read_text(encoding="utf-8")
    caller=(root/"include"/"ae3"/"original_movement_post_tail_557fc8.hpp").read_text(encoding="utf-8")
    cmake=(root/"unreal_bridge"/"CMakeLists.txt").read_text(encoding="utf-8")
    tests=(root/"tests"/"original_movement_normalize_274988_test.cpp").read_text(encoding="utf-8")
    for symbol in (
        "originalEEStackA1", "originalLiveVF1Before",
        "originalVF2Packed", "originalVF1BeforeThreshold",
        "originalMovementNormalizeExact_274988(",
    ):
        if symbol not in header:
            raise AssertionError("missing genuine VU0 normalizer API "+symbol)
    for piece in (
        "originalEpsilonBits=0x2EDBE6FFu",
        "source.originalEEStackA1[2]",
        "prior[2]",
        "originalVF2Packed",
        "originalVF1BeforeThreshold",
        "mulExact(pack[i],reciprocal)",
        "originalPerformedNormalize=false",
    ):
        if piece not in impl:
            raise AssertionError("retail normalizer source behavior missing "+piece)
    for piece in (
        "read_original_live_vf1_before_274988",
        "publish_original_274988_vu0",
        "originalMovementNormalizeExact_274988(",
        "originalMovementForward_276538(",
        "publish_original_274988_vu0(originalParentPlus50,source)",
        "if(result.originalPerformedNormalize)",
    ):
        if piece not in caller:
            raise AssertionError("original 557FC8 owner not connected to "+piece)
    import re
    all_targets=re.findall(r"add_executable\([^\n]*\n[\s\S]*?\)\n",cmake)
    tails=[x for x in all_targets if "original_movement_post_tail_557fc8.cpp" in x]
    if len(tails)!=8 or any("original_movement_normalize_274988.cpp" not in x for x in tails):
        raise AssertionError("native 274988 source missing from one or more of eight 557FC8 test targets")
    if "original_movement_normalize_274988.cpp" not in cmake[
        cmake.index("set(ORIGINAL_ACTOR_MANAGER_SOURCES"):
        cmake.index(")",cmake.index("set(ORIGINAL_ACTOR_MANAGER_SOURCES"))
    ]:
        raise AssertionError("native actor manager library lacks source 274988 normalizer")
    if "OriginalMovement274988SourceVU0ExactNormalization" not in cmake:
        raise AssertionError("original native 274988 normalization CTest missing")
    for needle in (
        "0x7fc01234u","bits(0x1p-35f)","bits(3.0f)",
        "originalVF1BeforeThreshold","std::domain_error",
    ):
        if needle not in tests:
            raise AssertionError("native normalized state regression absent "+needle)
    return {
        "native_original_mmi_xz_packing_connected":True,
        "native_exact_finite_vu0_subset_connected":True,
        "post_tail_test_targets_linked":len(tails),
        "standalone_normalizer_ctest_registered":True,
        "source_nearzero_skips_parent_50_store":True,
        "original_special_ps2_rounding_recovered":False,
        "original_windows_game_playable_verified":False
    }

if __name__=="__main__":
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--native-root",type=Path,default=Path(__file__).resolve().parents[1])
    ap.add_argument("--original-elf",type=Path)
    args=ap.parse_args()
    out=verify_native(args.native_root)
    out["original_source_verified"]=False
    if args.original_elf:
        out.update(verify_retail(args.original_elf))
    print(json.dumps(out,indent=2))
