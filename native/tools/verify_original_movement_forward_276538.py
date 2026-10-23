#!/usr/bin/env python3
"""Verify original Ape Escape 3 USA 276538 movement vector wrapper.

Private SCUS_975.01 is checked by pinned executable+window SHA.
Only addresses, hashes and instruction FIELD assertions are kept
in git, not any bytes extracted from the proprietary executable.

Without --original-elf this still checks native source and CTest
linkage, but reports retail verification as NOT PERFORMED.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ELF_SHA = "5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
START, END = 0x276538, 0x27655C
WINDOW_SHA = "9fac81a5a29c684c47eb01060fca63b2170cd868e717212e586451296db6a9be"


def elf_window(path: Path) -> bytes:
    contents = path.read_bytes()
    if hashlib.sha256(contents).hexdigest() != ELF_SHA:
        raise ValueError("not original verified USA SCUS_975.01 ELF")
    if contents[:7] != b"\\x7fELF\\x01\\x01\\x01" or (
        struct.unpack_from("<H", contents, 18)[0] != 8
    ):
        raise ValueError("original source is not ELF32 little-endian MIPS")
    phoff = struct.unpack_from("<I", contents, 28)[0]
    stride, count = struct.unpack_from("<HH", contents, 42)
    for i in range(count):
        kind, offset, guest_base, _, nbytes, _, _, _ = struct.unpack_from(
            "<8I", contents, phoff+i*stride
        )
        if kind == 1 and guest_base <= START and END <= guest_base+nbytes:
            raw = contents[offset+START-guest_base:offset+END-guest_base]
            if hashlib.sha256(raw).hexdigest() != WINDOW_SHA:
                raise ValueError("retail 276538 source nine-word window differs")
            return raw
    raise ValueError("original 276538 source segment missing")


def fields(op):
    return {
        "major": op >> 26,
        "base": (op >> 21) & 31,
        "rt": (op >> 16) & 31,
        "rd": (op >> 11) & 31,
        "function": op & 63,
        "immediate": op & 65535,
    }


def verify_retail(path: Path):
    raw = elf_window(path)
    words = {START+4*i:struct.unpack_from("<I",raw,4*i)[0]
             for i in range((END-START)//4)}
    add = fields(words[0x27653C])
    if (add["major"],add["base"],add["rt"],add["immediate"]) != (9,4,4,0x50):
        raise AssertionError("27653C is not original ADDIU a0,a0,0x50")
    sq = fields(words[0x276540])
    if (sq["major"],sq["base"],sq["rt"],sq["immediate"]) != (31,29,5,0):
        raise AssertionError("276540 is not complete 128-bit SQ a1,0(sp)")
    jal = words[0x276548]
    target = ((0x27654C & 0xF0000000) | ((jal & 0x3FFFFFF) << 2))
    if jal >> 26 != 3 or target != 0x274988:
        raise AssertionError("276548 original JAL target no longer 274988")
    move = fields(words[0x27654C])
    if ((move["major"],move["base"],move["rt"],move["rd"],
         (words[0x27654C]>>6)&31,move["function"]) !=
        (0,29,0,5,0,45)):
        raise AssertionError("27654C JAL delay-slot not original 64-bit DADDU a1,sp")
    ret = fields(words[0x276554])
    if ((ret["major"],ret["base"],ret["rt"],ret["function"]) != (0,31,0,8)):
        raise AssertionError("276554 not original JR ra")
    restore = fields(words[0x276558])
    if ((restore["major"],restore["base"],restore["rt"],restore["immediate"])
        != (9,29,29,0x20)):
        raise AssertionError("276558 JR delay-slot stack restore differs")
    return {
        "elf_sha256": ELF_SHA,
        "original_window_sha256": WINDOW_SHA,
        "window": "0x276538..0x27655C",
        "instruction_count": len(words),
        "original_274988_normalizer_jal": "0x276548",
        "source_128bit_SQ": "0x276540",
        "source_signed_addiu": "0x27653C",
        "retail_source_verified": True,
    }


def verify_native(root: Path):
    header = (root/"include"/"ae3"/"original_movement_forward_276538.hpp").read_text(
        encoding="utf-8"
    )
    tail = (root/"include"/"ae3"/"original_movement_post_tail_557fc8.hpp").read_text(
        encoding="utf-8"
    )
    test = (root/"tests"/"original_movement_forward_276538_test.cpp").read_text(
        encoding="utf-8"
    )
    cmake = (root/"unreal_bridge"/"CMakeLists.txt").read_text(
        encoding="utf-8"
    )
    for symbol in ("originalMovementForward_276538(",
                   "originalMovementAddiuParent50_276538(",
                   "call_original_274988(",
                   "static_cast<std::int32_t>(after)"):
        if symbol not in header:
            raise AssertionError("native 276538 wrapper missing "+symbol)
    if "originalMovementForward_276538(" not in tail:
        raise AssertionError("native 557FC8 still requires external 276538")
    if "OriginalMovement276538NativeVectorForward" not in cmake:
        raise AssertionError("native source 276538 regression missing from CTest")
    for fixture in ("0xffffffff80000010ull", "0xffffffff81234050ull",
                    "0x81234000ull", "0xabcdef01u"):
        if fixture not in test:
            raise AssertionError("source stack/ADDIU 276538 ABI case missing "+fixture)
    return {
        "native_source_wrapper_connected": True,
        "original_274988_normalization_body_recovered": False,
        "standalone_ctest_registered": True,
        "native_ctest_executed": False,
        "playable_windows_game_verified": False,
    }


if __name__ == "__main__":
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--native-root",type=Path,
                   default=Path(__file__).resolve().parents[1])
    p.add_argument("--original-elf",type=Path)
    args = p.parse_args()
    result = verify_native(args.native_root)
    result["retail_source_verified"] = False
    if args.original_elf:
        result.update(verify_retail(args.original_elf))
    print(json.dumps(result,indent=2))
