# Retail EE 0x5B GetEntryAddress and post-kernel patch loop — 2026-10-19

**Status:** Tested native C execution of original retail instructions. **NOT A PLAYABLE GAME.**

Original US `SCUS_975.01` SHA-256:
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
The game ELF, BIOS, ISO, and extracted save-state RAM are not distributed.

## New original-game execution

The previous checkpoint executed the original guest `0x5A Copy` handler and
returned to the original initializer at `0x00440398` after installing 1,856
bytes of kernel payload at KSEG0 `0x80076000`.

This checkpoint continues original MIPS execution from `0x00440398` all the way
through the initializer's return at **`0x0043FE78`**, including:

1. The second real guest `Copy` call: `0x006A2DB8` -> `0x00082000`, 40 bytes.
2. Two original `FlushCache` syscall wrappers (`0x64`), with guest modes `0`
   and `2` recorded by the experimental compatibility kernel. Native EE RAM
   does not currently implement separate physical instruction/data caches.
3. A guest `SetSyscall` registration for `0x5B` -> `0x80076000`.
4. **Six executions of the original copied game kernel GetEntryAddress MIPS
   handler** at `0x80076000`, scanning the six-entry table located in guest
   KSEG0 memory at `0x80076710`. Table bytes originate from the 1,856-byte
   game-supplied payload, not manually populated test results.
5. **Six additional guest `SetSyscall` registrations** using the returned
   handler addresses. The syscall registry now supports 512 entries, so the
   game's `0x12C` registration is representable.
6. The genuine `bnezl` branch-likely loop, its taken-path delay slot, return
   register loads, final stack restoration and `JR $ra` to `0x0043FE78`.

Verified installed syscall table:

| Syscall | Guest handler |
|---|---|
| 0x5B | 0x80076000 |
| 0xFC | 0x80076440 |
| 0xFE | 0x80076440 |
| 0xFD | 0x800762A0 |
| 0xFF | 0x800762A0 |
| 0x12C | 0x80076488 |
| 0x08 | 0x800766C0 |

This table is supported by the original copied payload and the code's guest
MIPS load/compare/branch execution, not invented kernel return values.

## Test evidence

- `test_guest_get_entry.c`: seven original-code query trials (six matches and
  one non-match); single-step versus batched parity; retryable KSEG0 memory
  fault; atomic rejection of a corrupted retail opcode.
- `test_boot_post_patch.c`: actual preceding kernel payload installation,
  second Copy, two FlushCache requests, six nested GetEntryAddress executions,
  seven SetSyscall registrations, and return to `0x0043FE78`. One-instruction
  and batched executions finish with identical guest registers, full 32 MiB RAM,
  and kernel registry state. Observed combined stage retired count: 4,119.
- Full opt-in boot diagnostic: **321,851 guest instructions** retired through
  the same return point under an **experimental compatibility kernel**.
- Native-Clang AddressSanitizer and UndefinedBehaviorSanitizer passed for the
  two new regression programs; prior Timer3 syscall-chain and EE syscall-table
  regressions passed.
- New C modules and modified syscall table compile to Windows x64 COFF objects
  with `clang-cl /W4 /WX`. No Windows game binary exists.

## Explicit limitations

The default retail boot diagnostic still stops at the real `SetupThread`
request at 315,897 retired guest instructions. Moving past it uses an opt-in,
experimental kernel policy and synthetic boot-time search data. `FlushCache`
requests are recorded, **not** emulated with cycle-accurate instruction/data
cache or genuine Sony BIOS exception dispatch. The Windows boot diagnostic
requires compilation in a Visual Studio developer environment. No real game
rendering or gameplay has been established.

## Focused Linux commands

```sh
AE3_ELF=/path/to/SCUS_975.01
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_elf_image.c \
  ae3_ee_memory_bus.c ae3_guest_get_entry.c test_guest_get_entry.c \
  -o build/test_guest_get_entry
./build/test_guest_get_entry "$AE3_ELF"

cc -std=c11 -O2 -Wall -Wextra -Werror ae3_elf_image.c \
  ae3_ee_memory_bus.c ae3_ee_timer3.c ae3_ee_syscall_kernel.c \
  ae3_boot_next_init.c ae3_boot_timer3_resume.c ae3_guest_kernel_copy.c \
  ae3_boot_timer3_syscalls.c ae3_guest_get_entry.c ae3_boot_post_patch.c \
  ae3_boot_post_find.c ae3_guest_find_address.c test_boot_post_patch.c \
  -o build/test_boot_post_patch
./build/test_boot_post_patch "$AE3_ELF"
```

Next native boot step: trace the original caller from `0x0043FE78`, expand
actual PS2 kernel/IOP execution interfaces, and work toward authentic boot-time
EE kernel data and hardware behavior. The goal remains a playable native
Windows PC game, not an emulator or a perpetual collection of probes.