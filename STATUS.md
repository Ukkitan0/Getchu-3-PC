## Retail semaphores and first syscall-hook registration — 2026-10-18

- Verified 34 additional original guest instructions in `0x0043FC08` and its two `CreateSema` syscall wrappers; returned EE semaphore IDs are written by the original code to `0x006A1EA8`/`0x006A1EAC`.
- Opt-in native EE semaphore model supports 256 IDs, create, poll and signal, but is NOT an accurate PS2 BIOS kernel implementation.
- The next original initialization routine is executed through the first `SetSyscall` syscall `0x74` at PC `0x0043FE4C`, with handler `0x0043FCC0` and guest requested syscall `0x83`. No actual hook installation has occurred.
- ASan/UBSan and compiler-warning-as-error checks pass for the three new test programs; all three new C modules compile to Windows x64 COFF objects.
- Default authentic boot still stops at `SetupThread` and does NOT use the experimental kernel model without explicit flags. Not playable.
- Details: [CHECKPOINT_SEMA_HOOK.md](CHECKPOINT_SEMA_HOOK.md).

---

## Kernel/heap service boundary and first initialization call (2026-10-18)

- Added `ae3_boot_kernel_bridge.[ch]`: captures true SetupThread 0x3C and SetupHeap 0x3D registers, enforces externally supplied verified kernel response, validates checked service state, and executes subsequent original retail userland instructions. Test-only stack fixture is NEVER equivalent to real BIOS behavior.
- Added `ae3_boot_init_prefix.[ch]`: executes original `JAL` into 0x0043FE58, its delay slot, `addiu sp,-16` and `sd ra,0(sp)`, stopping at nested `JAL 0x0043FC08`.
- Added a boot probe CLI; default mode stops without faking BIOS results. The regression fixture retires 315,909 total real guest instructions before its first nested call, but no graphical gameplay boots.
- Both new test programs passed clang AddressSanitizer/UndefinedBehaviorSanitizer and Windows x64 COFF builds of the two new modules passed.
- Read CHECKPOINT_KERNEL_BRIDGE.md for exact inputs, verified scope and real kernel/graphics blockers.

---

## Expanded startup and whole-ELF instruction inventory — 2026-10-18

- **Still not playable, still not completely decompiled.** This checkpoint is the canonical development state.
- Original retail boot prefix + first real JAL callee + caller setup now retire **315,897 actual guest instructions**, ending BEFORE syscall `SetupThread` 0x3C at PC `0x001002FC`.
- Actual first callee fills **393,216 bytes** of upper normal EE RAM with 24,576 aligned 128-bit stores, and returns to its original caller. Actual per-byte state, branch delay slots, and fault-retry state verified, including ASan/UBSan and two Windows x64 COFF compile checks.
- `full_elf_inventory.py` indexes every word in the 5,407,320-byte retail `.text` section (**1,351,830** 32-bit words) and all **3,200** 64-bit VU packets in `.vutext`. This is not equivalent to validated function recovery.
- Next hard boot dependency: faithful EE `SetupThread`/`SetupHeap` kernel behavior; arbitrary fake stack pointers or host emulated menus will not count.
- Full details: [CHECKPOINT_BOOT_EXPANDED.md](CHECKPOINT_BOOT_EXPANDED.md).

---

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

## Forward branch + native inter-function caller checkpoint (2026-10-18)

- New `extend_stack.py` admits 72 bounded caller candidates containing BEQ/BNE/BLEZ/BGTZ/REGIMM forward transfers or direct J plus JAL/JALR. **1,604** distinct recovered candidate entrypoints total; **134** stack-callable candidates. An additional **1,432** original instruction locations were emitted for this pass.
- Preserves branch decisions across instruction-slice yields using `pending=3`; memory fault in a delay slot leaves the saved PC unchanged for retry.
- Fixed JALR invalid-target atomicity: rejected dynamic calls no longer modify the saved target or return address before reporting a fault.
- Test: 268 original-word vs native-C differential trials / 2,194 slices, and forced both paths of a real retail BEQ plus JALR wrapper. Native Linux dispatcher integration passed; Windows `clang-cl` compiled updated x64 COFF stack object.
- **Unchanged blocker:** no bootable playable game, no EE/IOP/GS/VU runtime, not all original code converted. The executable-derived C is private to the game owner's workspace, not committed to this public repository.

