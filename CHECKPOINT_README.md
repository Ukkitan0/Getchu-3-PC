# Getchu-3-PC — Resumable Native C checkpoint (2026-10-18)

This is a verified source-recovery milestone, NOT a playable Windows game.

GitHub canonical source: https://github.com/Ukkitan0/Getchu-3-PC

## What's new

`resume_lift.py` generates `ae3_resumable_leaves.c/.h`: C functions for 1,470
candidate MIPS R5900 scalar routines, with execution paused and resumed at
individual guest instruction boundaries, even between a branch and its delay
slot. `AE3_ResumeFrame` persists guest PC, pending next PC, branch-slot phase,
finished state, and retired instructions.

`ae3_native_dispatch.c/.h` now exports `ae3_native_frame_init()` and
`ae3_run_native_slice()` while retaining legacy one-shot functions.

## Verification performed

- 9,304 scalar interpreter-vs-C function comparisons passed.
- 1,680 forward CFG comparisons passed.
- 1,112 loop comparisons passed.
- 14,286 new resumable instruction-slice state comparisons passed across all
  1,470 candidates; of those checkpoints, 2,622 had a pending delay slot.
- Faulting PC stays intact and retry succeeds after memory mapping correction.
- Native dispatcher C integration test passed.
- Clang-cl compiled Windows x64 COFF objects; the largest object was checked
  with objdump as PE x86-64. No Windows executable or game play was validated.

## Local reproduction

Use your legally owned US retail `SCUS_975.01`; exact required SHA-256:
5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63

Linux: `AE3_ELF=/path/to/SCUS_975.01 bash BUILD_AND_TEST_LINUX.sh`.
Windows: set `AE3_ELF` and run `BUILD_WINDOWS_OBJECTS.bat` with Python, VS
C++ headers and clang-cl. It generates C and five **object files**, not an EXE.

The `ae3_*_leaves.c` files in this ZIP are generated from the user's original
game executable and are NOT published in the public GitHub repository.

Remaining blockers: complete and verified game code coverage; inter-routine
calls/indirect dispatch; 128-bit R5900 instructions and FPU/VU; full native IOP,
BIOS, disc, audio, graphics and input integrations. Previous recomps are only
references, not the main source tree.