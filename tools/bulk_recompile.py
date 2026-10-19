#!/usr/bin/env python3
"""Batch EE executable entry discovery / verified hand-off to ps2-web-recomp.

The opcode inventory is a code *candidate* inventory, not a function map; only
prior recovered direct-call *candidates* plus actual constructor pointers are seeded.
No game bytes, ISO, ELF or translations are written to the public source tree.
"""
from __future__ import annotations
import argparse
from collections import Counter
import csv
import gzip
import hashlib
import json
from pathlib import Path
from derive_elf_inventory import derive
import subprocess
import sys
import tempfile
from native_explore import parse_entries
from aot_cache import compile_cached

ROOT = Path(__file__).resolve().parent.parent
EXPECTED_ELF_SHA = '5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63'
INVENTORY = ROOT / 'analysis/full_elf/ee_text_word_inventory.tsv.gz'
STATIC_TARGETS = ROOT / 'analysis/full_elf/direct_jal_targets.tsv'
CONSTRUCTORS = ROOT / 'retail_constructor_addresses.tsv'
ANCHORS = (0x00100134, 0x0010033c, 0x0041a390, 0x00417c58, 0x00423e20)


def load_inventory(path: Path):
    """Read only the first two fields: 1.35m words, not millions of Python rows."""
    records = {}
    groups = Counter()
    with gzip.open(path, 'rt', encoding='utf-8') as inp:
        head = inp.readline().rstrip('\n').split('\t')
        if head[:2] != ['address', 'word']:
            raise ValueError('Unknown EE word inventory schema')
        for line in inp:
            parts = line.split('\t', 2)
            if len(parts) < 2:
                raise ValueError('Bad EE word row')
            addr = int(parts[0], 16)
            op = int(parts[1], 16)
            if addr & 3 or addr in records:
                raise ValueError(f'Duplicate or unaligned EE address {addr:08x}')
            records[addr] = op
            groups[f'{op >> 26:02x}'] += 1
    return records, groups


def load_calls(path: Path, records: dict[int, int]):
    targets = {}
    with path.open(newline='', encoding='utf-8') as inp:
        for row in csv.DictReader(inp, delimiter='\t'):
            pc = int(row['target'], 16)
            count = int(row['static_sites'])
            if (pc & 3) or count <= 0 or pc not in records:
                raise ValueError(f'Call target outside original code: {pc:08x}')
            if pc in targets:
                raise ValueError(f'Duplicate call target: {pc:08x}')
            targets[pc] = count
    return targets


def load_constructors(path: Path, records: dict[int, int]):
    out = []
    pointers = set()
    with path.open(encoding='utf-8') as inp:
        for line in inp:
            if not line.strip() or line.startswith('#'):
                continue
            pointer, target = [int(x, 16) for x in line.split()[:2]]
            if target == 0xffffffff:  # original retail sentinel, not a callback
                if not out:
                    raise ValueError('Constructor sentinel before all callbacks')
                break
            if pointer in pointers or target & 3 or target not in records:
                raise ValueError(f'Invalid constructor {pointer:08x} -> {target:08x}')
            pointers.add(pointer)
            out.append((pointer, target))
    if not out:
        raise ValueError('Constructor table empty')
    if len({t for _, t in out}) != len(out):
        raise ValueError('Duplicate constructor function pointer')
    return out


def sha256(path: Path):
    h = hashlib.sha256()
    with path.open('rb') as f:
        while chunk := f.read(8 * 1024 * 1024):
            h.update(chunk)
    return h.hexdigest()


