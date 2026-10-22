#!/usr/bin/env python3
"""Statically inventory verified SCUS_975.01 calls to CreateBind (0x255BD8).

No guest-code execution. Only a known concrete MIPS instruction chain (producer
of the a1 string pointer, optional materializing delay slot) is accepted.
Original binary and reports stay outside the repository.
"""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import struct

US_SHA256 = '5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
TEXT_START = 0x100000
TEXT_BYTES = 0x528258
REGISTRY_HELPER = 0x255BD8
MAX_LOOKBACK = 64  # bytes, not instructions


def signed16(value: int) -> int:
    value &= 0xFFFF
    return value - 0x10000 if value & 0x8000 else value


def segments_from_elf(blob: bytes) -> list[tuple[int, int, int]]:
    if len(blob) < 52 or blob[:6] != b'\x7fELF\x01\x01':
        raise ValueError('Not ELF32 little-endian')
    if struct.unpack_from('<H', blob, 18)[0] != 8:
        raise ValueError('ELF is not MIPS R3000/R5900')
    phoff = struct.unpack_from('<I', blob, 28)[0]
    entrysize, count = struct.unpack_from('<HH', blob, 42)
    if entrysize < 32 or count > 128 or phoff > len(blob) or entrysize * count > len(blob) - phoff:
        raise ValueError('Invalid ELF program-header table')
    result = []
    for i in range(count):
        p_type, offset, vaddr, _, size, _, _, _ = struct.unpack_from('<8I', blob, phoff + entrysize * i)
        if p_type != 1:
            continue
        if offset > len(blob) or size > len(blob) - offset or vaddr + size > 0x100000000:
            raise ValueError('Invalid ELF PT_LOAD file mapping')
        result.append((vaddr, offset, size))
    if not result:
        raise ValueError('Original executable has no mapped ELF program segments')
    return result


def read_va(blob: bytes, segments: list[tuple[int, int, int]], address: int, length: int) -> bytes | None:
    for va, offset, size in segments:
        if va <= address and length <= size - (address - va):
            start = offset + address - va
            return blob[start:start + length]
    return None


def read_name(blob: bytes, segments: list[tuple[int, int, int]], va: int) -> str | None:
    # One byte at a time: a valid name may be at the very end of PT_LOAD.
    result = bytearray()
    for i in range(101):
        part = read_va(blob, segments, va + i, 1)
        if not part:
            return None
        if part == b'\x00':
            break
        if not (32 <= part[0] <= 126):
            return None
        result.extend(part)
    else:
        return None
    if not result:
        return None
    return result.decode('ascii')


def call_target(pc: int, instruction: int) -> int | None:
    if instruction >> 26 != 3:
        return None
    return ((pc + 4) & 0xF0000000) | ((instruction & 0x03FFFFFF) << 2)


def is_branch(word: int) -> bool:
    op = word >> 26
    return op in (1, 2, 3, 4, 5, 6, 7, 20, 21, 22, 23) or (op == 0 and (word & 63) in (8, 9))


