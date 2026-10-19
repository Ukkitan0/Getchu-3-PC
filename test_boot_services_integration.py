#!/usr/bin/env python3
"""Compare original retail boot continuation with compatibility model vs older test fixture.

Both paths remain NON-AUTHENTIC because EE BIOS initial state is not available.
"""
import os
import re
import subprocess
import sys

if len(sys.argv) != 3:
    raise SystemExit('usage: test_boot_services_integration.py BOOT_PROBE SCUS_975.01')
exe, elf = sys.argv[1:]
def run(*switches):
    result = subprocess.run([exe, elf, *switches], capture_output=True, text=True, check=True, timeout=20)
    return result.stdout
base = run()
assert 'Stopped correctly: no verified BIOS response supplied' in base
assert 'SetupHeap' not in base
old = run('--test-kernel-fixture', '--test-sema-hle', '--test-ee-alias')
new = run('--test-ee-boot-model', '--test-sema-hle', '--test-ee-alias')
assert 'EXPERIMENTAL startup kernel compatibility policy' in new
assert 'Guest stack derived from real syscall arguments: 01FA0000 + 00060000 - 00000010 = 01FFFFF0' in new
assert 'heap recorded: 00772910+00010000' in new
assert 'EXPERIMENTAL kernel registered syscall 83' in new
assert 'EXPERIMENTAL kernel registered syscall 5A' in new
for field in [r'Guest FindAddress #[12] returned ([0-9A-F]{8}) after (\d+) physical RAM reads',
              r'Original code reached ([0-9A-F]{8}) after both guest handlers',
              r'Retired original EE instruction count: (\d+)']:
    a, b = re.findall(field, old), re.findall(field, new)
    assert a == b and a, (field, a, b)
assert old.count('Guest FindAddress #') == 2
print('PASS default boot stops before unresolved authentic SetupThread')
print('PASS guest-derived compatibility thread/heap model reaches both original guest handlers')
print('PASS modeled execution and isolated legacy test fixture produce identical FindAddress results and instruction counts')
print('NOTE compatibility stack-top reserve=16 is policy, not a verified PS2 BIOS value')