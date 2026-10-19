#!/usr/bin/env python3
"""Game-free, deterministic EE ELF fixture for real native AOT integration testing.

This is original test code, not a retail Ape Escape executable or game asset.
It exercises the upstream R5900 -> C++ -> native ps2run path by executing
LUI/ORI/ADDIU/SW and the genuine PS2 Exit syscall.
"""
from __future__ import annotations
import argparse
from pathlib import Path
import struct

ENTRY = 0x00100100
WRITE_ADDRESS = 0x00012000
WRITE_VALUE = 0x00001234
TEXT_OFFSET = 0x100
# MIPS R5900 little endian, no pseudo-ops. Halt via kernel syscall 0x04.
# Main calls an original direct branch (JAL), whose callee uses a JALR to a
# dynamically addressed callback at 0x00100160. The callback is deliberately
# unreachable by static JAL analysis: it must be explicitly seeded in
# eerecomp's --entries list, exercising the private bulk-code-discovery path.
CALLBACK = ENTRY + 0x60
WORDS = (
    0x3C080001,  # +00 lui   t0,0x0001
    0x35082000,  # +04 ori   t0,t0,0x2000       -> 00012000
    0x2404000A,  # +08 addiu a0,zero,10
    0x0C040050,  # +0c jal   0x00100140
    0x00000000,  # +10 delay slot
    0xAD020000,  # +14 sw    v0,0(t0)
    0x24030004,  # +18 addiu v1,zero,4         -> Exit
    0x0000000C,  # +1c syscall
    0x1000FFFF,  # +20 b . (unreachable fallback)
    0x00000000,  # +24 nop
    *(0 for _ in range(6)),  # pad to +40
    0x03E08021,  # +40 addu s0,ra,zero (save parent return address)
    0x3C190010,  # +44 lui t9,0x0010
    0x37390160,  # +48 ori t9,t9,0x0160
    0x0320F809,  # +4c jalr ra,t9 (dynamic code target)
    0x00000000,  # +50 delay slot
    0x0200F821,  # +54 addu ra,s0,zero
    0x03E00008,  # +58 jr ra
    0x00000000,  # +5c delay slot
    0x2482122A,  # +60 addiu v0,a0,0x122A -> 0x1234
    0x03E00008,  # +64 jr ra
    0x00000000,  # +68 delay slot
)


def write_entry_list(destination: Path) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(f'{CALLBACK:08x}\n', encoding='ascii')



def build_bytes() -> bytes:
    text = struct.pack('<' + 'I' * len(WORDS), *WORDS)
    out = bytearray(TEXT_OFFSET + len(text))
    out[0:16] = b'\x7fELF\x01\x01\x01' + bytes(9)
    # ET_EXEC, EM_MIPS, ELFv1, entry, phdr offset, no section table.
    struct.pack_into('<HHIIIIIHHHHHH', out, 16,
                     2, 8, 1, ENTRY, 52, 0, 0, 52, 32, 1, 0, 0, 0)
    # PT_LOAD, offset, vaddr, paddr, filesz, memsz, executable and readable.
    struct.pack_into('<IIIIIIII', out, 52,
                     1, TEXT_OFFSET, ENTRY, ENTRY, len(text), len(text), 5, 0x1000)
    out[TEXT_OFFSET:] = text
    return bytes(out)


def inspect(data: bytes) -> None:
    if len(data) != TEXT_OFFSET + 4 * len(WORDS):
        raise ValueError('Unexpected ELF fixture size')
    if data[:16] != b'\x7fELF\x01\x01\x01' + bytes(9):
        raise ValueError('Not the expected ELF32 little-endian fixture')
    if struct.unpack_from('<HHIIIIIHHHHHH', data, 16) != (
            2, 8, 1, ENTRY, 52, 0, 0, 52, 32, 1, 0, 0, 0):
        raise ValueError('Wrong ELF header')
    if struct.unpack_from('<IIIIIIII', data, 52) != (
            1, TEXT_OFFSET, ENTRY, ENTRY, 4 * len(WORDS), 4 * len(WORDS), 5, 0x1000):
        raise ValueError('Wrong executable segment')
    if struct.unpack_from('<'+'I'*len(WORDS), data, TEXT_OFFSET) != WORDS:
        raise ValueError('MIPS instruction payload changed')


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--entries', type=Path, help='Write dynamic JALR code-entry seed')
    parser.add_argument('--check', action='store_true', help='Validate existing fixture without rewriting')
    args = parser.parse_args(argv)
    if args.check:
        inspect(args.out.read_bytes())
    else:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        content = build_bytes()
        inspect(content)
        args.out.write_bytes(content)
    if args.entries:
        write_entry_list(args.entries)
    print(f'SYNTHETIC_AOT_FIXTURE_OK {args.out} entry=0x{ENTRY:08X} '
          f'write=0x{WRITE_ADDRESS:08X} value=0x{WRITE_VALUE:08X} '
          f'bytes={len(build_bytes())}')


if __name__ == '__main__':
    main()