#!/usr/bin/env python3
"""Game-free stress fixture: 143 real MIPS indirect calls and their returns.

Creates an ELF32 MIPS PT_LOAD segment with 143 short original test callbacks
and a pointer table. All callback entries must be translated, then a native
runtime must execute every JALR and the final guest Store+Exit syscall.
This intentionally uses NO game-derived source or assets.
"""
from __future__ import annotations
import argparse
from pathlib import Path
import struct

ENTRY = 0x00100100
CALLBACK_BASE = 0x00100200
POINTER_TABLE = 0x00102000
TARGET = 0x00012000
COUNT = 143
FILE_OFFSET = 0x100
# R5900, all delay slots preserved. The table terminates with a NULL word.
MAIN = (
    0x3C120010,  # lui s2,0x0010
    0x36522000,  # ori s2,s2,0x2000 (pointer table)
    0x00008821,  # addu s1,zero,zero (counter)
    0x8E590000,  # loop: lw t9,0(s2)
    0x13200005,  # beq t9,zero,done
    0x26520004,  # delay: addiu s2,s2,4 (always advance)
    0x0320F809,  # jalr ra,t9
    0x00000000,  # delay: nop
    0x1000FFFA,  # b loop
    0x00000000,  # delay: nop
    0x3C080001,  # done: lui t0,0x0001
    0x35082000,  # ori t0,t0,0x2000
    0xAD110000,  # sw s1,0(t0)
    0x24030004,  # addiu v1,zero,4 (Exit)
    0x0000000C,  # syscall
    0x1000FFFF,  # b forever if Exit ignored
    0x00000000,  # nop
)
CALLBACK = (0x26310001, 0x03E00008, 0x00000000)  # addiu s1,s1,1 ; jr ra ; nop


def callback_addresses() -> tuple[int, ...]:
    return tuple(CALLBACK_BASE + 12 * i for i in range(COUNT))


def build_bytes() -> bytes:
    size = POINTER_TABLE - ENTRY + 4 * (COUNT + 1)
    text = bytearray(size)
    struct.pack_into('<' + 'I'*len(MAIN), text, 0, *MAIN)
    for i, addr in enumerate(callback_addresses()):
        struct.pack_into('<III', text, addr - ENTRY, *CALLBACK)
        struct.pack_into('<I', text, POINTER_TABLE - ENTRY + 4*i, addr)
    struct.pack_into('<I', text, POINTER_TABLE - ENTRY + 4*COUNT, 0)
    out = bytearray(FILE_OFFSET + len(text))
    out[:16] = b'\x7fELF\x01\x01\x01' + bytes(9)
    struct.pack_into('<HHIIIIIHHHHHH', out, 16, 2, 8, 1, ENTRY, 52, 0,
                     0, 52, 32, 1, 0, 0, 0)
    struct.pack_into('<IIIIIIII', out, 52, 1, FILE_OFFSET, ENTRY, ENTRY,
                     len(text), len(text), 5, 0x1000)
    out[FILE_OFFSET:] = text
    return bytes(out)


def check(data: bytes) -> None:
    if data != build_bytes():
        raise ValueError('Synthetic constructor ELF fixture mismatch')


def entries_file(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(''.join(f'{x:08x}\n' for x in callback_addresses()), encoding='ascii')


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--out', type=Path, required=True)
    p.add_argument('--entries', type=Path)
    p.add_argument('--check', action='store_true')
    a = p.parse_args(argv)
    if a.check:
        check(a.out.read_bytes())
    else:
        a.out.parent.mkdir(parents=True, exist_ok=True)
        a.out.write_bytes(build_bytes())
    if a.entries:
        entries_file(a.entries)
    print(f'SYNTHETIC_143_CONSTRUCTORS_OK code-seeds={COUNT} bytes={len(build_bytes())} '
          f'expected-ram={TARGET:08x}:{COUNT:08x}')


if __name__ == '__main__':
    main()