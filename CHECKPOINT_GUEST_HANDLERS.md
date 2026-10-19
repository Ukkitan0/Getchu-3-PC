# AE3 original EE syscall registrations and guest FindAddress — 2026-10-18

## Source-bound facts

| Retail PC | Action |
| --- | --- |
| `0x0043FE4C` | `SetSyscall` syscall 0x74, first installing 0x83 -> 0x0043FCC0 |
| `0x0043FD8C` | Retail invokes same wrapper a second time to install 0x5A -> 0x0043FC88 |
| `0x0043FD04` | First 0x83 request is raised by the retail wrapper at 0x0043FD00 |
| `0x0043FCC0` | Original kernel-memory FindAddress handler; performs 32-bit word comparisons |
| `0x0043FDC4` | Next original conditional branch, after two fixture-backed searches |

## New implementation

- `ae3_ee_syscall_kernel.[ch]`: guest-address-only syscall table; atomic registration/lookup, no invented host handler.
- `ae3_boot_hook_init.[ch]`: retires the genuine SetSyscall wrappers, preserves v0 and branch-delay slots, routes the first 0x83 invocation.
- `ae3_guest_find_address.[ch]`: executes the retail 0x43FCC0 instructions including branch-likely and JR delay-slot semantics; faults atomically on missing kernel word, preserving state for retry.
- `test_boot_syscall_registry.c`: real retail ELF and original instruction tests, two synthetic memory searches, one-instruction parity, repeated wrapper routing, invalid inputs and unmapped-memory retries.
- `extract_pcsx2_ee_reference.py`: reconstructs optional user's PCSX2 EE RAM sample without saving it in the repository.

## Observations using user's saved title-screen state

The recorded (post-boot) kernel words return `0x80014F0C` for value `0x0043FCC0` after 21,444 word reads and no match (`0`) for `0x0043FC88` after 131,073 reads. The snapshots were not captured at the original SetupThread call; do not treat this as an authoritative BIOS boot state.

## Boundary of correctness

The *default* boot diagnostic does not synthesize the initial kernel return. The optional test mode uses a synthetic thread stack and semaphore HLE, and halts at the first 0x83 guest syscall. The handler runner will not proceed without explicitly mapped kernel-memory bytes. No GS/IOP/audio/disc/gameplay implementation or playable Windows executable is included.