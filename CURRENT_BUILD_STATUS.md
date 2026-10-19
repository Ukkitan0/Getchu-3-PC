# October 19 — original allocator through AddIntcHandler kernel boundary

- Real 0x0043FE78 -> 0x004404B0 guest call now executes in the native
  compatibility runtime; the original BSS sentinel is -1.
- 8-KiB zero fill is **semantically translated** from the retail optimized
  MMI memset call, not its individual MMI instructions.
- Original MIPS loops write 128 descending pointers and initialize a
  64-node free list at 0x00770E80. Both memory structures are checked
  against independent expected memory images.
- A further original JAL reaches syscall 0x10 `AddIntcHandler` at
  0x004357D4 (interrupt 11, handler 0x00440A50, next 0).
- Explicit test-mode chain retires 323244 original guest instructions;
  translated MMI helper internals are excluded from the count.
- Native sanitizer tests, Windows x64 COFF compilation and preceding
  registered-handler regressions pass. No authenticated BIOS syscall result,
  no complete GS/VU, IOP, or playable Windows binary yet.

# October 19 — original game-installed GetEntryAddress loop completed

- Native retail-code path continues from 0x00440398 through original second
  40-byte Copy, both FlushCache syscall wrappers, and six calls to the MIPS
  handler copied into KSEG0 RAM at 0x80076000.
- The six guest handler addresses are recovered by real instruction execution
  from a table at 0x80076710, not hand-selected test responses.
- Seven syscall registrations now include 0x5B, 0xFC, 0xFE, 0xFD, 0xFF,
  0x12C and 0x08. Registry capacity expanded from 256 to 512 and KSEG0
  handler validation corrected to consult the EE memory bus.
- Executes original branch-likely loop and returns from the initializer to
  0x0043FE78 after 321,851 retired guest instructions in **test mode**.
- Two new original-executable regression programs pass Clang ASan/UBSan and
  instruction-by-instruction versus batched tests. Windows x64 core objects
  build with warnings treated as errors. Full from-scratch legacy generation
  was not rerun as part of this focused checkpoint.
- Default runtime still stops at authentic SetupThread (315,897 guest
  instructions); no faithful Sony BIOS, IOP services, VU/GS renderer,
  gameplay, or playable Windows game executable exists.

Details: CHECKPOINT_GET_ENTRY_LOOP.md.

# October 19 — original EE Timer 3 and game-installed kernel-patch checkpoint

- Game's Timer3 mode register at `0x10001810` now reads from an optional
  modeled MMIO device. The original MIPS branch at `0x44035C` works in
  one-step and batched modes; both outcomes tested.
- With **explicit experimental BIOS/kernel responses**, the genuine wrapper
  at `0x4402D0` registers `0x4402F0` as syscall 0x5A; the genuine MIPS handler
  copies all **1,856 bytes** of the game's built-in kernel payload to EE RAM
  at KSEG0 `0x80076000`. It returns at `0x440398` after 321,484 original
  guest instructions in the controlled end-to-end diagnostic.
- Four focused sanitizer tests pass and four Windows x64 COFF objects build.
  No actual Windows game launch, no GS rendering, and no faithful EE BIOS.
- Real default boot still stops at `SetupThread` (315,897 retired).

# October 19 — post-kernel lookup + Timer 3 boot checkpoint

- **New original-code coverage:** executes guest reconciliation at `0x0043FDC4`,
  including bounded repeated syscall `0x83` and the original guest
  `FindAddress` MIPS handler (return sites `0x0043FDE4` and `0x0043FE00`).
- **Matching-address fixture only:** reaches original call at `0x0043FE70`,
  runs original nested callee at `0x00440338` and stops before guest
  `LW $v1,0($v0)` at `0x00440354`, which reads `0x10001810` (EE T3_MODE).
- **Absent BIOS memory:** the original game requests a third `FindAddress`
  at `0x0043FD04` (no guessed address or return).
- `test_boot_post_find.c`, `test_boot_next_init.c` passed with Clang
  AddressSanitizer and UndefinedBehaviorSanitizer; original ELF opcode and
  one-step/batched parity validated. Two Windows x64 COFF objects compiled.
- Default boot still stops at authentic `SetupThread`. Explicit test fixtures
  and a policy-based kernel model are needed to reach the timer boundary;
  this is NOT a native boot, graphics implementation or playable PC build.

# 2026-10-18 EE memory-bus update

