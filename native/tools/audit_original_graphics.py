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
    render_gate_start = 0x280838
    render_gate_end = 0x280884
    file_start = render_gate_start - TEXT_VA + TEXT_FILE
    file_end = render_gate_end - TEXT_VA + TEXT_FILE
    gate_hash = hashlib.sha256(elf[file_start:file_end]).hexdigest()
    if gate_hash != "a2ed3f2062ea0d0f7de6d252dd468d1f8345833d57a950cf1f5eb67b64a9fa00":
        raise ValueError("retail render early-gate MIPS instructions changed")
    selector_ranges = [
        (0x26AEB4, 0x26AEEC, "1967c1dcdba937f708d2a81d196be87b1bc5e0a75f5dd3488738e7662402d128"),
        (0x26B550, 0x26B594, "6c54314ffd23bb3971c0a9f8742d58895114a1825c376c97c8843ce463a061ab"),
        (0x280884, 0x2808D4, "2489a7a7aad1da59f40bc6e84e31c095ec42c7812325bd1d5ff10e53b53af311"),
    ]
    selector_evidence = []
    for first, end, expected in selector_ranges:
        span = elf[first - TEXT_VA + TEXT_FILE:end - TEXT_VA + TEXT_FILE]
        digest_span = hashlib.sha256(span).hexdigest()
        if digest_span != expected:
            raise ValueError(f"retail selector instructions changed at {first:#x}")
        selector_evidence.append({
            "va_start": f"0x{first:08x}",
            "va_end_exclusive": f"0x{end:08x}",
            "source_sha256": digest_span,
        })
    # Fingerprint the actual retail code governing the newly native-ported
    # primary and secondary draw traversals, resource effect query, and
    # (still-unported) original bounds owner entry. Each range is end-exclusive.
    render_call_ranges = [
        (0x280810, 0x280968, "2e5208f2d3a5011fb14547dd136a3024c2f5ae7d1cf88e0cf87d48725f4e5dc3"),
        (0x280E20, 0x280EB8, "4a792023553cf070bf97e992aa560185e6992b88e4b1fd83fae4cff60ec9c9e0"),
        (0x26B3D0, 0x26B3FC, "764c6db6a386166eab9621a815e42c60738affbeeee7d903e82b41bd4a187198"),
        (0x26A968, 0x26A980, "ecbb590540e60a82dc670dfd606d0408498bc2d236b514168aa155e5bc34522b"),
        (0x280A90, 0x280ABC, "09728d913e02ed6e44f0cadb6aea8a436931d3e70ec0407a2bc10da601565f61"),
        (0x2802D4, 0x2802F8, "df96d533136361022447536b0e381b5ce9337cd0472f2f195b070785680e78ef"),
        (0x26C9B8, 0x26C9D4, "c08bbe94a2e307a3200b51de2522332b0c72805da4effe414a0eec93ef6ceaa8"),
    ]
    render_evidence = []
    for first, end, expected in render_call_ranges:
        contents = elf[first - TEXT_VA + TEXT_FILE:end - TEXT_VA + TEXT_FILE]
        actual = hashlib.sha256(contents).hexdigest()
        if actual != expected:
            raise ValueError(f"retail render traversal changed at {first:#x}")
        render_evidence.append({
            "va_start": f"0x{first:08x}",
            "va_end_exclusive": f"0x{end:08x}",
            "sha256": actual,
        })
    # Retail owner scheduler and modulo selection. '0x4233F8' writes the
    # division remainder through a stack output argument and returns it.
    owner_ranges = [
        (0x280B7C, 0x280C80, "69dc32f2de97a12e4ee3a7a0c0cd4d35f38990c7434ba26d7c1d95f75289b190"),
        (0x4233F8, 0x423418, "a70c622eefdea82e7f22aa193ffa23caa2fe4a23e6aff2dde2e04cb011ff0e2c"),
        (0x26A974, 0x26A980, "e1370cb89f6e17b9fc6f89479d8d1eb39fb7129817893a16fd6de4b85116b75e"),
    ]
    owner_evidence = []
    for first, end, expected in owner_ranges:
        span = elf[first - TEXT_VA + TEXT_FILE:end - TEXT_VA + TEXT_FILE]
        actual = hashlib.sha256(span).hexdigest()
        if actual != expected:
            raise ValueError(f"retail render owner source mismatch at {first:#x}")
        owner_evidence.append({"va_start":f"0x{first:08x}",
                               "va_end_exclusive":f"0x{end:08x}",
                               "sha256":actual})
    # Additional retail instruction fingerprints: decoded renderer-context
    # pointer slots, +0x110 resource lifecycle/flag accessors, source-timed
    # actor render entry, and the 0x2804F4 material packet dependency chain.
    # These prove source bytes only, NOT end-to-end native GPU drawing.
    recovered_graphics_ranges = [
        (0x3B76D0, 0x3B7740, "ac883e9b102c531ce8061c48a8161f27e2bf8a8909e408a55b3771ff4aa34a93"),
        (0x26A4C4, 0x26A55C, "7a15cd6dc48d8dac29d2e55b34ca37d9fc522de7c5f60726c315aa3958856e33"),
        (0x26A854, 0x26A8A4, "6da75758cde672462a2460741043bba9ebd7fa63c353fc7ff8dd14018eeea262"),
        (0x26A8AC, 0x26A8F4, "f6abab67768fdc1431cd7fc1c0e6ace3f72d08d0fc083c406bf10d63ee840717"),
        (0x26A938, 0x26A980, "6a5231aba8fe0365f8456d8e194fe574ad9bb7e75370bcb8d711d70b55972011"),
        (0x26831C, 0x268424, "4ed314ac49d5c0a81d11cfa20d14de81e50a5d4738a13ab9152ce2c4b1bf5c69"),
        (0x2802D4, 0x280328, "e55bff41b6a4c185107783132caf36163e8290aa0bd70ce146fbe242bea4a7b7"),
        (0x280328, 0x280364, "908308ffee03d34f74f18d64639dd39b25fdd97d2879a47c1ea9db0781d3d121"),
        (0x2803B4, 0x2804F4, "45115d2c0466c8299d83b2a716785192c90cc7a7111c4e5c64ed442182f372a2"),
        (0x2804F4, 0x280544, "056a50d5e7eacea38c602f34fa2933391b4a3dd185ef0e838cf9f41815bc3a37"),
    ]
    recovered_graphics_evidence = []
    for first, end, expected in recovered_graphics_ranges:
        data = elf[first - TEXT_VA + TEXT_FILE:end - TEXT_VA + TEXT_FILE]
        computed = hashlib.sha256(data).hexdigest()
        if computed != expected:
            raise ValueError(f"original renderer data mismatch at {first:#x}")
        recovered_graphics_evidence.append({
            "start_va": f"0x{first:08x}",
            "end_va_exclusive": f"0x{end:08x}",
            "source_sha256": computed,
        })
    # Source-verified original model draw bridge and full five-segment
    # packet assembler. Do not mistake this proof of instruction provenance
    # for completion of 0x27C614, 0x27C520 or the VU-assisted actor path.
    packet_ranges = [
        (0x280364, 0x2803B4, "e59a2f65148e03ce001d7efd4418e067b8554e7e06237db10450a1f327fefbb9"),
        (0x3AF8F0, 0x3AF9C4, "203f02462c8b8e43621242b6d67147c0beeaa91aedb52feb990f4a92b08fb7ea"),
        (0x3AF8B8, 0x3AF8F0, "6fc509515ce05af8fc878d9cc797a9f5028a4abbbf1445700a3fb086d6927b0c"),
        (0x3AF478, 0x3AF530, "7dfd9036d8b7b34767af38b93d11aa646250b1459e981b5326983920ff9ae40e"),
    ]
    packet_evidence = []
    for first, end, expected in packet_ranges:
        span = elf[first - TEXT_VA + TEXT_FILE:end - TEXT_VA + TEXT_FILE]
        actual = hashlib.sha256(span).hexdigest()
        if actual != expected:
            raise ValueError(f"retail graphics packet assembler changed at {first:#x}")
        packet_evidence.append({"start_va": f"0x{first:08x}",
                                "end_va_exclusive": f"0x{end:08x}",
                                "sha256": actual})
    # Original source packet segment copy routines. These use count*0x20
    # and count*0x30 native transfers, unlike the transform-heavy groups.
    copy_ranges = [
        (0x3AF600, 0x3AF680, "d81f350f97a6c04771884fcb544166391ceaac7604825fa11564234e4edf17a4"),
        (0x3AF680, 0x3AF6F8, "4953e4927eeaa9b066fbb1a72f30706bcfb3d644acd9f230b2da1ab96a2e416b"),
        (0x42B16C, 0x42B224, "a7f576b7fc50f78255778c0773db38be4ae78a38dbbf0758d7eb52dfbb35d2ad"),
    ]
    copy_evidence = []
    for first, end, expected in copy_ranges:
        section = elf[first - TEXT_VA + TEXT_FILE:end - TEXT_VA + TEXT_FILE]
        digest_copy = hashlib.sha256(section).hexdigest()
        if digest_copy != expected:
            raise ValueError(f"original native packet copy source changed at {first:#x}")
        copy_evidence.append({"start_va": f"0x{first:08x}",
                              "end_va_exclusive": f"0x{end:08x}",
                              "sha256": digest_copy})
    # Source graphics packet arena uses two exact 16-byte unit allocators,
    # at global owner+0x18 (backward) and owner+0x14 (forward).
    arena_ranges = [
        (0x3B3CE0, 0x3B3CFC, "13c8b65e6a2bcac7c1ffb3a980f280d4f2cbe70dcc81ca238fa0d27f993e8f92"),
        (0x3B3D00, 0x3B3D20, "4c2ea7ef8022fcd90bf053c968baa32e8650847c95829d708742e35e7639e9bd"),
    ]
    arena_evidence = []
    for first, end, expected in arena_ranges:
        data = elf[first - TEXT_VA + TEXT_FILE:end - TEXT_VA + TEXT_FILE]
        digest_arena = hashlib.sha256(data).hexdigest()
        if digest_arena != expected:
            raise ValueError(f"retail renderer allocation routine differs at {first:#x}")
        arena_evidence.append({"start_va": f"0x{first:08x}",
                               "end_va_exclusive": f"0x{end:08x}",
                               "sha256": digest_arena})
    report = {
        "source_verified_original_packet_arena": arena_evidence,
        "source_verified_native_packet_copies": copy_evidence,
        "source_verified_original_model_packet_pipeline": packet_evidence,
        "source_verified_render_context_and_actor_entry": recovered_graphics_evidence,
        "source_verified_render_owner": owner_evidence,
        "source_verified_draw_traversal": render_evidence,
        "retail_selector_instruction_ranges": selector_evidence,
        "retail_gate_va_range": ["0x00280838", "0x00280884"],
        "retail_gate_instruction_sha256": gate_hash,
        "retail_gate_ported_gate_only": True,
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
