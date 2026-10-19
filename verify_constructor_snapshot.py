#!/usr/bin/env python3
"""Audit constructor-table address evidence against an owner-supplied EE RAM image.

This is a read-only verification of AFTER-BOOT snapshot bytes, NOT proof
of the original BIOS-time RAM contents, constructor side effects, or a boot.
"""
import argparse
import struct
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('snapshot', type=Path)
    parser.add_argument('--directory', type=Path, default=Path(__file__).resolve().parent)
    args = parser.parse_args()
    ram = args.snapshot.read_bytes()
    if len(ram) != 32 * 1024 * 1024:
        parser.error('expected a 32 MiB uncompressed EE RAM image')
    evidence = args.directory / 'retail_constructor_addresses.tsv'
    rows = []
    for i, line in enumerate(evidence.read_text().splitlines(), 1):
        if not line.strip() or line.startswith('#'):
            continue
        parts = line.split()
        if len(parts) != 2:
            raise SystemExit(f'invalid constructor table row {i}')
        rows.append(tuple(int(part, 16) for part in parts))
    assert len(rows) == 144 and rows[-1] == (0x0062E660, 0xFFFFFFFF)
    entries = []
    for i, (addr, ptr) in enumerate(rows):
        if addr != 0x0062E89C - i * 4 or addr & 3:
            raise SystemExit(f'invalid entry location {addr:08X}')
        if ptr != struct.unpack_from('<I', ram, addr)[0]:
            raise SystemExit(f'constructor evidence mismatch at {addr:08X}')
        if ptr == 0xFFFFFFFF:
            assert i == len(rows) - 1
            continue
        if not 0x00100000 <= ptr < 0x02000000 or ptr & 3:
            raise SystemExit(f'bad guest callback at row {i}: {ptr:08X}')
        word = struct.unpack_from('<I', ram, ptr)[0]
        if word == 0:
            raise SystemExit(f'zero first guest instruction at {ptr:08X}')
        entries.append(ptr)
    if len(entries) != 143 or len(set(entries)) != len(entries):
        raise SystemExit('expected 143 unique MIPS constructor entry points')
    print(f'PASS: {len(entries)} distinct mapped constructor entry points, first={entries[0]:08X}, last={entries[-1]:08X}')
    print('PASS: 0xFFFFFFFF table terminator exactly at 0062E660; no bad/zero initial words')
    print('Reference is a later-game EE RAM image, not a verified fresh-boot RAM state.')


if __name__ == '__main__':
    main()