- True EE virtual/physical RAM aliases and optional BIOS/scratchpad mappings implemented in `ae3_ee_memory_bus.[ch]`.
- Original `FindAddress` MIPS handler now fetches through shared physical RAM, not a disconnected KSEG0 buffer.
- Explicitly test-only boot diagnostic reaches `0x0043FDC4` after **1,626,728 retired instructions**; both FindAddress results are zero because no boot-time BIOS has initialized EE kernel RAM.
- Later PCSX2 RAM snapshot parity: original handler finds `0x80014F0C` after 21,444 reads and returns zero for the second query after 131,073 reads. The snapshot is not a substitute BIOS.
- This is not a playable build or successful original BIOS boot. Real SetupThread/SetupHeap semantics, kernel memory population, IOP and graphics still missing.

# Current verified checkpoint — native EE handler dispatch and original FindAddress

Experimental and NOT PLAYABLE. Original US `SCUS_975.01` remains the sole game input.

- Authentic retail instruction prefix reaches `SetupThread` (315,897 retired instructions) without synthetic kernel effects.
- Opt-in fixture mode executes subsequent guest code through two `CreateSema`
  wrappers; experimental semaphore HLE supplies IDs.
- New kernel syscall table accepts original `SetSyscall` 0x74 registrations:
  0x83 -> 0x0043FCC0 and 0x5A -> 0x0043FC88.
- Original guest reaches the first 0x83 syscall at 0x0043FD04 (315,977
  retired instructions before guest handler execution). It is routed, not faked.
- New real MIPS `0x0043FCC0` handler executes against an externally supplied
  KSEG0 window. Tests run both handler invocations to the 0x0043FDC4
  branch using **synthetic kernel RAM**, including retryable unmapped reads,
  branch-likely delay slots, and one-step/batched state equality.
- The user-supplied later PCSX2 title-screen EE RAM snapshot provides an
  *independent reference* for word searches: `0x0043FCC0 -> 0x80014F0C`
  (21,444 reads) and `0x0043FC88 -> 0` (131,073 reads). It does not
  authenticate the original BIOS memory state during startup.
- Windows x64 COFF objects and Linux sanitizer tests pass for new C modules.
- No bootable or playable Windows game exists.

Repository: https://github.com/Ukkitan0/Getchu-3-PC

# Getchu-3-PC checkpoint — native boot through EE kernel hook boundary

Source canonical: https://github.com/Ukkitan0/Getchu-3-PC
Retail input: NTSC-U SCUS_975.01 (not included or redistributed).
Original SHA-256: 5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63

This checkpoint does NOT constitute a native playable port.

## Verified

- Bounded original EE boot prefixes reached SetupThread after 315,897 retired MIPS instructions, with no assumed BIOS return.
- Optional regression-only SetupThread stack pointer plus SetupHeap acknowledgement reach the real first init function.
- 34 more original guest instructions execute through the nested 0x0043FC08 initialization routine, two real CreateSema syscall wrappers, and return at 0x0043FE68. Kernel return IDs are injected explicitly or allocated by the experimental opt-in HLE kernel.
- With the explicitly selected experimental HLE, guest init code executes to SetSyscall 0x74 at 0x0043FE4C and requests registration of handler 0x0043FCC0 for syscall 0x83. No hook is installed. Total observed retired guest instructions: 315,962.
- Three targeted regression tests pass under AddressSanitizer and UndefinedBehaviorSanitizer.
- Three new C runtime modules build into Windows x64 COFF objects with clang-cl /W4 /WX.
- 1,646 candidate entrypoints are in the older limited-function native dispatcher (not full executable coverage).

## Commands

On Linux (requires original executable):
`./build/ae3_boot_probe /path/to/SCUS_975.01`
`./build/ae3_boot_probe /path/to/SCUS_975.01 --test-kernel-fixture --test-sema-hle`

On Windows use `BUILD_BOOT_DIAGNOSTIC_WINDOWS.bat` in a Visual Studio developer terminal. Default stops at SetupThread; `set AE3_BOOT_TEST_HLE=1` opts into test-only continuation. Windows build and actual runtime still need verification on Windows.

Next: real SetupThread kernel semantics; SetSyscall 0x74 registrations and handler dispatch; deeper EE code/IOP/SIF/VU/GS integration. A running game window is not provided.

### 2026-10-18 EE startup model extension

Added `ae3_ee_boot_services.c` and `test_ee_boot_services.c`, plus the
`--test-ee-boot-model` diagnostic. This new **opt-in compatibility mode**
computes stack pointer and heap metadata from game syscall registers and
validates ranges and overlap against the shared EE bus. It avoids a literal
stack-pointer return value in the diagnostic. The 16-byte stack-top reserve
remains a chosen policy (NOT confirmed Sony BIOS behavior). Native unit,
sanitizer and full downstream guest-handler parity tests passed. Default boot
still cannot pass authentic `SetupThread`; graphics/gameplay remain unavailable.