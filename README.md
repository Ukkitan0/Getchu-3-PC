# Getchu-3-PC — Ape Escape 3 native PC decompilation

**Canonical source project.** Target: a native Windows port of the NTSC-U retail game (*Ape Escape 3*, `SCUS_975.01`). The old recompilation experiments are references only. **This source is not a playable Windows game.** No original game executable or assets are distributed in this repository.

## Latest verified checkpoint — October 18, 2026

| Layer | Recovered candidate entrypoints | Verified behavior |
|---|---:|---|
| Scalar/branch/loop leaves | 1,470 | Native C, instruction-sliced execution, original-MIPS comparisons |
| Callers with direct/indirect calls and loops | 176 | Bounded nested JAL/JALR stack, delay slots, checked indirect targets |
| **Unified guest execution interface** | **1,646** | Routes to appropriate executor; preserves PC, registers, RAM, and fault-retry state |
| **EE ELF image loader** | 1 retail PT_LOAD segment | Maps executable byte-for-byte and clears BSS in 32 MB guest RAM |

The guest execution adapter and ELF loader are written in portable C11 (`ae3_guest_execution.c/.h`, `ae3_elf_image.c/.h`). They compile as Windows x64 COFF objects with `clang-cl`, without requiring copyrighted game code in the repository. The Linux integration tests passed using the verified retail executable. The original execution entry at `0x00100134` is **not yet translated**; the adapter explicitly rejects it rather than claiming the game booted.

These 1,646 entries are conservative **candidate** functions, not verified complete-game coverage. We still lack EE full 128-bit/MMI/COP1/VU instructions, sufficient caller/function coverage, EE/IOP/BIOS services, PS2 graphics, audio and disc/game runtime integration.

## Rebuild and verify

Use an unmodified `SCUS_975.01` from your own copy of the NTSC-U game. Required SHA-256:

`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`

The Python generators enforce that SHA. Set the path before running them.

**Linux (Python 3.10+ and a C11 compiler)**

```bash
AE3_ELF=/path/to/SCUS_975.01 bash BUILD_AND_TEST_LINUX.sh
```

This generates private C from the user's own executable, compiles the recovered code, runs independent MIPS differential tests, maps the original ELF to EE guest RAM, then validates the common execution adapter. A full from-scratch rebuild can take several minutes.

**Windows x64 (Python 3, LLVM `clang-cl` / MSVC C++ tools)**

```bat
set AE3_ELF=C:\ApeEscape3\SCUS_975.01
BUILD_WINDOWS_OBJECTS.bat
```

Windows outputs are object files to link into a later native game runtime, **not a runnable game executable**.

## Architecture

- `bulk_lift.py`, `cfg_lift.py`, `loop_lift.py`: conservative leaf recovery; low 64-bit scalar instructions.
- `resume_lift.py`: independent instruction slices with saved PC and MIPS delay state.
- `call_lift.py`, `stack_lift.py`, `extend_stack.py`, `loop_stack.py`: call/return, conditionals, backward edges and whitelisted indirect targets.
- `ae3_native_dispatch.c/h`: legacy per-kind dispatch; `ae3_guest_execution.c/h`: unified host-facing dispatch with a single typed frame.
- `ae3_elf_image.c/h`: checked, transactional ELF32 little-endian MIPS guest-memory mapping.
- `audit_boot_entry.py`: verifies actual initial 29 MMI PADDUW zero clears, four HI/LO clears, and 32 COP1 register clears; does not simulate them.
- `test_guest_execution.c`, `test_elf_image.c`: original-code integration tests; `test_*.py`: differential comparisons with independent MIPS decoding.

The generated C, ELF/ISO, disassembly and game assets stay out of public Git history. See [STATUS.md](STATUS.md) and [CHECKPOINT_GUEST_RUNTIME.md](CHECKPOINT_GUEST_RUNTIME.md) for exact validation and blockers.