def producer(regs: dict[int, tuple[int, int]], word: int, at: int) -> None:
    """Conservative MIPS32 constant provenance, not an instruction emulator.

    Every instruction which could overwrite a modeled destination must kill
    its old value unless it is one of the exact literal constructors here.
    In particular ANDI, XORI, SLTI, DADDIU and loads must not preserve an
    unrelated earlier string pointer in $a1.
    """
    op = word >> 26
    rs, rt, rd = (word >> 21) & 31, (word >> 16) & 31, (word >> 11) & 31
    imm = word & 0xFFFF
    if op == 15:  # LUI: target is invalidated even for noncanonical rs
        regs.pop(rt, None)
        if rt and rs == 0:
            regs[rt] = (imm << 16, at)
    elif op in (8, 9, 13):  # ADDI / ADDIU / ORI
        src = regs.get(rs) if rs else (0, at)
        regs.pop(rt, None)
        if rt and src is not None:
            value = ((src[0] | imm) if op == 13 else
                     (src[0] + signed16(imm))) & 0xFFFFFFFF
            regs[rt] = (value, src[1])
    elif op == 0:  # SPECIAL R-type instructions
        fn = word & 63
        if fn in (33, 37) and (rs == 0 or rt == 0):
            source = rs if rt == 0 else rt
            known = regs.get(source) if source else (0, at)
            regs.pop(rd, None)
            if rd and known is not None:
                regs[rd] = known
        elif fn not in (8, 9, 12, 13):
            # Conservative for arithmetic, shifts, MFHI/MFLO etc.
            regs.pop(rd, None)
    elif op in (40, 41, 42, 43, 44, 45, 46, 47, 56, 57, 61, 63):
        # stores do not overwrite their source integer registers
        return
    elif op in (16, 17, 18, 19, 28, 31):
        # COP0/COP1/COP2/SPECIAL2/SPECIAL3 can expose register transfers
        # outside this small proof. Forget all pending values.
        regs.clear()
    elif op not in (2, 3, 4, 5, 6, 7, 20, 21, 22, 23):
        # Immediate arithmetic, shifts, loads, special, etc. Unknowns may
        # target rt; losing provenance is preferable to a false match.
        regs.pop(rt, None)


