## 2026-10-19: second static constructor numeric call reached (NOT PLAYABLE)

The real second constructor `0x00417C58` tail-jumps to `0x00417B50`; its
original code invokes the genuine guest return stub twice, then enters the
`0x0041EFD0` numeric routine with IEEE-754 double 10.0. **97** original guest
instructions execute across the second constructor and numeric prefix, stopping
**before** `JAL 0x0041F118 -> 0x00423E20`. The second constructor has not
finished. **877/877** pinned opcode entries match the owner's later-game EE
reference. Clang sanitizers, GCC, two Windows x64 object builds, boot diagnostic
linking and existing targeted regressions pass. No verified complete retail ELF
boot or playable Windows executable. See `CHECKPOINT_SECOND_CONSTRUCTOR.md`.

## 2026-10-19: first original static constructor executed (NOT PLAYABLE)

Executed real `0x00428AE8` JALR into `0x0041A390`, tail jump to `0x0041A088`,
and the original twelve `LD/DADDIU/SD` reference counter updates. Cold
zero-count fixture: **225** original guest instructions and 12 actual helper
stub returns; prior-initialized counts: **164** instructions and no helper
calls, but still 12 incrementing guest stores. The dispatcher reaches the
**second original callback `0x00417C58`** pending at `0x00428AE8`.
**712/712** pinned retail instruction rows match the memory reference;
GCC, Clang sanitizers, Windows x64 object and linked boot diagnostic pass.
The original ELF end-to-end boot, BIOS and playable Windows port remain
unfinished. See `CHECKPOINT_FIRST_CONSTRUCTOR.md`.

## 2026-10-19: original static-constructor dispatcher reached (NOT PLAYABLE)

Original `0x0010033C` now enters `0x00628210`, executes the two retail JAL
calls and reaches the original C++ constructor dispatcher at `0x00428AB8`.
The recovered later-state constructor table has **143** distinct code-entry
addresses ending at `0x0062E660` with the `0xFFFFFFFF` sentinel. The first
real callback is `0x0041A390`, pending at the `0x00428AE8` JALR after
**52** new guest instructions (composed experimental total **329,149**).
An explicitly empty-table fixture instead follows the authentic conditional
return and reaches the next call at `0x00100354` after **64** instructions.
The callbacks themselves are **not yet executed**. 504 exact-pinned opcode
rows match the owner's RAM reference. New Windows x64 object, sanitizer,
and regression checks pass; the full retail ELF is still not booted natively.
See `CHECKPOINT_STATIC_CONSTRUCTORS.md`.

## 2026-10-19: original post-TLB caller and 512 guest byte stores (NOT PLAYABLE)

The native experimental CPU executes the retail `0x0043FEA0` saved-RA load,
`J 0x00441B70`, two guest global `SW` clears and tail-calls the original
`0x0042B320` memset. For this call, `0x00771288` is misaligned to 16,
so the real byte-store loop runs: **512 actual MIPS SB** and **3,609**
new retired guest instructions. It stops before `JR $ra` at `0x0042B3D8`;
no return address is assumed for the authentic Sony BIOS path.
An independently verified **unique** retail call site at `0x0010031C`
provides the expected return address `0x00100324`. The additional
original `JR $ra`, FlushCache wrapper/compatibility request and main startup
execute **12 more** instructions (only if actual saved RA matches) and stop
before `JALR $s0` at `0x0010033C` targeting `0x00628210`. Cumulative
experimental count **329,097**; **436 pinned opcode words** matched snapshot.
Focused GCC/Clang sanitizer regression and Windows x64 COFF object compiled.
No playable port, Sony BIOS/IOP/GS execution, or authentic full retail ELF run.
See `CHECKPOINT_POST_TLB_MEMSET.md`.

## 2026-10-19: retail InitTLBFunctions guest execution completed (NOT PLAYABLE)

Original `0x0043FE98 -> 0x0043F470` executes through the genuine retail
816-byte MIPS Copy to `0x80075000`, two FlushCache requests, eight syscall
registrations, six copied GetEntryAddress searches and return to `0x0043FEA0`.
**1,922** further original guest instructions in experimental test mode,
**325,476** cumulative; **391** opcode rows verified against saved EE RAM.
Compiled and sanitizer-tested on Linux; two Windows x64 objects built; a
retail-ELF integrated run and actual BIOS/IOP/GS/gameplay remain unavailable.
See `CHECKPOINT_TLB_FULL.md` for exact evidence and test commands.

