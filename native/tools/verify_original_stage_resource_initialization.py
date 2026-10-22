#!/usr/bin/env python3
"""Audit the owner's USA Ape Escape 3 ELF and real woods_c stage assets.

Reads private original files supplied by the owner; prints only source
addresses, hashes, counts and structural diagnostics. Never writes retail
data or embeds source executable/resources in the repository.
"""
from __future__ import annotations
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct

USA_ELF_SHA256 = "5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
ORIGINAL_WINDOWS = {
    (0x22D7B4, 0x22D970): "5bfbc7ed490928ae7283efd43c092448e40a5cd7c26be2dedc73c3db580f4b95",
    (0x24DC84, 0x24DCB4): "e534af352ecfee6d28373bfa9cef6759a8c53f0961ed91ca1c18aa9d6c36bd4a",
    (0x24D4F0, 0x24D5FC): "7ec60322e5ae08a2e117a0b4c120837a4a7ac1d3c16fda83a2e6930afe6b20ad",
    (0x26DF38, 0x26DF50): "1f7d4a0f338b0a7d04366122d34d6c429ce01828b2cf08441b021fc4a4f2c400",
}
# Ordered retail stage-init resource-key lookups. These are original
# lookup NAMES, not fabricated source filenames or file extensions.
ORIGINAL_STAGE_KEYS = (
    (0x22D828, 0x6E7388, "placeholder", 0x22D844, 0x272294),
    (0x22D8C0, 0x6E7398, "navigation", 0x22D8C8, 0x4F0A5C),
    (0x22D8E8, 0x6E73A8, "actionseq", 0x22D904, 0x30C798),
    (0x22D928, 0x6E73B8, "monkeydb", 0x22D95C, 0x25BFE0),
)
EXTRA_CALLS = {
    0x22D938: 0x24DC9C,
    0x24DC94: 0x24D4F0,
    0x26DF48: 0x26D24C,
}
PCK_RETAIL_SHA256 = {
    "apelodtex.pck": "3a6c06d2cf36ce9d3b5b28fd1cb29bff8a0826836a45000bef4ec15d5c1b1d00",
    "bg.pck": "e406ff6466f6a930b2f9c9ea8a0f926f3f9352170f2dd41e85d6390c7120352e",
    "character.pck": "993c71929259083102c3cb643c4683edb0e6db757cb71106589c35b7745d61b3",
    "exdb.pck": "185db8cfd710760d9c715e0d1cce8ac4ecf6b18201673ce00bc6815078ae4615",
    "navigation.pck": "d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0",
}
EXPECTED_PCK_COUNTS = {"apelodtex.pck": 4, "bg.pck": 73,
                       "character.pck": 79, "exdb.pck": 2,
                       "navigation.pck": 2}

def check(value: bool, message: str) -> None:
    if not value:
        raise ValueError(message)

def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()

def le16(data: bytes, at: int) -> int:
    check(0 <= at <= len(data) - 2, f"truncated original u16 at {at:#x}")
    return struct.unpack_from("<H", data, at)[0]

def le32(data: bytes, at: int) -> int:
    check(0 <= at <= len(data) - 4, f"truncated original u32 at {at:#x}")
    return struct.unpack_from("<I", data, at)[0]

