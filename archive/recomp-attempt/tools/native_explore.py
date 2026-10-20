#!/usr/bin/env python3
"""Monotonic code-discovery merge for the private Windows native recompilation loop.

Reads eerecomp's missing indirect entry addresses from a diagnostic run.
Rejects malformed entries, validates retail main-text candidates against the
owner-provided opcode inventory, and atomically augments entries.txt.
"""
from __future__ import annotations
import argparse
import gzip
import json
from pathlib import Path
import re
import tempfile

HEX = re.compile(r'^(?:(?P<module>[A-Za-z0-9_.-]+):)?(?:0x)?(?P<address>[a-fA-F0-9]{1,8})$')


def parse_entries(path: Path) -> set[str]:
    if not path.exists():
        return set()
    tokens = set()
    for line in path.read_text(encoding='ascii').splitlines():
        raw = line.split('#', 1)[0].strip()
        if not raw:
            continue
        match = HEX.fullmatch(raw)
        if not match:
            raise ValueError(f'Invalid indirect entry token in {path}: {raw!r}')
        addr = int(match.group('address'), 16)
        if addr & 3:
            raise ValueError(f'Unaligned entry {addr:08x}')
        module = match.group('module')
        tokens.add((module + ':' if module else '') + f'{addr:08x}')
    return tokens


def inventory_addresses(inventory_path: Path) -> set[int]:
    found = set()
    with gzip.open(inventory_path, 'rt', encoding='utf-8') as file:
        header = file.readline().rstrip('\r\n').split('\t')
        if header[:2] != ['address', 'word']:
            raise ValueError('Unknown original retail inventory header')
        for line in file:
            addr = int(line.split('\t', 1)[0], 16)
            found.add(addr)
    return found


def merge_entries(entries: Path, misses: Path, inventory: Path, *, max_added: int = 10000):
    known = parse_entries(entries)
    missing = parse_entries(misses)
    if len(missing) > max_added:
        raise ValueError(f'{len(missing)} missing candidates exceed safety budget {max_added}')
    allowed = inventory_addresses(inventory)
    rejected = []
    for token in missing:
        if ':' not in token and int(token, 16) not in allowed:
            rejected.append(token)
    if rejected:
        raise ValueError(f'Missing entry outside validated retail EE inventory: {sorted(rejected)[:5]}')
    newly_added = sorted(missing - known)
    if newly_added:
        entries.parent.mkdir(parents=True, exist_ok=True)
        all_entries = sorted(known | missing)
        with tempfile.NamedTemporaryFile(mode='w', encoding='ascii', dir=entries.parent, prefix='.entries-', delete=False) as file:
            file.write('\n'.join(all_entries) + '\n')
            tmp = Path(file.name)
        tmp.replace(entries)
    return {'new_entries': len(newly_added), 'total_entries': len(known | missing),
            'main_entries': sum(':' not in t for t in (known | missing)),
            'module_entries': sum(':' in t for t in (known | missing)),
            'new_sample': newly_added[:12], 'changed': bool(newly_added)}


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--entries', type=Path, required=True)
    p.add_argument('--misses', type=Path, required=True)
    p.add_argument('--inventory', type=Path, required=True)
    p.add_argument('--report', type=Path)
    p.add_argument('--max-added', type=int, default=10000)
    a = p.parse_args(argv)
    if a.max_added < 1:
        p.error('--max-added must be positive')
    r = merge_entries(a.entries, a.misses, a.inventory, max_added=a.max_added)
    if a.report:
        a.report.parent.mkdir(parents=True, exist_ok=True)
        a.report.write_text(json.dumps(r, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(r))
    return 0


if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except (OSError, ValueError) as exc:
        print('NATIVE_EXPLORE_ERROR:', exc)
        raise SystemExit(1)