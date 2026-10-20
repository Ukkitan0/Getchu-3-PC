#!/usr/bin/env python3
"""Derive private R5900 instruction inventory + direct-JAL table from user-owned ELF.

No Ghidra, LLVM, Sony files or game-specific hardcoded instruction ranges needed.
The input executable must be validated by bulk_recompile.check_elf() before this
is used in the production pipeline. These files must NEVER be committed: their
raw instruction words are obtained from the user's copyrighted executable.
"""
from __future__ import annotations
from collections import Counter
import csv
import gzip
import json
import os
from pathlib import Path
import struct
import tempfile

MAX_EXECUTABLE_BYTES = 96 * 1024 * 1024
PT_LOAD = 1
PF_X = 1
EM_MIPS = 8


def read_elf_code(elf: Path) -> dict[int, int]:
    data = elf.read_bytes()
    if len(data) < 52 or data[:4] != b'\x7fELF':
        raise ValueError('Not an ELF executable')
    if data[4] != 1 or data[5] != 1:
        raise ValueError('Expected 32-bit little-endian PS2 ELF')
    (etype, machine, version, entry, phoff, shoff, flags, ehsize,
     phentsize, phnum, shentsize, shnum, shstrndx) = struct.unpack_from('<HHIIIIIHHHHHH', data, 16)
    if etype != 2 or machine != EM_MIPS or version != 1:
        raise ValueError('Expected 32-bit MIPS executable, not other ELF machine/type')
    if phentsize < 32 or phnum < 1 or phnum > 128 or phoff + phnum * phentsize > len(data):
        raise ValueError('Invalid or out-of-bounds ELF program headers')
    words: dict[int, int] = {}
    total_size = 0
    for i in range(phnum):
        (pt, off, addr, paddr, filesz, memsz, prot, align) = struct.unpack_from(
            '<IIIIIIII', data, phoff + i * phentsize)
        if pt != PT_LOAD or not prot & PF_X:
            continue
        if filesz > memsz or off + filesz > len(data) or (addr & 3) or (filesz & 3):
            raise ValueError('Invalid or unaligned executable segment')
        total_size += filesz
        if total_size > MAX_EXECUTABLE_BYTES:
            raise ValueError('Total executable segments exceed safety budget')
        segment = memoryview(data)[off:off + filesz]
        for j in range(0, filesz, 4):
            va = addr + j
            if va in words:
                raise ValueError(f'Overlapping executable segments at 0x{va:08x}')
            words[va] = int.from_bytes(segment[j:j+4], 'little')
    if not words:
        raise ValueError('ELF contains no executable PT_LOAD code')
    if entry not in words:
        raise ValueError(f'ELF entry 0x{entry:08x} is outside all executable segments')
    return words


def direct_jal_targets(words: dict[int, int]) -> Counter:
    # MIPS JAL: opcode 3, 26-bit immediate, PC-region high nibble.
    # Raw word candidates may be unreachable or embedded data, not certified functions.
    counts = Counter()
    for addr, insn in words.items():
        if insn >> 26 != 3:
            continue
        target = ((addr + 4) & 0xf0000000) | ((insn & 0x03ffffff) << 2)
        if target in words:
            counts[target] += 1
    return counts


def derive(elf: Path, out: Path) -> dict:
    words = read_elf_code(elf)
    calls = direct_jal_targets(words)
    out.mkdir(parents=True, exist_ok=True)
    inv = out / 'ee_text_word_inventory.tsv.gz'
    targets = out / 'direct_jal_targets.tsv'
    # Create both files as temp data first. Never partially overwrite an old file
    # because a host filesystem has become full or a source ELF is truncated.
    with tempfile.NamedTemporaryFile(dir=out, suffix='.tsv.gz', delete=False) as f:
        tmpinv = Path(f.name)
    with tempfile.NamedTemporaryFile(dir=out, suffix='.tsv', delete=False) as f:
        tmpcalls = Path(f.name)
    try:
        with gzip.open(tmpinv, 'wt', encoding='ascii', compresslevel=6, newline='\n') as dst:
            dst.write('address\tword\n')
            for addr, op in sorted(words.items()):
                dst.write(f'{addr:08X}\t{op:08X}\n')
        with tmpcalls.open('w', encoding='ascii', newline='\n') as dst:
            dst.write('target\tstatic_sites\n')
            for target, count in sorted(calls.items()):
                dst.write(f'{target:08X}\t{count}\n')
        tmpinv.replace(inv)
        tmpcalls.replace(targets)
    finally:
        tmpinv.unlink(missing_ok=True)
        tmpcalls.unlink(missing_ok=True)
    return {'elf': str(elf), 'inventory': str(inv), 'direct_targets': str(targets),
            'words': len(words), 'unique_direct_JAL_targets': len(calls),
            'original_instruction_addresses': [f'{min(words):08X}', f'{max(words):08X}'],
            'guest_game_data_is_private': True}


def main(argv=None):
    import argparse
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--elf', type=Path, required=True)
    p.add_argument('--out', type=Path, required=True)
    a = p.parse_args(argv)
    # The production Windows launcher uses pinned SHA validation in bulk_recompile.
    # Direct CLI users must validate their version before using resulting tables.
    print(json.dumps(derive(a.elf,a.out),indent=2))


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError) as err:
        print('ELF_INVENTORY_ERROR:', err)
        raise SystemExit(1)