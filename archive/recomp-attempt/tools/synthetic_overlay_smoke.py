#!/usr/bin/env python3
"""Game-free native AOT fixture that unpacks and executes an ELF code window.

First run must miss an indirect call to its runtime-generated code. The normal
upstream PS2_DUMP/windows.py capture/recompile pipeline then resolves it.
"""
from __future__ import annotations
import argparse
from pathlib import Path
import struct

ENTRY = 0x00100100
WINDOW = 0x00110000
TARGET = 0x00012000
OFFSET = 0x100
# The relocated guest window contains:
# addiu v0,zero,143; jr ra; nop. Runtime code writes the first two words.
WORDS = (
    0x3C080011,  # lui t0,0x0011 -> 00110000
    0x3C092402,  # lui t1,0x2402
    0x3529008F,  # ori t1,t1,0x008f -> addiu v0,zero,143
    0xAD090000,  # sw t1,0(t0)
    0x3C0903E0,  # lui t1,0x03e0
    0x35290008,  # ori t1,t1,8 -> jr ra
    0xAD090004,  # sw t1,4(t0)
    0x0100F809,  # jalr ra,t0
    0x00000000,  # delay
    0x3C0A0001,  # lui t2,0x0001
    0x354A2000,  # ori t2,t2,0x2000
    0xAD420000,  # sw v0,0(t2)
    0x24030004,  # addiu v1,zero,4
    0x0000000C,  # syscall
    0x1000FFFF,  # should not return here
    0x00000000,  # delay
)

def build_bytes() -> bytes:
    program = struct.pack('<' + 'I' * len(WORDS), *WORDS)
    data = bytearray(OFFSET + len(program))
    data[0:16] = b'\x7fELF\x01\x01\x01' + bytes(9)
    struct.pack_into('<HHIIIIIHHHHHH',data,16,
                     2,8,1,ENTRY,52,0,0,52,32,2,0,0,0)
    struct.pack_into('<8I',data,52,1,OFFSET,ENTRY,ENTRY,
                     len(program),len(program),5,0x1000)
    struct.pack_into('<8I',data,84,1,0,WINDOW,WINDOW,
                     0,0x4000,5,0x4000)
    data[OFFSET:] = program
    return bytes(data)

def inspect(data: bytes) -> None:
    if data != build_bytes():
        raise ValueError('Synthetic unpacked-code ELF fixture mismatch')

def main(argv=None) -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--out', type=Path, required=True)
    p.add_argument('--check', action='store_true')
    a = p.parse_args(argv)
    if a.check:
        inspect(a.out.read_bytes())
    else:
        a.out.parent.mkdir(parents=True,exist_ok=True)
        a.out.write_bytes(build_bytes())
    print(f'SYNTHETIC_CODE_WINDOW_OK entry={ENTRY:08x} window={WINDOW:08x} '
          f'expected-store={TARGET:08x}:0000008f')
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