# Getchu-3-PC — Ape Escape 3 native Windows decompilation

**Active, canonical decompilation project. Experimental. NOT PLAYABLE.**

Goal: reconstruct the US PS2 retail executable `SCUS_975.01` into native C,
then develop a Windows game runtime with genuine EE/IOP services, graphics,
input, sound and saves. Older recompilation efforts are reference material only.
This public repository contains recovery tools, original implementations,
and tests, **not** the copyrighted ISO, ELF, BIOS, ROM or game assets.

## Current checkpoint — OSD Get/Set original game return

The original game now executes 44 additional retail MIPS instructions beyond
`GetOsdConfigParam` at `0x00435B94`: two getters and two setters, testing and
then restoring an explicitly provided OSD configuration. The original guest
returns to the next parent JAL at **`0x0043FE98`**. **323,554** original
instructions accounted for across composed experimental segments; **290/290**
original words independently match the user-provided PCSX2 savestate. GCC,
Clang ASan+UBSan and Windows x64 COFF object compilation pass. The complete
retail-ELF boot diagnostic was not executed here. **NOT PLAYABLE.**

See [CHECKPOINT_OSD_ROUNDTRIP.md](CHECKPOINT_OSD_ROUNDTRIP.md).

## Previous checkpoint — GetThreadId, ChangeThreadPriority and OSD service boundary

- Executes 17 more real original MIPS instructions across `GetThreadId` (`0x2F`), `ChangeThreadPriority` (`0x29`, test priority 1), and original return to `0x0043FE90`.
- Executes 14 more real original instructions into the next initializer and stops at genuine `GetOsdConfigParam` (`0x4B`) syscall at `0x00435B94` without manufacturing BIOS configuration.
- Uses explicit **test-only main thread ID=255**, not a guessed Sony BIOS value, and a stateful native kernel priority record. 253/253 pinned retail opcode rows pass against the user's savestate reference.
- **323,510** composed experimentally tested guest MIPS instructions; native C tests, Clang ASan+UBSan and Windows COFF objects pass. No complete retail-ELF boot test or playable port yet.

See [CHECKPOINT_THREAD_PRIORITY_OSD.md](CHECKPOINT_THREAD_PRIORITY_OSD.md).

## Latest checkpoint — native CreateThread return and next GetThreadId

- Experimental `CreateThread` (`0x20`) registry captures the original
  9-word EE thread descriptor from game RAM, assigns a dormant object ID,
  and executes the original MIPS return/branch/delay slot to `0x00436C20`.
  Allocation failure uses the retail `DeleteSema` (`0x41`) cleanup path.
- The next guest callee executes the authentic nested COP0 Status test in
  `0x004365B0`. Explicit test Status=0 reaches the original `GetThreadId`
  (`0x2F`) syscall at **`0x004359D4`**, after 35 further guest instructions
  across the two continuations. The alternate status route halts safely.
- **323,479** guest instructions are accounted for across composed experimental
  paths. **221 original instruction words** verify against the user-supplied
  post-boot RAM snapshot. GCC, Clang ASan/UBSan, Windows x64 COFF object
  compilation, and previous targeted tests pass. Original ELF end-to-end
  execution is unavailable in this session; **no playable Windows game**.

See [CHECKPOINT_THREAD_GETID.md](CHECKPOINT_THREAD_GETID.md).

## Latest checkpoint — corrected CreateSema and original CreateThread boundary

- Recovered missing retail MIPS instruction bytes from the user's PCSX2 EE
  savestate (post-boot reference, **not BIOS boot state**). All **171** original
  words across three pinned TSV files match its EE RAM bytes.
- Corrected syscall `0x40`: **CreateSema**, not CreateEventFlag (`0x50`).
  The actual MIPS return wrapper, semaphore ID delay-slot store, and thread
  descriptor setup now execute to **CreateThread syscall `0x20` at
  `0x004358E4`**. Both success and failure branches have verified native
  regression coverage, including Clang ASan/UBSan.
- **323,444** composed original guest instructions on the experimental boot
  path; retail ELF complete boot integration, authentic BIOS, graphics, audio,
  IOP, and gameplay remain **unverified/unimplemented**. Not playable.

See [CHECKPOINT_SEMA_THREAD.md](CHECKPOINT_SEMA_THREAD.md).

## Previous checkpoint — second Timer2 MODE and event initializer

