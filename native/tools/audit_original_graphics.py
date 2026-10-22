#!/usr/bin/env python3
"""Read-only source evidence for USA SCUS_975.01 graphics call sites.

Requires the owner's private original executable, verified by SHA-256.
No game bytes, disassembly dumps, or copyrighted resources are written.
References below are observed MIPS LUI + ADDIU/ORI address materializations,
not recovered C++ rendering function implementations.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import struct
from pathlib import Path

RETAIL_SHA256 = "5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
TEXT_FILE = 0x1000
TEXT_VA = 0x100000
TEXT_SIZE = 0x528258
RODATA_FILE = 0x5B9000
RODATA_VA = 0x6B8000
ANCHORS = {
    "I3dRender": (0x6EB3F8, [0x253748]),
    "local matrix array": (0x6ECE40, [0x268F38, 0x269164]),
    "model matrix array": (0x6ECE58, [0x268F58, 0x269184]),
    "calc render": (0x6EDC40, [0x2807B8, 0x280A48]),
    "calc render sub": (0x6EDC60, [0x280D90, 0x281070]),
    "camera data": (0x6C3FD0, [0x114EAC, 0x114F54]),
}


def u32(source: bytes, offset: int) -> int:
    return struct.unpack_from("<I", source, offset)[0]


def address_references(source: bytes, address: int) -> list[int]:
    """Find retail LUI reference materializations in .text by instruction VA.

    Return the LUI VA, with an ADDIU or ORI completion at most 7 instructions
    later. Some real instructions between the pair are MIPS branch delay slots.
    These are *candidate* references: register flow is not decompiled here.
    """
    refs = []
    possible_high = {(address >> 16) & 0xFFFF,
                     ((address + 0x8000) >> 16) & 0xFFFF}
    for off in range(TEXT_FILE, TEXT_FILE + TEXT_SIZE - 32, 4):
        instruction = u32(source, off)
        if instruction >> 26 != 15 or (instruction & 0xFFFF) not in possible_high:
            continue
        register = (instruction >> 16) & 31
        high = instruction & 0xFFFF
        for delta in range(1, 8):
            next_inst = u32(source, off + delta * 4)
            opcode = next_inst >> 26
            if opcode not in (9, 13) or ((next_inst >> 21) & 31) != register:
                continue
            low = next_inst & 0xFFFF
            # ADDIU sign extends the lower half; ORI zero extends it.
            signed_low = low if opcode == 13 or low < 0x8000 else low - 0x10000
            if (high * 0x10000 + signed_low) & 0xFFFFFFFF == address:
                refs.append(off - TEXT_FILE + TEXT_VA)
                break
    return sorted(set(refs))


def verify_and_audit(elf: bytes) -> dict:
    digest = hashlib.sha256(elf).hexdigest()
    if digest != RETAIL_SHA256:
        raise ValueError("not the verified original Ape Escape 3 USA SCUS_975.01")
    if elf[:4] != b"\x7fELF" or elf[4] != 1 or elf[5] != 1:
        raise ValueError("invalid ELF32 LE header")
    assert len(elf) >= TEXT_FILE + TEXT_SIZE
    report = {
        "verified_usa_executable_sha256": digest,
        "meaning": "MIPS text-to-rodata reference provenance only; not game renderer reconstruction",
        "anchors": [],
    }
    for label, (expected_va, expected_refs) in ANCHORS.items():
        string_at = elf.find(label.encode("ascii") + b"\0", RODATA_FILE)
        if string_at < 0:
            raise ValueError("retail graphics anchor missing: " + label)
        va = string_at - RODATA_FILE + RODATA_VA
        if va != expected_va:
            raise ValueError(f"graphics anchor VA mismatch: {label} {va:#x}")
        refs = address_references(elf, va)
        if refs != expected_refs:
            raise ValueError(f"graphics LUI xrefs changed: {label} {refs}")
        report["anchors"].append({
            "label": label,
            "retail_string_va": f"0x{va:08x}",
            "retail_lui_ref_vas": [f"0x{r:08x}" for r in refs],
        })
    return report


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("elf", type=Path, help="path to owner-private SCUS_975.01")
    parser.add_argument("--output", type=Path, help="optional small JSON evidence report")
    arguments = parser.parse_args()
    report = verify_and_audit(arguments.elf.read_bytes())
    encoded = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if arguments.output:
        arguments.output.write_text(encoded, encoding="utf-8")
    else:
        print(encoded, end="")


if __name__ == "__main__":
    main()
