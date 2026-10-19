# Getchu-3-PC — Original CreateThread and COP0 nested-initializer checkpoint

October 19, 2026. **Nonplayable experimental native EE boot diagnostic.**

## Authentic original guest execution

The continuation begins where the previous verified test stopped: the retail
CreateThread (`0x20`) syscall at `0x004358E4`. PS2SDK identifies the layout as
`ee_thread_t` (9 x 32-bit guest words at offsets 0x00..0x20); the original
retail code populated these fields before entering the service.

- The new `AE3_EEThreadKernel` reads/captures the original guest descriptor,
  validates mapped code, stack, size, and GP, and assigns a *compatibility*
  object ID. The created thread is dormant, with **NO native or guest thread
  execution, context-switching, interrupts, scheduler, or real BIOS IDs**.
- The real syscall return executes guest `JR $ra`, its delay slot, the
  `BGEZ`/delay-slot ID store to `0x006A1788`, and on success the exact guest
  initializer stores to `0x00779B98` and `0x00779B9C`. It enters the actual
  retail next call `0x00436C20` after 12 guest instructions, counting the
  CreateThread syscall.
- Thread allocation failure (including full pool, invalid stack descriptor)
  takes the original guest `DeleteSema` (`0x41`) cleanup call. The semaphore
  object is removed from the same compatibility registry as CreateSema,
  and the retail epilogue returns -1 to `0x0043FE90`.
- Inside `0x00436C20` the original game saves its frame, invokes the
  original `0x004365B0` COP0 Status query, and checks `(Status XOR 1) & 1`.
  With *explicit test-supplied* Status=0, the original return restores
  registers and reaches the actual `GetThreadId` (`0x2F`) syscall at
  **`0x004359D4`** after another 23 guest instructions. With Status bit 0
  set it instead stops before the unsupported critical-section callee at
  `0x00436C48`. Status EIE bit 0x10000 is explicitly rejected.
- New success-path instruction accounting: prior 323,444 + 12 + 23 =
  **323,479** counted retail MIPS instructions. The figure is a sum of
  separately validated experimental segments and **NOT** evidence of a
  continuous, authentic retail ELF/BIOS boot.

## Tests actually run

- Optimized GCC regression: CreateThread success and resource-exhaustion
  cleanup, malformed descriptor, CPU/RAM single-step/batch equality, negative
  syscall path, opcode mutation, and recoverable memory faults: **PASS**.
- Clang ASan+UBSan regression of the same paths: **PASS**.
- Previous `test_ee_sema_kernel`, `test_boot_event_create` and
  `test_boot_timer2_finish` regressions: **PASS**.
- Three source modules (`ae3_ee_thread_kernel.c`,
  `ae3_boot_thread_continue.c`, `ae3_boot_thread_next.c`) compiled to
  x86-64 Windows COFF objects using `clang-cl /W4 /WX`: **PASS**.
- Integrated boot diagnostic successfully linked on Linux with 36 C source
  modules. **NOT RUN WITH RETAIL ELF** in this session because the original
  separately dumped `SCUS_975.01` executable is unavailable.
- `verify_opcode_snapshot.py` confirmed **221/221** pinned retail MIPS words
  across five TSV tables match the recovered 32-MiB user PCSX2 EE savestate
  reference (which is post-boot data, not genuine BIOS startup state).

## Limitations / next engineering boundary

The next original guest service is `GetThreadId` at `0x004359D4` (0x2F).
The native kernel does not have enough genuine BIOS scheduling state to
infer its return ID, so execution stops before servicing that syscall.
The alternative COP0 bit0 branch reaches the unimplemented real guest call
at `0x00436C48`. No GS/VU/IOP runtime, audio/input/gameplay runtime,
authenticated Sony BIOS initialization, or playable Windows game yet.

This checkpoint contains only source code, compiled test/object outputs and
integrity records. No user's savestate, extracted EE memory, retail ELF,
ISO, BIOS, licensed media assets, or ROM is redistributed.