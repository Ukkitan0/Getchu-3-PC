#!/usr/bin/env python3
"""Correlate authentic Lua 5.0 globals with verified ELF CreateBind calls.

This is a PRIVATE static evidence report, not a functional game binding list.
The original ELF must match its recorded USA SHA-256. Original scripts
are never copied, decompiled, executed or written to the repository.
"""
from __future__ import annotations
import argparse
import json
from pathlib import Path
from collections import defaultdict
import audit_original_lua50_calls as lua
import map_original_lua_registrations as elf


def reconcile(lua_report: dict, elf_report: dict, host_names: list[str]) -> dict:
    if elf_report.get('original_verified_elf_sha256') != elf.US_SHA256:
        raise ValueError('refusing unverified original ELF registration evidence')
    if lua_report.get('schema') != 'getchu3-original-lua50-global-dependencies/v1':
        raise ValueError('unexpected Lua script dependency report schema')
    # Collate only registrations actually resolved through a verified CreateBind
    # call. Unknown helper families are not assumed to be unregistered.
    sites = defaultdict(set)
    for row in elf_report['registrations']:
        name = row['name']
        address = row['register_jal']
        if not name or not address.startswith('0x'):
            raise ValueError('malformed original ELF registration record')
        sites[name].add(address)
    # Published original constructor vtable locations are separate evidence
    # from actual CreateBind registration calls. A six-word table is only a
    # candidate list, not a confirmed callback implementation address.
    vtables = {}
    for record in (elf_report.get('published_flag_vtable_slot_candidates', []) +
                   elf_report.get('published_launcher_vtable_slot_candidates', [])):
        name = record['name']
        if name in vtables:
            raise ValueError('duplicate original vtable identity')
        candidates = [slot['pointer'] for slot in record.get('slots', [])
                      if slot.get('points_into_original_text')]
        vtables[name] = {'address': record['vtable_address'],
                         'published_symbol': record.get('published_symbol'),
                         'mapped': bool(record['available']),
                         'candidate_text_pointers': sorted(set(candidates))}
    native_names = set(sites)
    host = set(host_names)
    defined = set(lua_report['global_writes'])
    reads = set(lua_report['global_reads'])
    direct = set(lua_report['direct_global_calls'])
    if not direct.issubset(reads):
        raise ValueError('corrupt Lua analysis: direct call missing from global reads')
    # A Lua script can also call library globals, dynamically created globals,
    # or functions from other helpers. Report candidates, not invented APIs.
    rows = []
    for name in sorted(reads | native_names | host | set(vtables)):
        row = {
            'name': name,
            'observed_original_global_load': name in reads,
            'candidate_direct_call': name in direct,
            'defined_by_original_script': name in defined,
            'registered_at_scanned_original_CreateBind': name in native_names,
            'present_in_native_host': name in host,
            'original_register_sites': sorted(sites[name]),
            'published_original_vtable': vtables.get(name),
        }
        if name in direct and name in native_names and name not in host:
            row['priority'] = 'original_direct_call_missing_native_host'
        elif name in direct and name in vtables and name not in host and name not in defined:
            row['priority'] = 'original_direct_call_has_published_vtable_not_bound'
        elif name in direct and name not in native_names and name not in host and name not in defined:
            row['priority'] = 'unresolved_direct_call_requires_additional_source_proof'
        elif name in native_names and name not in host:
            row['priority'] = 'retail_registration_not_yet_in_native_host'
        elif name in direct and name in host:
            row['priority'] = 'present_name_semantics_not_verified'
        else:
            row['priority'] = 'informational'
        rows.append(row)
    return {
        'schema': 'getchu3-original-lua50-binding-evidence/v1',
        'verified_usa_elf_sha256': elf.US_SHA256,
        'retail_scripts_count': len(lua_report['scripts']),
        'original_createbind_names': len(native_names),
        'published_vtable_names': len(vtables),
        'native_host_names': len(host),
        'rows': rows,
        'critical_caveat': 'Opcode reads, one verified ELF registration helper and published virtual-table slots are separate evidence. Slot text pointers are NOT identified native Lua callback implementations. This is NOT proof of behavior, complete registration or script call order.',
    }


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--elf', type=Path, required=True, help='private original USA SCUS_975.01')
    p.add_argument('--scripts', type=Path, required=True, help='private area.luc or extracted stage folder')
    p.add_argument('--host-source', type=Path, required=True, help='native/src/lua_stage.cpp')
    p.add_argument('--out', type=Path, required=True, help='private report JSON file outside git')
    args = p.parse_args()
    registered = elf.inspect(args.elf.read_bytes())
    dependencies = lua.audit(args.scripts, args.host_source)
    report = reconcile(dependencies, registered, lua.host_callbacks(args.host_source))
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    counts = defaultdict(int)
    for row in report['rows']:
        counts[row['priority']] += 1
    print('ORIGINAL_LUA_BINDING_EVIDENCE', json.dumps(dict(counts), sort_keys=True))
    print('Private report:', args.out.resolve())
    return 0


if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except (OSError, ValueError, lua.InvalidChunk) as error:
        raise SystemExit(f'ORIGINAL_LUA_BINDING_EVIDENCE_FAILED: {error}') from error
