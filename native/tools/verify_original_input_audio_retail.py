#!/usr/bin/env python3
"""Audit original Ape Escape 3 USA input/audio Lua command call edges.

Reads only a private owner-supplied USA SCUS_975.01. No game bytes are
emitted or stored in this repo. This is NOT a controller or playback test.
"""
from __future__ import annotations

import argparse
import hashlib
import struct
from pathlib import Path

EXPECTED_ELF_SHA256 = "5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63"
# ELF file offset 0x1000 is mapped to executable VA 0x00100000.
SEGMENT_DELTA = 0xFF000


def audit(original_elf: Path) -> list[str]:
    data = original_elf.read_bytes()
    if hashlib.sha256(data).hexdigest() != EXPECTED_ELF_SHA256:
        raise AssertionError("not SHA-verified original USA SCUS_975.01")

    def word(va: int) -> int:
        return struct.unpack_from("<I", data, va - SEGMENT_DELTA)[0]

    def scalar(va: int) -> float:
        return struct.unpack_from("<f", data, va - SEGMENT_DELTA)[0]

    def literal(va: int, name: str) -> None:
        offset = va - SEGMENT_DELTA
        encoded = name.encode("ascii") + b"\0"
        assert data[offset:offset + len(encoded)] == encoded, f"literal {name} mismatch"

    def jal(site: int, target: int) -> None:
        instruction = word(site)
        assert (instruction >> 26) == 3, f"missing JAL at {site:x}"
        actual = ((site + 4) & 0xF0000000) | ((instruction & 0x03ffffff) << 2)
        assert actual == target, f"JAL at {site:x} changed: {actual:x} != {target:x}"

    checks = []
    # Original Lua commands construct event objects. These are NOT the
    # game audio driver's playback entrypoints.
    for name, site, constructor in (
        ("se", 0x219264, 0x373264),
        ("se_stop", 0x219138, 0x3732fc),
        ("wait", 0x2193cc, 0x37321c),
        ("add_entity_se", 0x2195a0, 0x3738dc),
    ):
        jal(site, constructor)
        checks.append(f"{name}:0x{site:X}->0x{constructor:X}")
    jal(0x512034, 0x266cf4)
    for va, key in (
        (0x6ECC10, "empty"), (0x6ECC18, "net"), (0x6ECC20, "club"),
        (0x6E0AB8, "name"), (0x6E2D90, "target_name"),
        (0x6E2DF0, "vol"), (0x6E2DF8, "time"),
        (0x6E2E08, "exdb_se_id"), (0x6E2E18, "watch_rollerhandle_name"),
        (0x6E2E30, "sign"), (0x6E2E38, "is_2d"), (0x6E2E40, "is_3d"),
    ):
        literal(va, key)
    assert scalar(0x6E2DF4) == 1.0, "se volume default differs"
    assert scalar(0x6E2E00) == 1.0, "wait duration default differs"
    # Both literals are actually loaded by their retail command parsers.
    assert word(0x219224) == 0xC5D40000, "se f20 volume load changed"
    assert word(0x21939c) == 0xC5F40000, "wait f20 duration load changed"
    checks.extend(["weapon:0x512034->0x266CF4", "se_volume:1.0",
                   "wait_duration:1.0", "verified_literals:12"])
    return checks


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("elf", type=Path, help="private original USA SCUS_975.01")
    arguments = parser.parse_args()
    print("ORIGINAL_INPUT_AUDIO_RETAIL_EVIDENCE_PASSED",
          *audit(arguments.elf))


if __name__ == "__main__":
    main()