- The original guest MODE read at `0x00440710` is now performed against a
  mapped Timer 2 native register, not inferred. The enabled branch executes
  10 original instructions and returns at `0x0043FE88`.
- Disabled MODE follows the original SetT2_MODE guest JAL and shared
  COP0/ERET helper to `0x0044074C` (29 instructions, 1 MMIO write). The
  alternate prior-interrupt state stops at the original EIntr JAL.
- The next main initializer JAL enters the genuine routine `0x004368B8`;
  its guest global check at `0x006A1788` takes either the real skip return
  or reaches syscall `0x40` at `0x00435AE4`. No kernel reply is faked.
- **323,423** original instructions accounted for on the default synthetic
  compatibility-model boot chain, +30 since the previous checkpoint.
  Full retail ELF integration still unverified in this session, because the
  retail executable is unavailable. Nonplayable native boot diagnostic only.

See [CHECKPOINT_TIMER2_FINISH_EVENT.md](CHECKPOINT_TIMER2_FINISH_EVENT.md).

## Latest checkpoint — real EnableIntc wrapper and next initializer

- The original guest program now executes its `EnableIntc` JAL at
  `0x004405C8`, reaches retail syscall 0x14 for cause 11 (Timer 2), passes
  through an **experimental** registered-interrupt compatibility service,
  and returns through the original guest frame to `0x0043FE80`.
- The following original JAL enters `0x004406F0` and stops before the next
  Timer 2 MODE hardware read at `0x00440710`. The opt-in chain is **323,393**
  counted retail instructions, 55 beyond the prior checkpoint.
- Standalone pinned-original-opcode regressions and Windows x64 object builds
  passed. The full-ELF integrated run requires the user's own original ELF,
  absent here; **this is not an operational native game or true BIOS model**.

See [CHECKPOINT_ENABLE_INTC.md](CHECKPOINT_ENABLE_INTC.md).

## Current checkpoint — retail Timer 2 MODE through real COP0/ERET writes

- The opt-in boot runner has advanced from the guest `LW` at `0x00440590`
  through its original branch and delay slot. It executes the **retail MIPS**
  `SetT2_COUNT`, `SetT2_COMP`, and `SetT2_MODE` wrappers and their shared
  COP0/ERET store routine, preserving guest addresses and register updates.
- A new native Timer 2 MMIO model supports physical and uncached KSEG1 aliases,
  COUNT/MODE/COMP, write-one-to-clear flags, and caller-specified clock ticks.
  Unsupported HBLANK/gate timing is explicitly rejected, not approximated.
- With explicit test-only COP0 Status=0 and reset Timer 2 MODE=0, three real
  MMIO writes configure COUNT=0, COMP=0xFFFF, and timer MODE; execution reaches
  the next original `EnableIntc` JAL at **`0x004405C8`**, after **323,338**
  counted guest MIPS instructions. With MODE bit 7 already enabled, the game
  correctly skips COUNT/COMP initialization; both paths are tested.
- Tests verify retail opcodes, step-vs-batch parity, atomic MMIO fault retry,
  alias consistency, unsupported timing modes, and regression preservation.
  GCC/Clang sanitizer tests pass, and three Windows x64 COFF objects build.
- These are limited, experimental kernel/COP0 behaviors, **not a Sony BIOS or
  playable Windows game**. The ordinary boot diagnostic still stops at the
  authentic `SetupThread` request. The next guest kernel boundary is
  `EnableIntc` (EE syscall `0x14` for the lower-level service).

See [CHECKPOINT_TIMER2_BOOT.md](CHECKPOINT_TIMER2_BOOT.md).

## Current native checkpoint — EE interrupt registration to Timer 2 MODE

- Actual allocator startup now executes through its `AddIntcHandler` (`0x10`)
  wrapper, with an **experimental stateful INTC handler registry** instead of a
  preselected return ID. Guest cause 11 registers handler `0x00440A50`.
- The original guest `JR`, `BGEZL` branch-likely and delay slots are executed,
  including **both success and negative error-return paths**. The successful
  guest writes the returned ID into physical EE RAM at `0x006A2E28`.
- With **explicit guest COP0 Status bit 16 clear**, the real status-check helper
  `0x0043FBA0` runs and the guest reaches its next unmapped MMIO read at PC
  `0x00440590`: `0x10001010`, **EE Timer 2 MODE (not INTC_STAT)**.
