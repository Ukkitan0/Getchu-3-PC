# Native source recovery — branch-and-call wrappers

**Checkpoint:** 2026-10-18, primary repository `Getchu-3-PC`.

## What was changed

1. New `extend_stack.py`: statically identifies bounded caller bodies containing at least one conditional/unconditional forward branch plus one direct or indirect call. Enforces a whitelist of scalar instructions and known direct-call targets, and validates every branch target against accepted instruction addresses (not delay slots).
2. `stack_lift.py`: integrates 72 previously unsupported routines with the existing bounded stack and instruction-sliced native-C emission. Branch decisions are made before the delay slot. Adds `pending=3` for a branch delay slot and preserves its chosen target across scheduler yields.
3. Corrected `JALR` error atomicity: target resolution now validates a local candidate **before** changing the activation's saved `target`, `$ra`, PC, or pending state.
4. `test_stack.py`: expands independent original-word MIPS tests to branch and jump instructions, checks fault atomicity, and compares the complete stack state, GPRs and RAM.
5. `test_branch_call_paths.py`: deterministic original-code path test at `0x003817D0`, both BEQ outcomes, including real indirect call to recovered leaf `0x003B3CE0` on the not-taken path.
6. Updated dispatcher integration count and optimized Linux build/test script to link prebuilt generated shared modules at the final stage.

## Measurements

- Previously: 1,532 candidate entrypoints, 62 stack-capable caller entries.
- New: **1,604** candidate entrypoints, **134** stack-capable caller entries.
- New pass emits **1,432 original instruction locations** from 72 new caller bodies.
- `test_stack.py 2`: 268 trials, 2,194 execution slices, 160 completed, 106 faulted, 1,928 yielded, with maximum call-stack depth 2 reached.
- Targeted deterministic BEQ test: taken path **9 instructions**, not-taken path **23 instructions** including call into a verified leaf.
- `clang-cl`: updated stack module is valid x86-64 COFF `.obj`.
- Shared-library dispatcher integration passed; no native game initialization or rendering test is possible from this standalone translation subset.

## Constraints

This pass accepts only bounded **forward** branches in caller bodies and supported low-64-bit scalar instructions. It does not handle caller backedges, branch-likely annulment, arbitrary subroutine targets, PS2 DMA/VU/GS/IOP, or a functional Windows port. Recovery starts are heuristically inferred from retail direct-call targets, so recovered routines are candidates, not complete proven functions.