def write_outputs(out: Path, records, groups, calls, constructors, seed_mode="constructors", *, reset_entries=False):
    out.mkdir(parents=True, exist_ok=True)
    if seed_mode not in {"constructors", "common", "all"}:
        raise ValueError(f"Unknown seed mode: {seed_mode}")
    # The upstream analyser already walks direct JALs. The safe default only
    # supplies indirect constructor targets; broad scans are opt-in because a
    # JAL-shaped word may appear in data or unreachable executable padding.
    selected_calls = ({pc for pc, count in calls.items() if count >= 2}
                      if seed_mode == "common" else set(calls) if seed_mode == "all"
                      else set())
    seeds = sorted({*selected_calls, *(t for _, t in constructors), *ANCHORS})
    for entry in seeds:
        if entry not in records:
            raise ValueError(f'Entry not present in original game word inventory {entry:08x}')
    entry_file = out / 'entries.txt'
    # Never discard addresses found by earlier native exploration rounds.
    # The compiler accepts bare hex addresses and image-name:offset tokens.
    # Validate stored main-ELF addresses against the same authenticated ELF
    # inventory before preserving them; reject corrupt/stale code instead of
    # silently passing it into the compiler.
    previous = parse_entries(entry_file) if entry_file.exists() and not reset_entries else set()
    for token in previous:
        if ':' not in token and int(token, 16) not in records:
            raise ValueError(f'Existing discovered entry outside original EE inventory: {token}')
    seed_tokens = {f'{x:08x}' for x in seeds}
    combined = sorted(seed_tokens | previous)
    content = ''.join(token + '\n' for token in combined)
    # Atomically update, without touching mtime on no-op rebuilds.
    if not entry_file.is_file() or entry_file.read_text(encoding='ascii') != content:
        with tempfile.NamedTemporaryFile(
                mode='w', dir=out, prefix='.entries-', encoding='ascii',
                newline='\n', delete=False) as fp:
            fp.write(content)
            tmp = Path(fp.name)
        try:
            tmp.replace(entry_file)
        finally:
            tmp.unlink(missing_ok=True)
    (out / 'constructors.tsv').write_text('pointer_address\tguest_target\n' + ''.join(
        f'{address:08x}\t{target:08x}\n' for address, target in constructors), encoding='ascii')
    (out / 'direct_targets.tsv').write_text('target\tsites\n' + ''.join(
        f'{address:08x}\t{count}\n' for address, count in sorted(calls.items())), encoding='ascii')
    report = {
        'format': 'ae3-native-bulk-recomp/v1',
        'input': 'original US retail EE word inventory, constructor table and raw JAL target candidates',
        'expected_elf_sha256': EXPECTED_ELF_SHA,
        'inventory_word_count': len(records),
        'unique_direct_jal_candidate_targets': len(calls),
        'constructor_target_count': len(constructors),
        'constructor_first': f'0x{constructors[0][1]:08x}',
        'total_unique_seed_entries': len(seeds),
        'total_compile_entries': len(combined),
        'preserved_discovered_entries': len(set(combined) - seed_tokens),
        'preserved_module_entries': sum(':' in t for t in combined),
        'entries_reset_explicitly': bool(reset_entries),
        'entry_profile': seed_mode,
        'direct_call_candidates_included_as_seeds': len(selected_calls),
        'all_entries_in_retail_inventory': True,
        'entry_validation': 'addresses are aligned and appear in original EE text; targets in data/unreachable code remain possible',
        'complete_runtime_supported': False,
        'opcode_major_histogram': dict(sorted(groups.items())),
        'note': 'Candidate code entries only. Recompiling them does not prove boot, semantics, BIOS fidelity or graphics.',
    }
    (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    return report, entry_file


def check_elf(path: Path):
    if not path.is_file():
        raise FileNotFoundError(f'Missing original game executable: {path}')
    got = sha256(path)
    if got != EXPECTED_ELF_SHA:
        raise ValueError(f'Incorrect game executable SHA-256: {got}, expected {EXPECTED_ELF_SHA}')
    return got


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--out', type=Path, default=ROOT / 'fastlane/generated', help='Private generated entry files')
    p.add_argument('--seed-mode', choices=('constructors', 'common', 'all'), default='constructors',
                   help='constructors=safest, common=JAL targets with 2+ call sites, all=full raw call target set')
    p.add_argument('--elf', type=Path, help='Owner-supplied original SCUS_975.01 executable')
    p.add_argument('--eerecomp', type=Path, help='Built upstream eerecomp executable: invokes full translation')
    p.add_argument('--cpp-out', type=Path, help='Private destination for translated C++ code')
    p.add_argument('--modules', type=Path, help='Private extracted SNR2 modules from owned ISO')
    p.add_argument('--overlays', type=Path, help='Private captured executable code-window images')
    p.add_argument('--reset-entries', action='store_true',
                   help='Explicitly discard discovered guest code and rebuild seed list (not default)')
    args = p.parse_args(argv)
    if (args.modules or args.overlays) and not args.eerecomp:
        p.error('--modules/--overlays require --eerecomp')
    for directory in (args.modules, args.overlays):
        if directory is not None and not directory.is_dir():
            raise FileNotFoundError(f'Private recompiler input directory missing: {directory}')
    if args.eerecomp and (not args.elf or not args.cpp_out):
        p.error('--eerecomp requires both --elf and --cpp-out')
    if args.elf:
        check_elf(args.elf)
    inv, targets = INVENTORY, STATIC_TARGETS
    if not inv.is_file() or not targets.is_file():
        if not args.elf:
            raise FileNotFoundError('Retail instruction inventory is not in the public repository. Supply --elf with your own original SCUS_975.01; private tables will be reconstructed automatically.')
        private = args.out / 'derived_from_owned_elf'
        print('Generating missing private EE opcode inventory and call-target table from SHA-verified retail ELF',flush=True)
        result = derive(args.elf, private)
        inv, targets = Path(result['inventory']), Path(result['direct_targets'])
    records, groups = load_inventory(inv)
    calls = load_calls(targets, records)
    ctors = load_constructors(CONSTRUCTORS, records)
    report, entries = write_outputs(args.out, records, groups, calls, ctors, args.seed_mode,
                                    reset_entries=args.reset_entries)
    print(json.dumps({k: v for k, v in report.items() if k != 'opcode_major_histogram'}, indent=2))
    if args.eerecomp:
        if not args.eerecomp.is_file():
            raise FileNotFoundError(f'Upstream recompiler not found: {args.eerecomp}')
        args.cpp_out.mkdir(parents=True, exist_ok=True)
        print('Preparing cached native AOT translation from validated retail ELF', flush=True)
        cached = compile_cached(args.eerecomp, args.elf, entries, args.cpp_out,
                                args.modules, args.overlays)
        print('AE3_AOT_TRANSLATION ' + json.dumps(cached), flush=True)
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, ValueError, subprocess.CalledProcessError) as exc:
        print(f'BULK_RECOMP_ERROR: {exc}', file=sys.stderr)
        sys.exit(1)