- The opt-in boot diagnostic now retires **323,264 original MIPS instructions**;
  tests pass with GCC/Clang sanitizers, one-step/batched parity, and all three
  new modules compile to Windows x64 COFF. The original 8-KiB MMI memset was
  translated by semantic equivalence and not counted as retired instructions.
- **Not playable:** real Sony BIOS `SetupThread` still blocks default boot;
  hardware timer/INTC/COP0 semantics, VU/GS, IOP and gameplay remain missing.

See [CHECKPOINT_INTC_TIMER2.md](CHECKPOINT_INTC_TIMER2.md).

## Prior native checkpoint — allocator and AddIntcHandler boundary

- Executed the retail initializer at `0x004404B0`, including the genuine
  loop building **128 descending stack-area links**, and nested routine
  `0x00441780` initializing a **64-entry** free list.
- The preceding 8-KiB optimized EE MMI `memset` at `0x0042B320` is converted
  to a **native C semantic operation for the exact verified call**, not
  instruction-level MMI emulation. Source tests check every output byte.
- Reached `AddIntcHandler` (EE syscall `0x10`) at `0x004357D4` with interrupt
  cause **11** and handler **`0x00440A50`**, without manufacturing any BIOS reply.
- Explicit compatibility-mode diagnostic reaches **323,244 retired original
  guest instructions**, excluding the internally unexecuted MMI memset words.
- Clang AddressSanitizer/UndefinedBehaviorSanitizer and original ELF
  single-step/batched regression tests pass. Windows x64 COFF compiled;
  **no playable Windows executable exists**.
- Default authentic startup remains blocked at Sony BIOS `SetupThread`.

See [CHECKPOINT_ALLOC_INTC.md](CHECKPOINT_ALLOC_INTC.md).

## Latest native boot checkpoint — full copied GetEntryAddress syscall loop

- The original executable now runs from the preceding game-installed kernel
  payload copy at `0x00440398` through the **full six-handler registration
  loop**, returning from the genuine MIPS initializer at `0x0043FE78`.
- The original second `Copy` call moves 40 bytes into `0x00082000`.
  Two actual `FlushCache` requests with modes 0 and 2 are recorded by the
  explicitly **experimental** kernel interface; separate PS2 caches are not
  simulated.
- The genuine MIPS `GetEntryAddress` handler is executed directly from the
  copied `0x80076000` game code, using its original guest-memory table at
  `0x80076710`. Six guest queries return addresses that register syscalls
  `0xFC`, `0xFE`, `0xFD`, `0xFF`, `0x12C` and `0x08`.
- The opt-in full boot diagnostic reaches `0x0043FE78` after **321,851 retired
  guest instructions**, including the nested guest handler calls. The default
  boot continues to stop at `SetupThread` without an authentic BIOS response.
- Linux sanitizer and one-step/batched parity tests pass, and the new modules
  compile as Windows x64 **objects**. **No playable Windows executable exists.**

See [CHECKPOINT_GET_ENTRY_LOOP.md](CHECKPOINT_GET_ENTRY_LOOP.md).

## Verified checkpoint — 2026-10-19

