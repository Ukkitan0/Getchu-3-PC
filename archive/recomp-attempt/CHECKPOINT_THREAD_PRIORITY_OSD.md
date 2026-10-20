# Getchu-3-PC — GetThreadId, priority and OSD initialization

October 19, 2026. **Native C EE compatibility diagnostic; NOT PLAYABLE.**

## Advancement (real US retail instruction stream)

The previous experimental path stopped before the original `GetThreadId` (`0x2F`) syscall at `0x004359D4`. This checkpoint adds two separate verified native-C guest execution segments:

1. **17 original MIPS instructions**: the actual `GetThreadId` syscall, the game's nested `JR` and delay slot at `0x004359D8`, its `a0=v0` argument propagation, `JAL` to `ChangeThreadPriority` (`0x29`) at `0x00435974` with priority `1`, and all original saved-register restores up to parent PC `0x0043FE90`. Both syscalls use the real original `SYSCALL` instruction, not host-inserted branch jumps.
2. **14 original MIPS instructions**: parent `0x0043FE90` calls the retail next initializer `0x0043FF80`, which creates two original EE stack frames, saves its preserved registers, enters `0x0043FF18`, and reaches `GetOsdConfigParam` (`0x4B`) syscall at **`0x00435B94`**. The pending syscall is deliberately not serviced without genuine or explicitly modeled OSD configuration.

Previously reported test path: 323,479 counted original guest MIPS instructions. This checkpoint adds 31 = **323,510** across separately verified *experimental* segments; this is **not** an uninterrupted genuine BIOS boot or a percentage decompiled.

## Explicit kernel policy

`ae3_ee_thread_kernel_bind_current` accepts an explicit current boot thread ID and priority for testing. The integrated diagnostic uses **ID 255 and initial priority 64** as declared compatibility inputs, not BIOS-observed values, and prevents future `CreateThread` allocations from reusing that reserved ID. A standalone `GetThreadId` call with no bound current thread fails atomically rather than inventing a current ID. `ChangeThreadPriority` supports the modeled current thread and `TH_SELF` (0) with range validation; invalid IDs/priorities return a negative guest result without changing host state. Successful service returns a compatibility result of zero; the game's caller then ignores this result and reloads its stored created-thread ID.

The project does NOT emulate thread scheduling, sleep/wake/preemption or interrupts. Dormant CreateThread records remain dormant. Real Sony kernel priorities, IDs, timing and privilege transitions remain unknown.

## Verification performed in this session

- Native GCC and Clang compilation with `-Wall -Wextra -Werror`: **PASS**.
- New `test_boot_thread_priority`: original opcode verification, explicit unbound/current context, invalid ID policy, one-instruction/batched CPU+RAM equality, `GetThreadId` and `ChangeThreadPriority` return values, stack stores and register restores, recoverable MMIO/memory faults and opcode-mutation rejection: **PASS**.
- Clang AddressSanitizer + UndefinedBehaviorSanitizer for new test: **PASS**.
- Existing `test_boot_thread_continue` optimized and sanitizer regressions: **PASS**.
- Existing semaphore registry and guest memory-bus regressions: **PASS**.
- All **253 pinned instruction rows**, across seven TSV opcode tables, independently crosschecked against 32-MiB `eeMemory.bin` from the user's earlier PS2 savestate: **PASS, 0 mismatches**. The savestate is post-boot evidence, not authentic original BIOS state.
- Windows x64 COFF objects for `ae3_boot_thread_priority.c`, `ae3_boot_osd_entry.c` and updated `ae3_ee_thread_kernel.c` compile with `clang-cl /W4 /WX`: **PASS**.
- The full Linux boot-diagnostic program links with 38 modules (not run against the retail ELF in this session). `clang-cl` alone in this Linux container does not have the Windows SDK headers/libraries to link the actual Windows diagnostic EXE; the three standalone COFF objects compiled successfully.

## Repeatable tests

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_ee_memory_bus.c \
  ae3_ee_thread_kernel.c ae3_boot_thread_priority.c ae3_boot_osd_entry.c \
  test_boot_thread_priority.c -o build/test_boot_thread_priority
./build/test_boot_thread_priority
python3 verify_opcode_snapshot.py /local/path/to/eeMemory.bin
```

For the full opt-in experimental chain with a legally owned game ELF in the project folder:

```sh
./build/ae3_boot_probe SCUS_975.01 --test-ee-boot-model --test-sema-hle --test-ee-alias --test-timer2-mode
```

That *complete* retail-ELF test was **not** executed here because `SCUS_975.01` itself was not available; only the previously provided PCSX2 savestate was. The archived source and COFF objects contain no retail ELF, ISO, BIOS, graphics, audio or other proprietary game contents.

## Next boundary

`GetOsdConfigParam` `0x4B` at `0x00435B94`: model configuration supplied from an explicit source (or read and validate the original guest OSD buffer), execute the original wrapper/return path and subsequent `0x00435B80` setter. Do not invent Sony BIOS/user OSD settings. The alternate COP0 Status bit 0 route at `0x00436C48` also remains unimplemented.

The goal is still a full playable **native Windows** port. IOP, GS/VU and the broader engine remain unfinished. No playable EXE is claimed.