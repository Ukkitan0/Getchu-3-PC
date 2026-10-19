#!/usr/bin/env python3
"""Check pinned retail MIPS instructions against an OWNER-SUPPLIED 32-MiB EE RAM dump.

Uses a game save-state RAM snapshot as secondary code evidence, NOT as BIOS
state or as proof that the game was natively booted. No game bytes are bundled.
Obtain the .bin locally with extract_pcsx2_ee_reference.py when necessary.
"""
import argparse
import struct
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("snapshot", type=Path)
    parser.add_argument("--directory", type=Path, default=Path(__file__).resolve().parent)
    args = parser.parse_args()
    image = args.snapshot.read_bytes()
    if len(image) != 32 * 1024 * 1024:
        parser.error("snapshot must contain exactly 32 MiB of eeMemory.bin")
    count = 0
    failures = []
    for path in sorted(args.directory.glob("retail_*_words.tsv")):
        local_count = 0
        for number, line in enumerate(path.read_text().splitlines(), start=1):
            row = line.strip()
            if not row or row.startswith("#"):
                continue
            try:
                address_text, expected_text = row.split()
                address, expected = int(address_text, 16), int(expected_text, 16)
            except (ValueError, TypeError):
                failures.append(f"{path.name}:{number}: bad opcode row")
                continue
            if address & 3 or address + 4 > len(image):
                failures.append(f"{path.name}:{number}: unmapped/unaligned {address:#x}")
                continue
            actual = struct.unpack_from("<I", image, address)[0]
            if actual != expected:
                failures.append(f"{path.name}:{number}: PC {address:08X} expected {expected:08X}, actual {actual:08X}")
            count += 1
            local_count += 1
        print(f"{path.name}: {local_count} pinned guest instructions checked")
    for failure in failures[:30]:
        print(f"FAIL {failure}")
    if failures:
        raise SystemExit(f"FAILED: {len(failures)} mismatch or malformed rows")
    print(f"PASS: {count} pinned MIPS instruction bytes match the user-supplied EE snapshot")


if __name__ == "__main__":
    main()
