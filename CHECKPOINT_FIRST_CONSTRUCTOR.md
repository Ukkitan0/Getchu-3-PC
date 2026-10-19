# Getchu-3-PC — original first constructor executed

2026-10-19 — **experimental native C EE startup only; NOT a playable game**.

## Retail execution

The original `0x00428AE8` `JALR $t6` now invokes the first table callback
at `0x0041A390`. This constructor sets `(a0,a1)=(1,0xFFFF)` and executes
its real tail jump to `0x0041A088`, not a host-language substitute.

The original body performs twelve `LD`/`DADDIU`/`SD` reference-count
increments in guest RAM. Its first-call condition is **old count = 0**, not
an arbitrary Boolean flag; it checks whether the incremented count equals
one. For each previously-zero counter it calls the original two-instruction
`JR $ra; NOP` stub at `0x004173B0`, which is present in the retail image.
The last helper call is a tail jump after restoring the saved registers.

The `JALR` delay slot updates the constructor-table pointer. The guest
stack is restored by the original instructions, and the dispatcher executes
its `LW`, `BNE`, and delay slot to reach the next retail callback:

- `0x00428AE8` -> `0x0041A390` -> `0x0041A088`.
- All twelve reference-count fields at guest `0x0072D5A8`..`0x0072D658`
  are accessed at their **actual distinct, nonsequential addresses**.
- Explicit cold fixture (all twelve counts zero): **225** guest MIPS
  instructions retired, **12** original counter stores, **12** calls to the
  guest `0x004173B0` return stub.
- Explicit warm fixture (all twelve counts one): **164** guest MIPS
  instructions, **12** stores (each count becomes two), **0** helper calls.
- The original dispatcher loops to `0x00428AE8` with the second target
  `0x00417C58` prepared in `$t6`; the second callback remains unexecuted.
- A separate fixture with a `0xFFFFFFFF` terminator in the second table
  slot takes the real done branch to `0x00428AFC`.

These counts start from the actual bounded constructor-pending state in the
standalone fixture. They are **not** evidence of a complete authentic
PS2 BIOS startup. No arbitrary guest opcode handling or BIOS response
is invented.

## Evidence and tests

- `ae3_boot_first_constructor.[ch]` implements only the exact pinned PCs,
  opcodes, register writes, branch-delay slots and 32/64-bit bus accesses.
- `retail_first_constructor_words.tsv` adds **208** exact original
  instruction-address/word rows from the owner's post-boot 32 MiB EE
  memory snapshot (which is not redistributed). `verify_opcode_snapshot.py`
  verifies **712 / 712** repository pinned rows against this reference.
- `test_boot_first_constructor.c` exercises cold, warm, and immediate
  terminator paths; step-one-vs-batched CPU and RAM parity; invalid opcodes;
  missing instruction mapping; and atomic recovery from a guest flag-store
  memory fault.
- GCC `-O2 -Wall -Wextra -Werror` and Clang ASan+UBSan pass.
- Windows `clang-cl /W4 /WX /O2` produces an x64 COFF object.
- Existing static-dispatch, TLB installer, post-TLB memset and main-return
  regressions pass. The Linux diagnostic links with **45 C source modules**.
- No full retail-ELF diagnostic or native Windows executable execution was
  available for this checkpoint. A Windows COFF object is not a playable game.

## Next boundary

The next original constructor is `0x00417C58`, tail-jumping to the original
routine at `0x00417B50`. Execute it with the actual caller state and side
effects, then continue through the remaining constructor table. Rendering,
IOP, audio, input, authentic BIOS startup and gameplay are still absent.