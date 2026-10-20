# Getchu-3-PC — Retail InitTLBFunctions completed (experimental)

2026-10-19. **Native EE guest execution checkpoint, NOT a playable game.**

## Verified retail instruction sequence

Beginning at `0x0043FE98`, the original game enters `0x0043F470`
(identified as an InitTLBFunctions-type kernel-patching initializer). The
new bounded native C interpreter executes the **original game's MIPS instructions**
through the routine's full return to its caller at **`0x0043FEA0`**.

- Executes original `SetSyscall(0x74)` wrappers to register the `0x5A Copy`
  handler at `0x0043F418` and later `0x5B GetEntryAddress` at `0x80075000`.
- Calls the retail game's installed `0x5A` guest Copy handler at `0x0043F418`.
  The original `LW`/`SW`/branches copy **816 bytes (0x330)** of the game's
  code/data from `0x006A1840` to EE KSEG0 `0x80075000`, committing exactly
  **204 original word stores**, not a synthetic `memcpy` shortcut.
- Executes both original `FlushCache(0x64)` wrappers (guest modes 0 and 2),
  records both requests. There is **no PS2 instruction/data cache model**;
  no cache effects are claimed.
- Registers the game's `0x54` syscall entry from the original constant table.
- Runs the game's freshly copied **`GetEntryAddress(0x5B)` MIPS lookup routine**
  at `0x80075000` six times. It reads the six-entry table actually copied to
  `0x80075300`, including branch and delay-slot behavior; no handler addresses
  are hardcoded into the lookup result.
- Registers five more guest syscall handlers, numbers `0x55..0x59`, using
  the returned guest table values.
- Executes the final original `GetEntryAddress(0x03)` lookup, writes its
  actual result to guest RAM `0x006A1BB8` and executes the original saved-GPR
  loads, `JR $ra` and stack pointer restoration, returning to `0x0043FEA0`.

The recovered post-boot source snapshot supports the following *example*
retail six-entry table in copied RAM: `0x55 -> 0x80075038`,
`0x56 -> 0x800750C8`, `0x57 -> 0x80075108`, `0x58 -> 0x80075158`,
`0x59 -> 0x800751A8`, and `0x03 -> 0x80075330`. Runtime lookups fetch
these entries from guest RAM, rather than inserting fixed handler IDs.

## Measurements and verification

- **1,695 new guest instructions** through the original copy, cache wrappers,
  and first `GetEntryAddress` entry; **227 additional** through the six
  lookups, five registrations, and function return: **1,922 total**.
- Composed experimental boot-chain counter: **325,476 guest instructions**;
  prior checkpoint was 323,554. This is *not* the actual number of instructions
  from a fully emulated authentic BIOS boot.
- **391/391 pinned MIPS opcode rows** match the owner's provided 32 MiB
  PCSX2 post-boot EE RAM reference (101 new rows, potentially overlapping
  at code PCs). The 14 runtime opcodes at `0x80075000` were verified at
  their original source address `0x006A1840` in the snapshot, and the
  live runtime fetches the contents copied into KSEG0.
- GCC `-O2 -Wall -Wextra -Werror` focused regression: PASS.
- Clang AddressSanitizer + UndefinedBehaviorSanitizer: PASS.
- Two new `clang-cl /W4 /WX /O2` Windows x64 COFF modules: PASS.
- Integrated Linux boot diagnostic with all modules: compiled and linked.
- Prior focused OSD, thread, and semaphore tests: PASS.
- No retail `SCUS_975.01` ELF accessible in this container; the **complete
  integrated runtime path was not executed with the actual ELF**.

Tests compare one-guest-instruction against batched execution, all 32 MiB
fixture RAM, complete native syscall registries, branch-likely behavior,
return PC, copied data, table lookups, and failure atomicity. Synthetic
fixture data does **not** prove authentic Sony BIOS data or hardware behavior.

Focused test (does not require game files):

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_ee_memory_bus.c \
   ae3_ee_syscall_kernel.c ae3_boot_tlb_patch.c ae3_boot_tlb_finalize.c \
   test_boot_tlb_patch.c -o build/test_boot_tlb_patch
./build/test_boot_tlb_patch
```

Opt-in integration when the user provides their own retail ELF:

```sh
./build/ae3_boot_probe SCUS_975.01 --test-ee-boot-model \
    --test-sema-hle --test-ee-alias --test-osd-roundtrip=00004019
```

## Limitations and next target

Only experimental guest-kernel compatibility is used after the real
`SetupThread` boundary. No authentic Sony BIOS startup, scheduler, IOP/SIF,
VU/GS rendering, audio or playable game. At `0x0043FEA0`, the *next*
original instructions restore the parent's return address and jump toward
`0x00441B70`. That remains the next unexecuted path.

The archive intentionally excludes the original game executable, ELF,
ISO, BIOS, savestate and EE memory bytes.

References: PS2SDK's `ee/kernel/src/tlbfunc.c`, `ee/kernel/include/syscallnr.h`
and the owner's independently supplied retail EE memory snapshot.