def original_elf(elf_path: Path) -> dict:
    elf = elf_path.read_bytes()
    check(sha256(elf) == USA_ELF_SHA256, "wrong original USA SCUS_975.01 SHA-256")
    check(elf[:7] == b"\x7fELF\x01\x01\x01" and le16(elf, 18) == 8,
          "expected 32-bit little-endian MIPS retail ELF")
    phoff = le32(elf, 28)
    phentsize, phnum = struct.unpack_from("<HH", elf, 42)
    check(phentsize >= 32 and phoff + phentsize * phnum <= len(elf),
          "ELF program headers truncated")
    segments = []
    for index in range(phnum):
        at = phoff + index * phentsize
        typ, off, va, _, size, _, _, _ = struct.unpack_from("<8I", elf, at)
        if typ == 1:
            check(off + size <= len(elf), "original ELF load segment exceeds file")
            segments.append((va, va + size, off))

    def view(va: int, size: int) -> bytes:
        for lo, hi, off in segments:
            if lo <= va and va + size <= hi:
                return elf[off + va - lo:off + va - lo + size]
        raise ValueError(f"unmapped original ELF VA {va:#x}")

    def jump(pc: int, target: int, opcode: int) -> None:
        instruction = le32(view(pc, 4), 0)
        check(instruction >> 26 == opcode and
              (((pc + 4) & 0xf0000000) | ((instruction & 0x03ffffff) << 2)) == target,
              f"retail branch/jump at {pc:#x} does not target {target:#x}")

    for (lo, hi), fingerprint in ORIGINAL_WINDOWS.items():
        check(sha256(view(lo, hi - lo)) == fingerprint,
              f"retail code at {lo:#x} differs from pinned original")
    for lookup_pc, literal_va, key, consumer_pc, consumer_va in ORIGINAL_STAGE_KEYS:
        check(view(literal_va, len(key) + 1) == key.encode() + b"\x00",
              f"original resource key at {literal_va:#x} differs")
        jump(lookup_pc, 0x24DC84, 3)
        jump(consumer_pc, consumer_va, 3)
    jump(0x22D938, 0x24DC9C, 3)
    jump(0x24DC94, 0x24D4F0, 2)
    jump(0x26DF48, 0x26D24C, 2)
    # Source lookup at 0x22D928 is followed by a second original
    # lookup at 0x22D938; this is not the same flow as the first three.
    return {
        "retail_ELF_sha256": USA_ELF_SHA256,
        "whole_source_windows": len(ORIGINAL_WINDOWS),
        "verified_instruction_words": sum((hi - lo) // 4 for lo, hi in ORIGINAL_WINDOWS),
        "verified_direct_jumps": 11,
        "original_stage_init": "0x22d7b4",
        "lookup_wrapper": "0x24dc84",
        "lookup_core": "0x24d4f0",
        "typed_resource_selector": "0x26d24c",
        "ordered_lookup_keys": [
            {"name": key, "literal_va": hex(literal_va),
             "lookup_call": hex(lookup_pc), "consumer_call": hex(consumer_pc),
             "consumer_va": hex(consumer_va)}
            for lookup_pc, literal_va, key, consumer_pc, consumer_va in ORIGINAL_STAGE_KEYS
        ],
        "monkeydb_additional_lookup_call": "0x22d938",
    }

def absolute_source_name(source: bytes, position: int, table: int) -> str:
    check(0x10 <= position < table and
          (position == 0x10 or source[position-1] == 0),
          "PCK name/type pointer is not an absolute string start")
    end = source.find(b"\x00", position, table)
    check(position < end <= position + 255, "unterminated/oversized PCK source identifier")
    name = source[position:end]
    check(all(32 <= b < 127 for b in name), "nonprintable original PCK identifier")
    return name.decode("ascii")

def original_pck(path: Path) -> list:
    raw = path.read_bytes()
    check(sha256(raw) == PCK_RETAIL_SHA256[path.name],
          f"original {path.name} source SHA-256 mismatch")
    check(raw[:4] == b"PCK\x00", f"original {path.name} has wrong signature")
    table, count = struct.unpack_from("<II", raw, 4)
    check(0x10 <= table and table + count * 16 <= len(raw),
          f"original {path.name} entry table outside source")
    check(count == EXPECTED_PCK_COUNTS[path.name],
          f"original {path.name} member count mismatch")
    entries = []
    for n in range(count):
        name_offset, type_offset, data_offset, size = struct.unpack_from("<4I", raw, table + 16*n)
        check(data_offset >= table + count * 16 and
              data_offset <= len(raw) and size <= len(raw) - data_offset,
              "original PCK resource payload out of bounds")
        name = absolute_source_name(raw, name_offset, table)
        kind = absolute_source_name(raw, type_offset, table)
        entries.append((name, kind, raw[data_offset:data_offset + size]))
    return entries

def validate_nav(name: str, payload: bytes) -> dict:
    check(len(payload) >= 0x50, f"truncated source {name} nav header")
    count = le32(payload, 0x24)
    offsets = struct.unpack_from("<6I", payload, 0x38)
    check(count <= (len(payload) - offsets[1]) // 0x1e if offsets[1] <= len(payload) else False,
          f"original {name} nav descriptor table outside allocation")
    actions = Counter()
    vertices = Counter()
    extra = 0
    for record in range(count):
        descriptor = offsets[1] + record*0x1e
        selector = le16(payload, descriptor)
        action_count = le16(payload, descriptor + 2)
        vcount = le16(payload, descriptor + 0x16)
        check(action_count <= 3, f"{name}: action count exceeds fixed descriptor slots")
        check(1 <= vcount <= 3, f"{name}: unsupported original polygon count")
        for i in range(action_count):
            _type = le16(payload, descriptor + 0x0a + 2*i)
            _argument = le16(payload, descriptor + 0x10 + 2*i)
        for i in range(vcount):
            vertex_id = le16(payload, descriptor + 0x18 + 2*i)
            vertex_offset = offsets[2] + vertex_id*16
            check(vertex_offset <= len(payload) and
                  len(payload)-vertex_offset >= 12,
                  f"{name}: original vertex array source out of bounds")
        if selector != 0xffff:
            extra_offset = offsets[4] + selector * 32
            check(extra_offset <= len(payload) and
                  len(payload) - extra_offset >= 32,
                  f"{name}: original selected 0x20-byte record truncated")
            extra += 1
        actions[action_count] += 1
        vertices[vcount] += 1
    return {"name": name, "records": count,
            "descriptor_action_counts": dict(sorted(actions.items())),
            "polygon_point_counts": dict(sorted(vertices.items())),
            "resolved_optional_records": extra}

def original_stage(stage: Path) -> dict:
    check(stage.is_dir(), "source must be private extracted woods_c stage folder")
    inventory = {}
    nav_entries = None
    for filename in PCK_RETAIL_SHA256:
        parsed = original_pck(stage / filename)
        inventory[filename] = len(parsed)
        if filename == "navigation.pck":
            nav_entries = parsed
    check(sum(inventory.values()) == 160, "unexpected original woods_c PCK inventory")
    assert nav_entries is not None
    check([(name, kind) for name,kind,_ in nav_entries] ==
          [("nav_monkey.nav", "nav"), ("nav_enemy.nav", "nav")],
          "original woods_c navigation package member types/names differ")
    nav = [validate_nav(name, payload) for name,kind,payload in nav_entries]
    check([item["records"] for item in nav] == [39,20],
          "original woods_c navigation source count mismatch")
    for source_filename in ("placeholder.plc","actionseq.asq","monkeydb.xml"):
        check((stage/source_filename).is_file(),
              "source-named stage resource candidate absent: "+source_filename)
    return {
        "private_stage_sample": "woods_c",
        "pck_source_sha256_verified": len(PCK_RETAIL_SHA256),
        "original_package_members": inventory,
        "original_navigation_records": nav,
        "original_init_key_filename_candidates": {
            "placeholder": "placeholder.plc",
            "navigation": "navigation.pck",
            "actionseq": "actionseq.asq",
            "monkeydb": "monkeydb.xml",
        },
        "scope_note": "Filenames are observed stem matches, not proof of extension mapping in the original game loader.",
    }

def main() -> None:
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("original_usa_elf",type=Path)
    parser.add_argument("private_woods_c_stage_folder",type=Path)
    args=parser.parse_args()
    report={"source":original_elf(args.original_usa_elf),
            "original_stage":original_stage(args.private_woods_c_stage_folder)}
    print(json.dumps(report,indent=2))
    print("PASS: original USA ELF stage-init code and real woods_c stage packages/navigation verified")

if __name__=="__main__":
    main()
