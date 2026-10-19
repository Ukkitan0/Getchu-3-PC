# Getchu-3-PC — second retail constructor and numeric helper prefix

2026-10-19. Experimental bounded native C EE execution. **Not a playable Windows game.**

## Retail execution evidence

The original static constructor dispatcher at `0x00428AE8` invokes its second
pointer from guest RAM, `0x00417C58` (the second of 143 recovered callbacks).
The original entry builds the `(a0,a1)=(1,0xFFFF)` arguments, restores the
stack, and tail-jumps to `0x00417B50`. That routine calls the original
`0x004173B0` two-instruction return stub **twice**. The following `JAL`
passes the actual guest double-precision bits `0x4024000000000000` (10.0)
through `0x0041D6B8`, whose actual MIPS `J` targets `0x0041EFD0`.

- `ae3_boot_second_constructor.[ch]` executes **51 original MIPS instructions**
  from the second callback at the dispatcher through the numeric routine entry.
- `ae3_boot_second_numeric.[ch]` executes **46 more original instructions**
  through the double-bit classification and original bit arithmetic.
- The path stops **before** the original `JAL` at `0x0041F118`, which targets
  `0x00423E20`. The target and opcode are verified directly from guest memory.
  No numeric result, initialization side effects of that callee, or constructor
  return are fabricated. The second constructor is **not yet completed**.
- Cold first-constructor fixture: previous `329,376` instruction counter plus
  97 independently tested original instructions = **329,473** counted in
  experimental composed boot state, **not** an authentic BIOS-to-game trace.
- 82 second-constructor opcode addresses (including two dispatcher sites)
  and 83 numeric-routine opcode addresses were cross-checked against the
  owner's 32-MiB post-boot EE memory snapshot. Repository audit totals
  **877/877** pinned opcode entries. The checkpoint excludes all game data.

## Verification performed

`test_boot_second_constructor.c` prepares the actual prior constructor state
using the earlier bounded native modules. It compares stepping and batching
for the complete 97-instruction path, including full 32-MiB guest RAM parity;
checks the original caller RA and double bits; mutates guest instructions and
verifies atomic opcode rejection; injects unmapped instruction and both
constructor/numeric stack-store faults; and confirms retry without state
mutation. GCC `-O2 -Wall -Wextra -Werror`, Clang ASan/UBSan and clang-cl
`/W4 /WX` Windows x64 COFF object builds pass. The combined Linux boot
probe links with the two new modules. Earlier first constructor, constructor
dispatch, TLB copy and guest memset focused regressions pass.

The real retail executable `SCUS_975.01` was **not** available for an
integrated game boot test in this checkpoint, and the imported post-boot RAM
snapshot is opcode evidence rather than authoritative fresh-boot data.

## Next boundary

Implement and verify the original callee `0x00423E20` and its return from
`0x0041F118`, then the second numeric operation and guest global stores at
`0x006A6678` and `0x006A6680`. Only after both branches of the second
constructor are executed and its stack is restored should the constructor
dispatcher advance to the third callback `0x003A6CD0`.

True Sony BIOS startup, EE/IOP thread and syscall scheduling, VU/GS graphics,
audio, controllers, and the entire playable Windows port are still missing.