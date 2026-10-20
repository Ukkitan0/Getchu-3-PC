# Getchu-3-PC: retail boot chain + full-executable word inventory

**Date:** 2026-10-18. **Status: NOT A PLAYABLE GAME.**

## Directly executed retail code (not a simulated splash screen)

- `ae3_boot_prefix.c`: executes the original game instructions from `0x00100134` to `0x001002C4`, including 29 128-bit GPR clears, FPU state reset and its low-RAM/BSS clear.
- `ae3_boot_memory_fill.c`: executes the real first `JAL` callee at `0x001036D8`, with the `JAL` delay slot, up to return to `0x001002CC`. This code reads a 16-byte stack-guard pattern from EE RAM `0x006B8120` and fills upper EE RAM from `0x01FA0000` through `0x01FFFFFF` in 24,576 128-bit stores. This is **normal RAM**, not an MMIO device.
- The embedded 16-byte pattern starts with ASCII `sTaCkChEcKmAgIc` and ends in zero. Tests compare every written byte to the original pattern and verify that guest RAM outside the destination region stays unchanged.
- `ae3_boot_thread_setup.c`: continues the real instructions from `0x001002CC` through `0x001002F8`, stopping **before** `0x001002FC`, the `SetupThread` syscall number `0x3C`. The original arguments are read from guest registers, NOT fabricated: `gp=0x00734070`, stack= `0x01FA0000`, stack_size=`0x00060000`, args=`0x0072D6C0`, root_func=`0x00100388`.
- The boot run (first two modules) retires **315,885** instructions. The setup register sequence retires another **12**, totaling **315,897 guest instructions**, before the first kernel syscall. No syscall was executed or stubbed.
- The Linux tests passed, including AddressSanitizer and UndefinedBehaviorSanitizer; both new modules cross-compiled as Windows x64 COFF objects with `clang-cl /W4 /WX`.

## Full executable static inventory, without claiming full C decompilation

`full_elf_inventory.py` produces from the owner's SHA-verified original ELF:

- A compressed **one-row-per-word disassembly inventory** for every one of the **1,351,830** aligned words in `.text`, including words omitted by LLVM as repeated NOPs.
- Separate **3,200** 64-bit VU microcode packets from `.vutext`; these must not be treated as EE opcodes.
- An address-indexed static JAL target list and aggregate instruction/unknown-opcode metrics.

This is *static binary coverage*, not a verified decompiler, reachable-code map, working PS2 runtime or playable executable. `llvm-objdump` does not decode all R5900 MMI/VU/COP instructions; inline data inside `.text` also requires careful analysis. Analysis output is for local/private use and excluded from the public git repository.

## Next boot blocker

The guest reaches the real EE kernel syscall `SetupThread` (`$v1=0x3c`) at `0x001002FC`. A faithful EE kernel/BIOS service implementation must return the proper guest stack pointer and maintain thread state. Next is `SetupHeap` (`0x3D`) at `0x00100318` and game initialization calls. Stubbed arbitrary return values are not acceptable. VU/GIF/GS, IOP, input, sound and filesystem/disc support are also required for a playable Windows game.

## Commands

```bash
AE3_ELF=/path/to/SCUS_975.01 bash BUILD_AND_TEST_LINUX.sh
python full_elf_inventory.py /path/to/SCUS_975.01 --out analysis/full_elf
```

**Do not commit or redistribute** the user's retail ELF/ISO, generated per-word disassembly or game assets to the public repository.