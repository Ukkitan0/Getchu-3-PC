# Native C backward-branch caller checkpoint

Date: 2026-10-18. Canonical project: `Ukkitan0/Getchu-3-PC`.

**Not a playable Windows port.** This checkpoint continues executable-derived source recovery from the user's verified US `SCUS_975.01` ELF; generated game code and game files are intentionally excluded from the public GitHub repository.

## New source recovery

- New conservative `loop_stack.py` scanner permits bounded backward and forward BEQ/BNE/BLEZ/BGTZ/REGIMM/direct-J branches within scalar caller candidates, together with known-target JAL or whitelisted runtime JALR.
- **42 additional** original-code caller candidates, **1,091 additional** original instruction sites; these contain backward loops and at least one call per candidate. The combined stack runner accepts **176** distinct caller candidates (30 direct, 1 nested, 31 indirect, 72 prior forward-flow, 42 new backward-flow).
- Combining these with 1,470 scalar/CFG/loop leaf candidates yields **1,646 candidate entrypoints**. Candidate function boundaries are inferred, not established to be correct for the whole program; the figures do not establish a percentage of game coverage.
- A branch is accepted only if its target is inside the fully decoded body, not a delay slot, and not an instruction owned by a previously accepted routine. Calls to unavailable static targets and unsupported instructions remain rejected. Control transfers are performed after their delay slots, and instruction-budget yields preserve that state.
- `stack_lift.py` now incorporates `loop_stack.py` as the final discovery pass and keeps the previous `collect()` result API stable. The generated stateful C runner and the public stack dispatcher expose the recovered candidates.

## Verified original code

- `test_backward_call_paths.py` drives retail address `0x00105AC4`, calling retail leaf `0x00105A28` to read a table of 32-bit values, for loop lengths **0**, **1**, and **3**.
- For 3 entries, **55 guest instructions** are retired, two backward-branch delay slots are observed, all three original-code calls are reached, and the destination array matches the expected byte-level values. Native execution state, GPRs and RAM are compared after each execution slice to an independent original-word MIPS oracle.
- `test_backward_structure.py` checks all recovered loop bodies for complete targets and delay-slot/boundary exclusion.
- Stack-routine differential suite: `test_stack.py 2` tests 176 caller entries over **352 trials / 6,484 slices**; the suite includes more than the new 42 routines. Prior leaf, CFG, loop, resumable and call suites are preserved.
- The native Windows stack object builds with `clang-cl` for x64 COFF without warnings following constant folding of same-register BEQ/BNE conditions.

## Execution instructions

Run locally against an original game ELF: `AE3_ELF=/your/path/SCUS_975.01 bash BUILD_AND_TEST_LINUX.sh`. On Windows with Python and LLVM/Clang: run `BUILD_WINDOWS_OBJECTS.bat` with `AE3_ELF` set. Neither command builds or boots a playable game.

## Still missing

Substantially more executable coverage; accurate R5900 high halves, multimedia, COP1 and VU operations; verified executable/global initialization; EE/IOP scheduling and services; GPU/VIF/GIF/GS geometry rendering; asset/disc I/O, sound, input and game state integration. This source-recovery checkpoint is a limited, independently tested native translation—not a full game decompilation.