## October 19, 2026 — REAL native Lua 5.0 gameplay scripting subsystem

**Original-style AE3 stage Lua source now ACTUALLY RUNS inside the Windows
C++20 + Direct3D11 engine** using the official SHA256-pinned Lua 5.0.3
interpreter. The project keeps stage behavior written in Lua rather than
rewriting its script logic into C++. The engine directly executes
setupArea(opts), creates native entity descriptions, preserves original
Lua parameters, nested c_order/c_ctrl command trees and flags, and now
advances controller timelines with native elapsed game time. Timed wait,
nested/looping controllers, trigger signals and typed action event
dispatch are implemented. Independent Windows CTest ran real Lua 5.0
code and verified controller behavior; no PS2 hardware runtime is involved.

The Windows-native GUI offers "Run original Lua 5.0 stage" for locally
recovered Lua SOURCE and a "Signal Lua trigger" test control; standalone
Getchu3LuaStageRunner.exe runs the independently written example from
native/examples/native_binding_demo.lua. Lua 5.0.3's MIT license and
official checksum are tracked.

**These are native stage scripting and event systems, not a finished
Ape Escape 3 level.** The Win32 application still lacks actual original
models/world rendering, player/monkey movement and interaction, authentic
animation/collision, sound and complete game binding implementations.
Original PS2 .luc bytecode must first be converted to source on the user's
PC. See native/LUA50_NATIVE_GAMEPLAY.md for exact functionality and
remaining requirements.

---

## October 19, 2026 — Saru3VfiTool host-native asset preparation (NEW)