- Original retail ELF: SHA-256
  `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
- Recovered **1,646 conservative candidate entrypoints**, not complete-game C.
  The executable's entire `.text` is indexed, but much of it is not translated.
- The real retail startup executes 315,897 instructions up to `SetupThread`
  **without** test-only kernel behavior. A bounded native-C executor preserves
  the original 128-bit GPR state, stack memory, delay slots and guest PC.
- Explicit opt-in compatibility-kernel tests continue through `SetupHeap`,
  `CreateSema`, `SetSyscall`, and the original registered `FindAddress` handler.
- **New:** `ae3_boot_post_find.[ch]` runs the real comparison/backward-loop code
  at `0x0043FDC4`, including further calls to the original guest handler.
  Without authentic BIOS data it requests a **third** syscall `0x83` at
  `0x0043FD04`, not a fabricated success.
- **New:** with *clearly synthetic, matching kernel search entries*, the native
  runner returns to the original next initialization JAL at `0x0043FE70`,
  enters `0x00440338`, and executes the register/stack prologue. It stops
  **before** its original `LW` from `0x10001810` at `0x00440354`. That register
  is **EE Timer 3 MODE**, confirmed by [PS2SDK EE register definitions](https://github.com/ps2dev/ps2sdk/blob/master/common/include/ee_regs.h).
- Original-instruction unit tests, one-step-vs-batched state parity, fault
  recovery, Clang ASan/UBSan and Windows x64 **object** compilation pass.
  **No Windows executable or runtime graphics is claimed.**

See [CHECKPOINT_POST_FIND_TIMER3.md](CHECKPOINT_POST_FIND_TIMER3.md) and
[CURRENT_BUILD_STATUS.md](CURRENT_BUILD_STATUS.md) for exact evidence and limits.

## Try the Windows boot diagnostic (not a game)

In a VS 2022 Developer Command Prompt with Clang tools, set `AE3_ELF` to your
**own** NTSC-U retail `SCUS_975.01` and use:

```bat
set AE3_ELF=C:\ApeEscape3\SCUS_975.01
BUILD_BOOT_DIAGNOSTIC_WINDOWS.bat
```

By default the diagnostic stops at genuine `SetupThread` and does not invent
BIOS state. For the expressly artificial extended trace:

```bat
set AE3_BOOT_TEST_TIMER3_COPY=1
BUILD_BOOT_DIAGNOSTIC_WINDOWS.bat
```

This optional mode uses a documented stack-top compatibility policy,
experimental semaphores, and two **synthetic** words in kernel RAM. It continues via the explicitly experimental Timer 3 model and guest kernel
registration loop, returning to `0x0043FE78`; it never launches gameplay.

## Linux regression checks

```bash
AE3_ELF=/path/to/SCUS_975.01 bash BUILD_AND_TEST_LINUX.sh
```

The full generated-C suite can be slow. New focused tests:

```bash
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_elf_image.c \
  ae3_ee_memory_bus.c ae3_guest_find_address.c ae3_boot_post_find.c \
  test_boot_post_find.c -o test_postfind
./test_postfind "$AE3_ELF"

cc -std=c11 -O2 -Wall -Wextra -Werror ae3_elf_image.c \
  ae3_ee_memory_bus.c ae3_guest_find_address.c ae3_boot_post_find.c \
  ae3_boot_next_init.c test_boot_next_init.c -o test_next
./test_next "$AE3_ELF"
```

To run the two different modelled boot paths after building `build/ae3_boot_probe`:

```bash
build/ae3_boot_probe "$AE3_ELF" --test-ee-boot-model --test-sema-hle --test-ee-alias --test-post-find
build/ae3_boot_probe "$AE3_ELF" --test-ee-boot-model --test-sema-hle --test-ee-alias --test-next-mmio
```

## Implementation structure

- `bulk_lift.py`, `cfg_lift.py`, `loop_lift.py`, `resume_lift.py`,
  `call_lift.py`, `stack_lift.py`: bounded binary-to-C recovery.
- `ae3_elf_image.[ch]`: checked retail ELF32 to 32 MiB EE memory loader.
- `ae3_ee_memory_bus.[ch]`: KUSEG/KSEG0/KSEG1 physical RAM aliases;
  unknown hardware addresses cause faults, not fake responses.
- `ae3_boot_prefix.[ch]`, `ae3_boot_memory_fill.[ch]`: true 128-bit reset,
  BSS clearing and original first initialization fill.
- `ae3_ee_boot_services.[ch]`: test-only, guest-derived initial stack/heap
  model — **not Sony BIOS**.
- `ae3_boot_semaphore_init.[ch]`, `ae3_boot_hook_init.[ch]`,
  `ae3_guest_find_address.[ch]`: real guest CreateSema/SetSyscall wrappers
  and original MIPS address-search handler with experimental kernel support.
- `ae3_boot_post_find.[ch]`: search reconciliation and repeated caller path.
- `ae3_boot_next_init.[ch]`: original next callee through EE Timer 3 MODE read.
- `ae3_boot_post_patch.[ch]`, `ae3_guest_get_entry.[ch]`: second kernel
  Copy, two FlushCache requests, all six copied-code GetEntryAddress calls,
  seven SetSyscall registrations, original branch-likely loop and return.
- `ae3_boot_probe.c`: transparent boot diagnostic; default never invents
  the BIOS state.

## Remaining work before a playable Windows port

Faithful EE kernel/BIOS startup, timing-sensitive Timer 3 registers and
interrupts, broader main-game control-flow and R5900 instruction translation,
IOP and SIF services, VU/GS rendering, game asset/disc access, controller
integration, audio and save behavior. A successful isolated function test or
cross-compiled `.obj` file does **not** establish that the game boots or plays.

All generated game-derived C/ELF/ISO and PCSX2 snapshots should stay outside
public Git history. Historical detailed checkpoints remain under `CHECKPOINT_*.md`.