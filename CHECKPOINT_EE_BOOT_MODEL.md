# Getchu-3-PC — EE startup kernel compatibility checkpoint

Date: 2026-10-18

## What was added

- Native C guest-kernel service model `ae3_ee_boot_services.h/.c`.
- Captures real `SetupThread (0x3C)` and `SetupHeap (0x3D)` guest arguments.
- Validates alignment, disjoint guest stack/heap regions, EE RAM mapping and service ordering.
- Derives a 16-byte-aligned stack pointer from the original retail stack request
  (`0x01FA0000 + 0x00060000 - 0x10 = 0x01FFFFF0`) using an **explicit,
  unverified compatibility policy**.
- Maintains persistent guest-kernel metadata for stack, GP, root callback and heap.
- Commits both guest CPU and host model state together or changes neither.
- Adds `--test-ee-boot-model` to the native diagnostic and corresponding Windows
  build support; default path still halts at authentic unresolved kernel syscall.
- Adds native unit tests, sanitizer checks and a full downstream guest handler
  parity test against the older hardcoded fixture.

## Evidence

- The retail executable SHA-256 must match the original pinned NTSC-U version.
- Original boot arguments: GP 0x00734070, stack 0x01FA0000,
  size 0x00060000, arguments 0x0072D6C0, root callback 0x00100388.
- Original requested heap: 0x00772910, size 0x00010000.
- Both original guest `FindAddress` syscalls execute in the compatibility path;
  they return no match until actual boot-time kernel memory is implemented.
- The default path has NOT gained authentic BIOS execution or gameplay.

## Next milestone

Derive a real boot-time EE kernel image and reproduce the Sony kernel's
`SetupThread` return/side effects from execution evidence rather than a policy.
Then expand MIPS execution beyond `0x0043FDC4` and implement the subsequent
initialization functions and PS2 device services. The executable is not yet
fully decompiled, and there is no playable Windows port.