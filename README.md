# Getchu-3-PC — Ape Escape 3 native decompilation

Current direction: **reconstruct and verify the original Ape Escape 3 (NTSC-U, SCUS_975.01) MIPS R5900 code as native C**, then develop a native Windows runtime. This repository supersedes earlier PS2 recompilation experiments; they may be consulted as reference material, not treated as canonical code.

**Status (2026-10-18): research/source recovery, not playable.**

- Confirmed original executable SHA-256: `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
- 1,163 conservative straight-line scalar candidate routines compiled to native C; 9,304 reference comparisons passed.
- Another 168 bounded forward-branch/direct-jump candidate routines compiled; 1,680 reference comparisons passed.
- The 1,331 candidate routines are *not* verified coverage of the game. Some may be data misidentified as code; 128-bit R5900 operations, loops, indirect dispatch, EE/IOP system services, VU1 and GS graphics are outstanding.
- Native Windows COFF objects were cross-compiled; **no playable Windows executable is claimed**.

## Building from your legally obtained game

1. Obtain an unmodified NTSC-U `SCUS_975.01` from your own copy of Ape Escape 3. Do not upload it here or commit it.
2. Use Python 3.10+ and a C11 compiler (Clang / MSVC with an appropriate toolchain).
3. Set `AE3_ELF` to your local path to `SCUS_975.01`.
4. Run `python bulk_lift.py`, then `python cfg_lift.py`. Each tool refuses executable versions other than the pinned SHA-256.
5. For Linux verification: `cc -std=c11 -O1 -fPIC -shared ae3_scalar_leaves.c -o libae3_scalar.so`, `cc -std=c11 -O1 -fPIC -shared ae3_control_leaves.c -o libae3_control.so`, `python test_bulk.py 8`, and `python test_cfg.py 10`.
6. The generated C is experimental translated code, not a standalone runnable game.

## Repository policy

Only tooling, tests, source-recovery metadata, and our own runtime implementations belong in this public repository. Original game ISO/ELF, assets, state dumps, and generated machine-code-derived game C must **not** be committed. Generated code can be reproduced privately from a legal copy of the game.

## Engineering priorities

1. Verify reachable function boundaries and strengthen reference testing, especially branch delay slots, loops, FP and MMI.
2. Implement or integrate a native EE/IOP service layer and GPU/VU1/GS pipeline; preserve controller functionality.
3. Produce a demonstrably playable Windows build with title, gameplay, full world geometry, audio and saves.

Older host/recompilation trees are historical references only.