## Backward-branch callers with inter-function calls (2026-10-18)

- `loop_stack.py` adds **42** candidate routines / **1,091** new instruction sites containing at least one backward branch and JAL/JALR. Total stack-callable candidates: **176**. Total known candidate entries including scalar/CFG/loop leaves: **1,646**; represented instruction count is approximately **12,853** under the earlier checkpoint counting convention (not guaranteed full game code).
- `test_backward_structure.py` verifies all recovered branch targets and excludes delay slots/overlapping bodies; `test_backward_call_paths.py` verifies retail function `0x00105AC4` with 0, 1 and 3 real JAL loop iterations against independently decoded MIPS words.
- `test_stack.py 2`: 352 trials across 176 candidates, 6,484 compared slices. The previous scalar, control-flow, loop, resumable, call and dispatcher tests remain in the build suite.
- New Windows x64 COFF stack object compiled with `clang-cl` (no warnings after same-register BEQ/BNE folding).
- **No playable Windows executable.** The full game still requires large-scale MIPS translation and substantial EE/IOP/GS/VU and media runtime engineering.

## Unified guest execution and EE ELF image mapping (2026-10-18)

- `ae3_guest_execution.c/.h` unifies 1,470 resumable leaf candidates and 176 call-stack candidates into one typed native-C execution API. It selects a recovered engine using an actual entry address; reports `UNKNOWN` for unsupported addresses (including genuine boot entry `0x00100134`).
- It exposes deepest active guest PC, retired instruction count, completion status, and sliced execution. Its caller owns all state, registers and RAM. Reopened frames reset deterministically.
- `test_guest_execution.c` passes: 1,646 entries enumerable, leaf/stack parity, retail `0x00105AC4` 0/3-iteration loop and `0x00105A28` callee, pending delay slots, and a fault in mapped memory followed by successful continuation.
- `ae3_elf_image.c/.h` preflights the entire ELF before mutating guest RAM. Validated retail input: ELF32 little-endian, MIPS machine 8, one PT_LOAD at virtual/physical `0x00100000`, file offset `0x1000`, 0x62C5D0 copied bytes, memory size 0x672910, executable entry `0x00100134`.
- `test_elf_image.c` passes byte-exact PT_LOAD mapping, BSS zeroes, sentinels outside load, and six malformed-file rejection cases that preserve RAM.
- Both new modules cross-compiled as Windows x64 COFF objects with `clang-cl` (C11, warning-as-error). Prior stack/branch/loop tests also passed on this checkpoint; comprehensive from-scratch script exceeded the local command timeout after passing scalar/CFG/loop/resumable/direct-call stages; remaining component tests were run separately.
- **Still not playable:** the true game startup entry remains outside the supported translation set. No IOP, BIOS, PS2 graphics/audio/disc or gameplay scheduler integration has been demonstrated.

### First real boot instruction audit

- Retail entry `0x00100134` begins with 29 `PADDUW` clears of 128-bit GPRs (r1–r25, r28–r31), followed by 4 primary/secondary HI/LO clears, a taken link branch, then 32 `MTC1` FPR resets.
- `audit_boot_entry.py` validates each original 32-bit instruction word and refuses a mismatch. The first unmodeled FPU accumulator instruction is at `0x0010023C`; the source audit **does not run those 66+ instructions**.
- This confirms why 64-bit-only translated leaves cannot faithfully boot the EE: real 128-bit GPR and COP1 state are needed.

## Original retail startup executed to first JAL (2026-10-18)

- Added native-C `ae3_boot_prefix.c/.h` and `test_boot_prefix.c`. Runs original mapped executable instructions from `0x00100134` to the first call site `0x001002C4`.
- Verified **143,836 guest instructions** and **17,969 128-bit stores**, clearing 287,504 bytes while preserving adjacent sentinels and FPR/GPR/HI/LO state semantics. Arbitrary execution slicing and a fault-then-retry produce the same final state and entire 32 MB guest RAM as one execution.
- Linux C11 `-Werror`, Clang AddressSanitizer and UndefinedBehaviorSanitizer tests passed. Native x64 COFF generated with `clang-cl /W4 /WX`.
- The first JAL targets `0x001036D8`, which touches PS2 device MMIO and uses further 128-bit operations; **not supported**, so this is **not a game boot**. Missing native PS2 runtime remains the fundamental blocker to a playable Windows build.