#!/usr/bin/env python3
"""Original, game-free PS2 R5900 fixture for nonlocal AOT code and RAM dispatch.

143 indirectly reached callbacks span seven 16-KiB translation regions. Every
callback writes and reads a distinct guest RAM slot across several 4-KiB pages.
The final value depends on completing the whole chain. No retail data included.
"""
from __future__ import annotations
import argparse
from pathlib import Path
import struct

ENTRY = 0x00100100
CALLBACK_BASE = 0x00104000
CALLBACK_STRIDE = 0x100
POINTER_TABLE = 0x00110000
COUNT = 143
TARGET = 0x00012000
SCRATCH_BASE = 0x00013000
SCRATCH_STRIDE = 64
FILE_OFFSET = 0x100

# lw t9 from table, stop on null; each JALR returns and advances to next.
# The branch delay slot advances s2 even for the terminal null pointer.
MAIN = (
    0x3C120011,  # lui s2,0x0011: pointer table high half
    0x36520000,  # ori s2,s2,0
    0x00008821,  # addu s1,zero,zero
    0x8E590000,  # loop: lw t9,0(s2)
    0x13200005,  # beq t9,zero,done
    0x26520004,  # delay: addiu s2,s2,4
    0x0320F809,  # jalr ra,t9
    0x00000000,  # delay
    0x1000FFFA,  # branch back to loop
    0x00000000,  # delay
    0x3C080001,  # done: lui t0,0x0001
    0x35082000,  # ori t0,t0,0x2000
    0xAD110000,  # sw s1,0(t0)
    0x24030004,  # v1=Exit
    0x0000000C,  # syscall
    0x1000FFFF,  # unexpected syscall return: loop
    0x00000000,
)

def callback_addresses() -> tuple[int, ...]:
    return tuple(CALLBACK_BASE + CALLBACK_STRIDE * i for i in range(COUNT))

def scratch_address(index: int) -> int:
    if not 0 <= index < COUNT:
        raise ValueError('callback index out of range')
    return SCRATCH_BASE + index * SCRATCH_STRIDE

def callback_words(index: int) -> tuple[int, ...]:
    # Every distinct callback:
    #   t0 = SCRATCH_BASE + index * STRIDE
    #   s1++, store s1, reload s1, return
    # This makes the last counter sensitive to every guest RAM read and write.
    offset = index * SCRATCH_STRIDE
    if offset >= 0x8000:
        raise ValueError('callback RAM offset exceeds ADDIU immediate')
    return (
        0x3C080001,          # lui t0,0x0001
        0x35083000,          # ori t0,t0,0x3000
        0x25080000 | offset, # addiu t0,t0,offset
        0x26310001,          # addiu s1,s1,1
        0xAD110000,          # sw s1,0(t0)
        0x8D110000,          # lw s1,0(t0)
        0x03E00008,          # jr ra
        0x00000000,          # delay slot
    )

def build_bytes() -> bytes:
    code_size = POINTER_TABLE - ENTRY + 4 * (COUNT + 1)
    text = bytearray(code_size)
    struct.pack_into('<' + 'I' * len(MAIN), text, 0, *MAIN)
    for i, address in enumerate(callback_addresses()):
        struct.pack_into('<8I', text, address - ENTRY, *callback_words(i))
        struct.pack_into('<I', text, POINTER_TABLE - ENTRY + i * 4, address)
    struct.pack_into('<I', text, POINTER_TABLE - ENTRY + COUNT * 4, 0)
    result = bytearray(FILE_OFFSET + len(text))
    result[:16] = b'\x7fELF\x01\x01\x01' + bytes(9)
    struct.pack_into('<HHIIIIIHHHHHH', result, 16, 2, 8, 1, ENTRY, 52,
                     0, 0, 52, 32, 1, 0, 0, 0)
    struct.pack_into('<IIIIIIII', result, 52, 1, FILE_OFFSET, ENTRY, ENTRY,
                     len(text), len(text), 5, 0x1000)
    result[FILE_OFFSET:] = text
    return bytes(result)

def check(data: bytes) -> None:
    if data != build_bytes():
        raise ValueError('Synthetic nonlocal AOT ELF fixture mismatch')

def entries_file(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(''.join(f'{a:08x}\n' for a in callback_addresses()),
                    encoding='ascii')

def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--entries', type=Path)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args(argv)
    if args.check:
        check(args.out.read_bytes())
    else:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_bytes(build_bytes())
    if args.entries:
        entries_file(args.entries)
    print(f'SYNTHETIC_NONLOCAL_AOT_OK callbacks={COUNT} '
          f'code-start={CALLBACK_BASE:08x} code-last={callback_addresses()[-1]:08x} '
          f'ram-last={scratch_address(COUNT-1):08x} expected={COUNT:08x}')
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
