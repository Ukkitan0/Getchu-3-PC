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