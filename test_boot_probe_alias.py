#!/usr/bin/env python3
"""Smoke check for the original ELF + native EE alias boot diagnostic.

The explicitly selected test continuation uses synthetic SetupThread and
experimental semaphore HLE. No successful BIOS/game boot is claimed.
"""
import os
import re
import subprocess
import sys
from pathlib import Path

elf = Path(os.environ.get("AE3_ELF", sys.argv[1] if len(sys.argv) > 1 else "SCUS_975.01"))
probe = Path("build/ae3_boot_probe")

def run(*flags):
    return subprocess.run([str(probe.resolve()), str(elf.resolve()), *flags],
                          check=True, capture_output=True, text=True).stdout

normal = run()
assert "315897 retired guest instructions" in normal
assert "Stopped correctly: no verified BIOS response supplied" in normal
assert "test-only" not in normal.lower()

alias = run("--test-kernel-fixture", "--test-sema-hle", "--test-ee-alias")
for marker in ("TEST ONLY: running original 0x83 MIPS handler",
               "FindAddress #1 returned 00000000 after 131073 physical RAM reads",
               "FindAddress #2 returned 00000000 after 131073 physical RAM reads",
               "Original code reached 0043FDC4 after both guest handlers",
               "BIOS/kernel was not initialized"):
    assert marker in alias, (marker, alias)
retired = re.search(r"Retired original EE instruction count: (\d+)", alias)
assert retired and int(retired.group(1)) >= 1600000
print("PASS default boot stops at real SetupThread; opt-in alias path executes two original FindAddress searches against uninitialized physical EE RAM")