# Native recovery checkpoint — 2026-10-18

**Not playable.** Verified retail NTSC-U `SCUS_975.01` SHA-256: `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.

- 1,163 straight-line low-64-bit scalar candidate leaves (4,067 original instruction locations; 9,304 random interpreter comparisons passed).
- 168 bounded forward-control-flow candidate leaves (1,998 original instruction locations; 1,680 comparisons passed).
- 139 new backward-branch / bounded-loop candidates (2,162 original instruction locations, 155 static backedges; 1,112 baseline or 3,336 extended comparisons passed).
- **1,470 disjoint native dispatch entry addresses; 8,227 translated instruction locations.** A recovered entry address is not proof of a correct game function boundary.
- Native C compiled and tested on Linux. Four Windows x64 COFF objects were successfully produced with `clang-cl`; these are not a playable executable.
- New bounded-loop return value `-3` means 4,096 guest instructions were executed without completion; the caller MUST NOT mistake it for successful return or resume without tracking PC/guest state.

**Priority engineering obstacles:** validate true code/function boundaries, handle loops with resumable PCs, indirect calls and returns, high EE register lanes/MMI/COP1/VU, IOP/BIOS services, VIF/GIF/GS graphics, disc resources, audio and game runtime integration.

Sources in the GitHub repository are the canonical work. Old recompilation snapshots are references only.

## Resumable native execution checkpoint

- New source generator: `resume_lift.py`. Generates native C capable of
  executing 1,470 candidate routines in arbitrary-sized instruction slices,
  including branch and return delay-slot suspension.
- Includes 9,390 represented instruction addresses (the earlier per-subset
  total was 8,227, excluding return instructions in straight-line candidates;
  these are different counting conventions, not newly decompiled 1,163 words).
- Verified by `test_resume.py`: 14,286 native-C vs independent original-MIPS
  **slice snapshots** across random input registers/memory; 2,622 checkpoints
  in a pending branch/return delay-slot state. Existing independent tests
  (9,304 scalar + 1,680 forward + 1,112 loop) also passed.
- Memory mapping faults leave PC unchanged and can be retried after repair.
- Linux native dispatcher integration test passes and Windows x64 COFF object
  for `ae3_resumable_leaves.c` compiled using `clang-cl`.
- This is not evidence of a bootable game. The runtime, BIOS/IOP, GPU and
  gameplay are not connected.

## Direct-call native C checkpoint (2026-10-18)

- 30 additional non-overlapping, straight-line caller candidates (550 instruction sites), targeting 35 known direct JAL edges. Total distinct candidate entries 1,500; total represented original instruction sites 9,940.
- Recovered caller machine instructions update `$ra` before executing the actual JAL delay slot; leaf execution runs in a resumable child frame and returns to the caller continuation.
- Differential tests: 90 caller trials, 1,167 compared slices, 81 completed, 6 memory faults, 3 bounded still-running, plus an explicit caller memory-fault/retry check. These are behavioral comparisons against an independent original-word MIPS interpreter, not evidence of runtime playability.
- Common native dispatcher integration passed; native Windows x64 COFF call objects compile.
- **Not playable.** Only restricted scalar callers and previously recovered leaves; complex PS2 execution and rendering remain unimplemented.

## Stack execution and indirect call checkpoint (2026-10-18)

- Native `stack_lift.py` adds **1 nested JAL caller** and **31 dynamic JALR caller candidates** beyond the previous 30 direct callers.
- **1,532 distinct candidate entrypoints**, with 390 newly represented instruction sites (previous 9,940 + 390 = 10,330).
- 8-deep bounded native C stack with resumable frames, MIPS delay slots, and strict whitelist for dynamic indirect targets.
- `test_stack.py 3`: 1,656 execution slice comparisons passed across 186 original-code caller trials; real virtual call and unknown-target rejection tests passed.
- New stacked dispatcher and original legacy dispatcher integration passed; new Windows x64 COFF objects compiled with clang-cl.
- **Not playable.** The game runtime, GPU and many guest instructions remain unimplemented.