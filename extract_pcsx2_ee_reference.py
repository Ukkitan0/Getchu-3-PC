#!/usr/bin/env python3
"""Extract EE RAM from an OWNED PCSX2 .p2s file for optional diagnostics.

PCSX2 save states often store eeMemory.bin with ZIP compression method 93
(zstd). CPython's zipfile cannot yet read it on some installations; this tool
uses installed `zstd` for that method. State snapshots are *post-boot evidence*,
never replacements for original BIOS behavior. Do not commit extracted bytes.
"""
import argparse
import shutil
import struct
import subprocess
import zipfile
from pathlib import Path


def extract(source: Path, output: Path):
    raw = source.read_bytes()
    with zipfile.ZipFile(source) as z:
        entry = z.getinfo("eeMemory.bin")
        if entry.compress_type == 93:
            if not shutil.which("zstd"):
                raise RuntimeError("ZIP method 93 needs zstd executable on PATH")
            off = entry.header_offset
            if off + 30 > len(raw):
                raise RuntimeError("truncated local ZIP header")
            sig, *fields = struct.unpack_from("<IHHHHHIIIHH", raw, off)
            if sig != 0x04034B50:
                raise RuntimeError("invalid local ZIP entry")
            start = off + 30 + fields[-2] + fields[-1]
            end = start + entry.compress_size
            if end > len(raw):
                raise RuntimeError("truncated compressed entry")
            done = subprocess.run(["zstd", "-dc"], input=raw[start:end],
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                  check=True)
            memory = done.stdout
        else:
            memory = z.read(entry)
    if len(memory) != 0x02000000:
        raise RuntimeError(f"expected 32 MiB EE RAM, got {len(memory)} bytes")
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(memory)
    print(f"Wrote 32 MiB EE snapshot to {output}; do not commit or redistribute it.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("savestate", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    extract(args.savestate, args.output)