The optional third-party [Saru3VfiTool](https://github.com/mthsk/Saru3VfiTool)
extractor now has a **working standalone offline workflow** in this repo:
double-click [PREPARE_NATIVE_ASSETS_WINDOWS.bat](PREPARE_NATIVE_ASSETS_WINDOWS.bat),
choose your own Ape Escape 3 ISO or DATA.BIN, then choose a separately
obtained Saru3VfiTool Windows executable/DLL. The native ISO9660 reader copies
DATA.BIN byte-exact into a private workspace, Saru extracts recursive SZ/PCK
containers, TIM2 textures and game parameter metadata, and the native stage
scanner inventories recovered Lua files. Output stays under `native/private`
and is excluded from Git. Overwrites are prohibited by default.
See [native/SARU3_NATIVE_ASSET_INTEGRATION.md](native/SARU3_NATIVE_ASSET_INTEGRATION.md).

Optional **MIT [ae3-sdk](https://github.com/pxdl/ae3-sdk)** I3D-to-GLB
conversion is also integrated into the offline pipeline: after installing the
separate `ae3` CLI, supply `-Gltf` to
`native/tools/PREPARE_NATIVE_ASSETS_WINDOWS.ps1`. This is an independent
build-time converter of original I3D model bytes into skinned/rigged GLB
assets; the game executable never depends on it. See the Saru guide for
the exact command and limitations.

**No PS2 CPU/GPU emulation has been reintroduced.** GPLv3 Saru3VfiTool is
used as a separate optional developer utility, never bundled into or linked
from the Getchu3Native.exe host game. The app is not yet playable: actual
I3D level rendering and complete Lua/native gameplay systems remain open.

# Getchu 3 PC — TRUE NATIVE WINDOWS PORT (CURRENT)

**Active architecture:** Windows C++20 + Direct3D 11, native host file I/O,
native TIM2 texture decoding, original-game VFI asset loading, native Lua 5.0
stage discovery, and reconstructed player state. **No PS2 CPU, IOP, GS,
DMA, console hardware emulation or PS2-web-recomp executable in the active
build.** Do not use the old experimental PS2 hybrid pipeline.

**CURRENT LIMITATION:** This is a functional *native engine/asset workbench*,
NOT a playable Ape Escape 3 build. The original game menu, world rendering,
player movement, gameplay entities and complete stage Lua bindings are not
implemented. Source-only Windows CI cannot verify actual retail gameplay.

**Build:** Run [BUILD_NATIVE_WINDOWS.bat](BUILD_NATIVE_WINDOWS.bat) on a Windows
PC with Visual Studio 2022 C++/CMake. The legacy
[TEST_APE_ESCAPE_3_WINDOWS.bat](TEST_APE_ESCAPE_3_WINDOWS.bat) routes to that
same native-only build. It produces Getchu3Native.exe, Getchu3AssetProbe.exe,
and Getchu3StageProbe.exe inside native/build/Release. The UI shows a
development debug grid and previews supported original assets, not a game level.

**Major new game-specific discovery:** Original Ape Escape 3 stage logic is
stored in Lua 5.0 area.luc files. Research from the
[2021 AE3 reverse engineering article](https://www.cnblogs.com/algonote/p/15678057.html)
has been integrated as an original-game Lua stage scanner, local-only
recovery helper, and a native C++ player-save/HP layout.
See [native/RESEARCH_AE3_LUA50_NATIVE_PORT.md](native/RESEARCH_AE3_LUA50_NATIVE_PORT.md).

The [native Windows CI workflow](https://github.com/Ukkitan0/Getchu-3-PC/actions/workflows/native-windows.yml)
builds all host-only executables and tests native ISO/VFI, TIM2, Lua stage
recognition, and player-state behavior using non-commercial fixture data.

---

## Retired historical recompilation notes below — NOT the active port

## October 19, 2026 — native game AOT source integrity and visual-frame evidence

Native Windows source-to-C++ cache now SHA-verifies every generated translation
unit and `units.hpp`; incomplete or altered generated code forces a rebuild.
Upstream native frame captures are **P6 PPM**, so boot reports now count those
files and sample color/brightness to distinguish all-black output from more
visually varied frames. Separate `shots/session-.../round_XX` folders ensure
old captures cannot be mistaken for newly rendered frames. Details:
[CHECKPOINT_NATIVE_RENDER_EVIDENCE.md](CHECKPOINT_NATIVE_RENDER_EVIDENCE.md).
CI smoke and source-only tests do not imply that the actual retail game boots
or renders levels; a native playable Ape Escape 3 test remains unverified.

## October 19, 2026 — Fix for `SeedMode 8` in Windows one-click launcher

The original `TEST_APE_ESCAPE_3_WINDOWS.ps1` built a positional string
array resembling `-ExploreRounds 8` and splatted that array to another
PowerShell script. PowerShell interpreted `8` as the earlier positional
`SeedMode` parameter, which accepts only `constructors`, `common`,
or `all`.

The corrected launcher uses **named hashtable splatting** and passes
`ExploreRounds`, `ExploreFrames`, `Disc`, and `Run` as typed
parameters. A Windows CI regression executes the real launcher against a
game-free fake build script whose complete parameter list matches the
native pipeline; it checks `ExploreRounds 8`, `12`, `0`, and
software-renderer mode. That targeted test passed.

The build also treats a selected ISO as authoritative, rather than
silently picking up a leftover `SCUS_975.01` from an older unrelated
recompilation folder.

To update existing local folders without deleting any private code or
game files, replace these two files from `main`:
`TEST_APE_ESCAPE_3_WINDOWS.ps1` and
`RUN_BULK_NATIVE_WINDOWS.ps1`.

A metadata-only log collector is available as
`COLLECT_AE3_DIAGNOSTICS.bat` when `tools/diagnostic_bundle.py` exists.
It packages redacted build errors but not commercial ISO/ELF, guest RAM,
captured frames, or generated game code.

## 2026-10-19 — genuine runtime-generated guest code captured and AOT-executed (synthetic; game-free)

Windows CI run [39322608306](https://github.com/Ukkitan0/Getchu-3-PC/actions/runs/39322608306)
passed the dedicated dynamic-code stage. The native R5900 runtime executed an
original synthetic ELF that wrote new instructions to executable guest RAM,
discovered the missing JALR target, dumped guest memory, converted the window
to a named code image, recompiled the overlay with the real upstream recompiler
and executed that new code through guest RAM result 143 and Exit. **No retail
game execution or playable title screen is implied.** The parent workflow GPU
step may still be in progress. See `CHECKPOINT_NATIVE_DISC_BOOT.md`.

## Native game ISO integration checkpoint (2026-10-19)

The native game-build script accepts only an owned ISO (no manual ELF path
required), extracts BOOT2 from SYSTEM.CNF, supports SNR2 modules and
runtime-unpacked code overlays, emits per-round RAM/VU/frame evidence and a
prioritized boot diagnostic report. `TEST_APE_ESCAPE_3_WINDOWS.bat` provides
a Windows file-picker build-and-test entrypoint with persistent card saves.
The source-only ISO tests pass on GitHub CI; original retail game runtime
remains unverified. See `CHECKPOINT_NATIVE_DISC_BOOT.md`.

## October 19, 2026: verified seven-region native AOT + Windows GPU compilation (GAME-FREE)

GitHub Actions [run 39318625918](https://github.com/Ukkitan0/Getchu-3-PC/actions/runs/39318625918) succeeded. The real upstream R5900 recompiler and native ClangCL Windows runtime executed 143 indirect synthetic callbacks across seven distinct 16 KiB code regions and guest RAM reads/writes across three memory pages. The workflow also passed Windows WebGPU runtime compilation/linking. The owner-local Windows game builder now bootstraps pinned vcpkg/SDL2 and copies SDL2.dll to the test executable. **No actual retail Ape Escape 3 executable or ISO was executed in CI; no playable Windows test build exists yet.** See `CHECKPOINT_AOT_MULTIREGION.md`.

## Windows toolchain smoke now compiled a native runner (game-free stub)

GitHub CI at commit `b98c874` compiled the upstream real EE and VU compilers,
then built and linked native ClangCL/SDL2 `ps2runtime.lib`, `ps2game.lib` and
`ps2run.exe` on Windows Server 2022. That CI run returned failure from its
no-argument usage-exit check **after** the executable was built. This is not a
retail Ape Escape 3 execution. The runner only links empty registration stubs.
The usage-exit check was corrected, and a second CI compilation targeting the
verified official Win32 wgpu-native GPU path was added. Neither corrected
workflow's final result has been confirmed as passing.

## 2026-10-19: rebuild private opcode inventory from owned ELF (CI/public-repo fix)

A fresh public checkout omits retail code bytes by design. The native bulk
translator now derives executable instruction words and 12,228 raw JAL call
candidates from a **SHA-256-validated owner-provided ELF** into private output.
A synthetic ELF test verifies the automatic regeneration path, invalid ELF
rejection, and existing-inventory reuse. **28/28 offline Python tests pass.**
Initial Windows CI exposed the public missing-file assumption; the smoke workflow
has been restricted to tests requiring no retail ELF/derived opcode bytes.
A Windows game build has NOT been verified. See `CHECKPOINT_PRIVATE_ELF_RECONSTRUCTION.md`.

## 2026-10-19: native Windows runtime overlay + iterative AOT exploration (SOURCE ONLY, NOT PLAYABLE)

Reused MIT upstream `snuri00/ps2-web-recomp` at pinned `6526f48`, adding Win32 SDL2
HWND + WebGPU surface plumbing, MSVC GPU library linking, Win32 metrics, pin-verified
Windows wgpu-native v29.0.1.1 setup, mandatory window-enabled SDL2 build, ClangCL
toolchain, and bounded code-miss / VU1 exploration loops. Added an isolated CI smoke
workflow and 22 green Linux-side Python tests. **Windows runtime compilation and game
execution remain unverified.** See `CHECKPOINT_WINDOWS_NATIVE_ENGINE.md`.

## October 19 — bulk native recompilation branch (not yet playable)

Added a tested original-EE full-inventory entry generator, 143 static
constructor callback seeds, 12,228 direct-call candidates, eight regression
tests and an isolated native Windows upstream-C++ translation/build script.
Default upstream seed list is 146 safe anchors + constructor targets; C++
translation with the user-owned ELF and actual native Windows build remain
untested. No browser/WSL substitution. Source route documented in
`CHECKPOINT_BULK_RECOMP.md` and `fastlane/README.md`.

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

## Current checkpoint: static C++ constructor table (2026-10-19)
The retail `0x00628210` entry now executes through the first constructor dispatcher,
stopping before first callback `0x0041A390`. 143 saved-reference callback entries were recovered;
52 additional guest instructions execute on the reference-table branch, 64 on an explicit empty fixture.
504 opcode-table rows match the owner-provided later-game EE snapshot.
**No full retail-ELF boot, authentic Sony BIOS state, or playable Windows game.**
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

## Latest checkpoint — GetThreadId, ChangeThreadPriority and OSD service boundary

- Executes 17 more real original MIPS instructions across `GetThreadId` (`0x2F`), `ChangeThreadPriority` (`0x29`, test priority 1), and original return to `0x0043FE90`.
- Executes 14 more real original instructions into the next initializer and stops at genuine `GetOsdConfigParam` (`0x4B`) syscall at `0x00435B94` without manufacturing BIOS configuration.
- Uses explicit **test-only main thread ID=255**, not a guessed Sony BIOS value, and a stateful native kernel priority record. 253/253 pinned retail opcode rows pass against the user's savestate reference.
- **323,510** composed experimentally tested guest MIPS instructions; native C tests, Clang ASan+UBSan and Windows COFF objects pass. No complete retail-ELF boot test or playable port yet.

See [CHECKPOINT_THREAD_PRIORITY_OSD.md](CHECKPOINT_THREAD_PRIORITY_OSD.md).

# CURRENT: Original CreateThread and GetThreadId (October 19, 2026)

Experimental native translation now accounts for **323,479** original guest
instructions with explicit compatible COP0/kernel state, not genuine BIOS boot.
Success through the real CreateThread 0x20 guest return reaches the next nested
retail COP0 inquiry and GetThreadId 0x2F at 0x004359D4. Failure executes the
actual DeleteSema 0x41 cleanup. 221 pinned instruction words match the
user-provided post-boot EE RAM reference, and Windows COFF/GCC/Clang
sanitizer regressions pass. Gameplay, GS/VU/IOP, Sony BIOS, scheduling,
and playable Windows build remain unimplemented.

See CHECKPOINT_THREAD_GETID.md.

## October 19 — corrected CreateSema and CreateThread boundary (latest)

Recovered the 32-MiB EE RAM from the previously provided user savestate, and
validated all 171 pinned original opcode rows against it. Syscall 0x40 is
CreateSema; 0x50 is CreateEventFlag. Actual original MIPS now executes the
CreateSema result path into thread-descriptor construction and reaches the
CreateThread syscall 0x20 at 0x004358E4 (positive-ID path), or exits to the
parent initializer on generic negative service failure. Tested under ASan and
UBSan; compatibility mode only, **NOT A PLAYABLE PORT**.

## October 19 — Timer 2 second MODE + guest event syscall (latest)

Original retail instruction `0x00440710` is now executed, and both MODE
outcomes are represented. Default MODE-enabled branch returns to the parent
`0x0043FE88`, then the next guest initializer checks mapped global
`0x006A1788` and reaches syscall `0x40` at `0x00435AE4`. The normal
experimental fixture has 323,423 summed retired guest instructions,
30 additional since the preceding checkpoint. Real compiled C regressions
and ASan/UBSan pass. Full original ELF boot not rerun (executable absent).
No real BIOS or playable native Windows game. Refer to
`CHECKPOINT_TIMER2_FINISH_EVENT.md`.

## October 19 — EnableIntc + next Timer 2 MODE read checkpoint

The experimental original-code chain has moved from guest `0x004405C8` to
`0x00440710` (the latter remains pending before its Timer 2 MODE read), through
an explicit compatibility-model syscall 0x14 interrupt enable for cause 11.
The 323,393 instruction count is obtained by summing verified earlier
checkpoint counts with new fixture-tested paths, **not** by a newly run full
retail ELF trace. Refer to `CHECKPOINT_ENABLE_INTC.md` for precise tests,
limitations and original addresses. Not playable.

# October 19 — Timer 2 registers configured by original retail MIPS

- After the real AddIntcHandler wrapper, the game now executes 74 further
  original instructions in compatibility mode: 8 MODE-read/branch instructions
  and 66 instructions in its guest SetT2_COUNT, COMP, MODE and COP0/ERET
  helper path, ending at `EnableIntc` call PC `0x004405C8`.
- Three Timer 2 MMIO writes flow through guest KSEG1 aliases to one physical
  native register bank. The already-running branch skips COUNT/COMP writes.
- Explicit COP0 Status=0 is an experimental supplied test fixture, not BIOS.
- Regression tests run under GCC and Clang sanitizer setups; 3 Win64 COFF
  modules compile. Default boot continues to stop at `SetupThread`.

# 2026-10-19 — INTC AddIntcHandler through actual Timer 2 MODE boundary

- Original EE syscall `0x10` cause 11, handler `0x00440A50` is registered
  through an explicit compatibility-kernel registry. BIOS handler ID is NOT
  known or claimed to match Sony behavior.
- Original wrapper, BGEZL success/error paths, handler-ID memory store,
  COP0 Status=0 conditional critical helper and next Timer 2 MODE boundary
  at physical `0x10001010` / PC `0x00440590` verified.
- Full opt-in probe retires 323264 original EE instructions. Default boot
  stops at authentic SetupThread after 315897. No playable Windows game.

# 2026-10-18 EE memory-bus update

- True EE virtual/physical RAM aliases and optional BIOS/scratchpad mappings implemented in `ae3_ee_memory_bus.[ch]`.
- Original `FindAddress` MIPS handler now fetches through shared physical RAM, not a disconnected KSEG0 buffer.
- Explicitly test-only boot diagnostic reaches `0x0043FDC4` after **1,626,728 retired instructions**; both FindAddress results are zero because no boot-time BIOS has initialized EE kernel RAM.
- Later PCSX2 RAM snapshot parity: original handler finds `0x80014F0C` after 21,444 reads and returns zero for the second query after 131,073 reads. The snapshot is not a substitute BIOS.
- This is not a playable build or successful original BIOS boot. Real SetupThread/SetupHeap semantics, kernel memory population, IOP and graphics still missing.

# Current milestone — registered EE syscalls and original FindAddress guest handler

**Experimental. Not playable.** The previous stopping point at `SetSyscall` has
advanced through *both* original 0x74 registrations. Original syscall 0x83 is
routed to the retail 0x0043FCC0 guest handler, and its MIPS word-search code
now runs with a caller-provided KSEG0 map (not a fabricated BIOS result).

- Registered: `0x83 -> 0x0043FCC0`, `0x5A -> 0x0043FC88`.
- Retired startup guest instructions at first 0x83 syscall: **315,977** in
  opt-in fixture mode (initial SetupThread stack and semaphores still HLE).
- Two subsequent guest FindAddress invocations and their wrappers execute
  on synthetic kernel window through the next 0x0043FDC4 branch.
- Using the later PCSX2 title-screen *recorded* EE RAM for a reference search:
  0x0043FCC0 returned 0x80014F0C; 0x0043FC88 returned 0.
  This is **not** evidence of original boot-time BIOS contents.
- Tested against real retail ELF, one-instruction scheduling, faults, and
  AddressSanitizer/UndefinedBehaviorSanitizer; Windows x64 COFF build passed.
- Actual authentic boot still blocks at SetupThread without a kernel provider.
  GS/VU/IOP/SIF/audio/gameplay remain to be integrated.

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