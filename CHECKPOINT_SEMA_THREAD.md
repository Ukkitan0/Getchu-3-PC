# Getchu-3-PC — CreateSema original-game return to CreateThread

October 19, 2026. **Experimental EE compatibility mode, NOT PLAYABLE.**

## New evidence

Recovered the user-provided PCSX2 `SCUS-97501 (7571AAEE).01.p2s` EE RAM
snapshot (32 MiB `eeMemory.bin`, ZIP method 93 / Zstandard). Cross-checked
**all 171** pinned original MIPS words in the project's three `retail_*_words.tsv`
files against snapshot memory: **zero differences**. The snapshot is used only
to authenticate executable instructions; it is **post-boot state**, NOT a
replacement for original ELF loading or genuine Sony BIOS startup. No RAM,
BIOS, executable, or game assets are included in the source repository.

## Kernel identity correction

PS2SDK and documented EE syscall tables both identify **0x40 as CreateSema**;
**0x50 is CreateEventFlag**. Prior checkpoint wrongly described syscall 0x40
as a possible event flag. This path is a **semaphore + thread startup**,
not an event flag. The source filename `ae3_boot_event_create` remains a
compatibility identifier to avoid invalidating existing imports/tests.

## Executed original retail instructions

- Starting at the original pending syscall `0x00435AE4`, validate a mapped,
  aligned guest `ee_sema_t` (six 32-bit fields) from guest `$a0`. The existing
  experimental native semaphore pool services syscall `0x40` and assigns an
  internal positive ID or returns a generic negative error; guest memory,
  IDs, or return values are **not fabricated by the instruction runner**.
- Guest `JR $ra`, delay slot and branch `BLTZ $v0` at `0x004368FC` now run.
  In both outcomes its original delay-slot `SW` stores the returned semaphore
  ID at `0x006A9B90`.
- On success the genuine `LUI`, `ADDIU`, `SW`, `DADDU` and `JAL` instructions
  populate the next guest stack descriptor, then execute the **original**
  `0x004358E0` wrapper and stop before **CreateThread syscall `0x20` at
  `0x004358E4`**. The following compatibility-kernel service is unimplemented.
- On failure the game's original branch returns -1 through its preserved
  stack frame to `0x0043FE90`.
- Starting at `0x0043FE88`, success reaches the CreateSema pending boundary
  after 20 instructions, then retires 21 more guest instructions including
  the syscall itself, and reaches CreateThread: **41** guest instructions
  (versus **33** on the error path).
- In the full-chain diagnostic the same compatibility semaphore kernel is
  reused from the earlier two CreateSema calls. The IDs depend on the pool
  state and are **not Sony BIOS results**. Explicit default-fixture instruction
  accounting reaches **323,444**, composed from isolated verified segments,
  not from a complete uninterrupted original ELF boot trace.

## Verification

- New `test_boot_event_create.c` covers successful and rejected semaphore
  allocation, original delay-slot store of returned ID, single-step versus
  batched guest CPU/RAM parity, original thread descriptor field values,
  repeated service rejection, opcode mutation, and atomic MMIO/memory-fault
  recovery.
- Standalone native tests passed optimized GCC, and Clang
  `-fsanitize=address,undefined`; `ae3_boot_event_create.c` compiled as
  x86-64 COFF via `clang-cl /W4 /WX`. Full 33-module Linux diagnostic linked.
- `verify_opcode_snapshot.py` reads a user-owned 32-MiB RAM dump and checks
  the retail opcode TSV evidence; it ships neither the RAM image nor the
  user's savestate.
- No full original-ELF integration test has run in this session because
  the separate retail executable `SCUS_975.01` is not present. The savestate
  cannot substitute for that original executable in the native boot test.

## Build

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror \
  ae3_ee_memory_bus.c ae3_boot_event_create.c ae3_ee_sema_kernel.c \
  ae3_boot_semaphore_init.c test_boot_event_create.c -o build/test_boot_event_create
./build/test_boot_event_create
python3 extract_pcsx2_ee_reference.py /path/to/SCUS-97501.p2s /tmp/eeMemory.bin
python3 verify_opcode_snapshot.py /tmp/eeMemory.bin
```

## Next authentic boundaries

- `CreateThread` syscall `0x20` at `0x004358E4`, descriptor `$a0` in guest
  memory and original post-return MIPS already recovered from PCSX2 RAM.
  The next step is a carefully validated native compatibility thread object
  and guest return path; **do not assume actual Sony BIOS thread IDs**.
- Additional original calls at `0x0043694C` and `0x0043696C` are still
  unexecuted. IOP, VU/GS graphics, audio, input, and gameplay runtime do
  not exist in this native port. **Not playable.**
