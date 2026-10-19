## 2026-10-19: first original static constructor executed (NOT PLAYABLE)

Executed real `0x00428AE8` JALR into `0x0041A390`, tail jump to `0x0041A088`,
and the original twelve `LD/DADDIU/SD` reference counter updates. Cold
zero-count fixture: **225** original guest instructions and 12 actual helper
stub returns; prior-initialized counts: **164** instructions and no helper
calls, but still 12 incrementing guest stores. The dispatcher reaches the
**second original callback `0x00417C58`** pending at `0x00428AE8`.
**712/712** pinned retail instruction rows match the memory reference;
GCC, Clang sanitizers, Windows x64 object and linked boot diagnostic pass.
The original ELF end-to-end boot, BIOS and playable Windows port remain
unfinished. See `CHECKPOINT_FIRST_CONSTRUCTOR.md`.

# Checkpoint — native retail static constructor dispatcher

This is a native C **boot diagnostic, NOT a playable game**.

Latest experimental execution: `0x0010033C` JALR to retail initializer
`0x00628210`, two original nested calls, then constructor dispatcher at
`0x00428AB8`. The first real constructor call awaits execution at
`0x00428AE8 -> 0x0041A390`. The owner-provided *later-game* RAM snapshot
contains **143 distinct constructor pointers** terminating at `0x0062E660`.
The runtime never substitutes or automatically skips their effects.

See `CHECKPOINT_STATIC_CONSTRUCTORS.md`, `TEST_RESULTS_STATIC_CONSTRUCTORS.txt`,
`retail_constructor_addresses.tsv`, and `verify_constructor_snapshot.py`.

Run standalone tests (requires C compiler, not game files):

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror ae3_ee_memory_bus.c ae3_boot_static_dispatch.c test_boot_static_dispatch.c -o build/test_boot_static_dispatch
./build/test_boot_static_dispatch
```

For original-ELF boot diagnostic, supply your own unmodified
`SCUS_975.01`. Authentic BIOS/IOP/rendering/input/audio/gameplay remain missing.