def scan_calls(blob: bytes, segments: list[tuple[int, int, int]],
               text_start: int = TEXT_START, text_bytes: int = TEXT_BYTES,
               helper: int = REGISTRY_HELPER) -> list[dict]:
    code = read_va(blob, segments, text_start, text_bytes)
    if code is None or len(code) % 4:
        raise ValueError('Original executable missing complete .text range')
    words = struct.unpack('<' + 'I' * (len(code) // 4), code)
    calls = []
    regs: dict[int, tuple[int, int]] = {}
    for i, word in enumerate(words):
        pc = text_start + i * 4
        target = call_target(pc, word)
        if target is not None:
            # MIPS JAL's delay instruction executes before entering callee.
            after_delay = dict(regs)
            if i + 1 < len(words):
                delay_word = words[i + 1]
                if is_branch(delay_word):
                    after_delay = {}  # unpredictable branch in delay slot
                else:
                    producer(after_delay, delay_word, pc + 4)
            if target == helper and 5 in after_delay:
                address, originating_pc = after_delay[5]
                if pc - originating_pc <= MAX_LOOKBACK:
                    name = read_name(blob, segments, address)
                    if name:
                        calls.append({
                            'name': name,
                            'register_jal': f'0x{pc:08X}',
                            'game_string_address': f'0x{address:08X}',
                            'name_producer_pc': f'0x{originating_pc:08X}',
                            'game_register_helper': f'0x{helper:08X}',
                        })
            # On return, MIPS caller-saved regs (v0-v1,a0-a3,t0-t9)
            # are unknown. Preserve s0-s7 and gp/sp/fp only.
            regs = {reg: val for reg, val in after_delay.items()
                    if reg in (*range(16, 24), 28, 29, 30)}
            continue
        if is_branch(word):
            regs.clear()  # do not invent register values across control flow
            continue
        # Delay slots are handled by the owning branch/call, not as an
        # unconditional instruction in the fallthrough basic block.
        if i and is_branch(words[i - 1]):
            continue
        producer(regs, word, pc)
    return calls


# Published USA SetupBindFlag C++ constructor assignments, NOT proof of the
# virtual-call slot or method body. Treat each table only as six raw pointers.
# Evidence: Byaidu, "PlayStation 2 Game Reverse Engineering: Ape Escape 3",
# SetupBindFlag() pseudocode (https://www.cnblogs.com/algonote/p/15678057.html).
FLAG_VTABLES = {
    'gw_set_flag': 0x6B70F0,
    'gw_get_flag': 0x6B70D8,
    'gw_get_string': 0x6B70C0,
    'lw_set_flag': 0x6B7090,
    'lw_get_flag': 0x6B7078,
    'area_set_flag': 0x6B7060,
    'area_get_flag': 0x6B7048,
    'get_costume_possess': 0x6B7018,
    'get_costume_possess_ninja': 0x6B7000,
    'stage_var_get_int': 0x6B70A8,
    'is_survival_mode': 0x6B7030,
}


# Published retail SetupBindLauncher pseudocode (USA executable VA 0x21A93C).
# Symbolic aliases have no numeric virtual-table address in that excerpt:
# keep the original symbol and never fill in a fabricated pointer.
# Source: https://www.cnblogs.com/algonote/p/15678057.html
PUBLISHED_LAUNCHER_VTABLES = {
    'BindTester': (0x6ADF78, None),
    'createFeatureLauncher': (None, 's_SetSelectFortune'),
    'createWarpGateLauncher': (None, 's_SetSelectStage'),
    'createTutorialExit': (None, 's_SetSelectTutorial'),
    'createMGSGenerator': (0x6ADF00, None),
    'LocaleInfo_getID': (0x6ADF18, None),
}


def launcher_vtable_candidates(blob: bytes,
                               segments: list[tuple[int, int, int]]) -> list[dict]:
    result = []
    for name, (va, symbol) in sorted(PUBLISHED_LAUNCHER_VTABLES.items()):
        raw = read_va(blob, segments, va, 24) if va is not None else None
        slots = struct.unpack('<6I', raw) if raw is not None else ()
        result.append({
            'name': name,
            'vtable_address': f'0x{va:08X}' if va is not None else None,
            'published_symbol': symbol,
            'available': raw is not None,
            'slots': [{'index': i, 'pointer': f'0x{pointer:08X}',
                       'points_into_original_text':
                           pointer % 4 == 0 and
                           TEXT_START <= pointer < TEXT_START + TEXT_BYTES}
                      for i, pointer in enumerate(slots)],
        })
    return result


def flag_vtable_candidates(blob: bytes, segments: list[tuple[int, int, int]]) -> list[dict]:
    result = []
    for name, address in sorted(FLAG_VTABLES.items()):
        raw = read_va(blob, segments, address, 24)
        if raw is None:
            result.append({'name': name, 'vtable_address': f'0x{address:08X}',
                           'available': False, 'slots': []})
            continue
        pointers = struct.unpack('<6I', raw)
        result.append({
            'name': name,
            'vtable_address': f'0x{address:08X}',
            'available': True,
            'slots': [{'index': i, 'pointer': f'0x{va:08X}',
                       'points_into_original_text':
                           va % 4 == 0 and TEXT_START <= va < TEXT_START + TEXT_BYTES}
                      for i, va in enumerate(pointers)],
        })
    return result


def inspect(blob: bytes) -> dict:
    sha = hashlib.sha256(blob).hexdigest()
    if sha != US_SHA256:
        raise ValueError('Unknown game ELF: only verified USA SCUS_975.01 accepted')
    segments = segments_from_elf(blob)
    registrations = scan_calls(blob, segments)
    flag_tables = flag_vtable_candidates(blob, segments)
    launcher_tables = launcher_vtable_candidates(blob, segments)
    return {
        'original_verified_elf_sha256': sha,
        'source': 'Static SCUS_975.01 MIPS instruction data; no guest execution',
        'api_names_found': len(registrations),
        'registrations': registrations,
        'published_flag_vtable_slot_candidates': flag_tables,
        'published_launcher_vtable_slot_candidates': launcher_tables,
        'limits': 'Only a1 literal construction within 64 bytes of calls to 0x255BD8; '
                  'indirect calls and other registration helpers are intentionally omitted. '
                  'Published flag vtable addresses are treated as six raw 32-bit pointer '
                  'slots; slot purpose and callback function entry are NOT inferred.',
    }


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('original_elf', type=Path)
    p.add_argument('--out', type=Path, help='Write owner-private report outside repository')
    args = p.parse_args()
    result = inspect(args.original_elf.read_bytes())
    print('ORIGINAL_GAME_LUA_REGISTRATIONS', result['api_names_found'])
    for item in result['registrations']:
        print(item['register_jal'], item['name'], item['game_string_address'])
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
