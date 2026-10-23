# Ape Escape 3 USA — native original movement child 0x558030 lifecycle continuation

## Source evidence and exact limits

The owner-private original USA `SCUS_975.01` is the authoritative executable
(SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`).
This integration uses the **previously recovered and instruction-pinned**
original source sequence `0x558030..0x558084` (21 instructions,
SHA-256 `e4d59bf362eda8e908c4ffc4167674760b73b59f6d78710be2e9efddc9cfc3e4`).
The corresponding verifier is
`native/tools/verify_original_movement_post_step_558030.py`.

**Not recovered by this integration:** the intervening original
`0x557FC8..0x55802C` actor/collision work and its source
`0x30C0A0/0x276538` dependencies. A native owner **must independently
establish** that work and supply actual source entry state; merely reaching
`0x557F54` does not imply that those instructions have run.

## Delivered connected native controller path

The existing `OriginalMovementChildLifecycle_557e30` controls the
real original child (vtable `0x6B0B20`, slot 5 `0x557E30`).
It already executes `0x557ED8` setup, `0x557F34`
resource/movement, and opt-in source `0x557F54` VU0 vector handling.
After source `0x557F54` publication, **only if it did not return
early status 1**, an additional opt-in continuation now invokes the
existing genuine `originalMovementPostStep_558030` translation.

The new interface is
`OriginalMovementPostStepContinuation_558030`, extending
`OriginalMovementPostStepOwner_558030`, with a strictly
provenance-bearing method
`read_original_post_step_after_557fc8(child)`.
The method may return `std::nullopt` when the intervening original
engine methods have not executed or the real COP1/S5 source state is
not available. No substitute collision, S5, f20 or f21 values are
created. The persistent child lifecycle refuses the frame when the
original continuation cannot be established.

The native `0x558030` block then executes these source actions:

- `child+0xB4 >= 1.0f`: exact ordinary `DIV.S f21/f20`,
  source `0x55806C` parent `+0x70` store, return slot-5
  original status **2**. The existing `0x3161B8` manager then
  records status 2 and deletes the child using its genuine
  deleting-destructor lifetime.
- `child+0xB4 < 1.0f`: source
  `0x558050/0x558054` branch-likely parent `+0x1C` read,
  original `0x314C44` dispatch using verified S5 and
  child `+0x09` mode, and return slot-5 original status
  **0**. The existing `0x3161B8` manager records status 0
  and leaves the same child alive for its next normal frame.

If `0x557F54` output was not available/published, `0x558030`
cannot be invoked via this path. Any original owner exception
propagates and `0x316174` restores the original shared
update depth, while `0x3161B8` avoids falsely storing a completed
controller status.

## Integration and tests

Changed code:

- `native/include/ae3/original_movement_post_step_558030.hpp`
  — conditional original owner interface.
- `native/include/ae3/original_movement_child_lifecycle_557e30.hpp`
  — optional constructor interface.
- `native/src/original_movement_child_lifecycle_557e30.cpp`
  — post-vector/post-movement transition and proof boundary.
- `native/tests/original_movement_post_vector_lifecycle_test.cpp`
  — original manager status-0 re-entry, missing original upstream
  state, absent VF publication, and failed `0x314C44` owner.
- `native/unreal_bridge/CMakeLists.txt`
  — required translation unit linked to all six source
  lifecycle CTest executables. Original actor manager library
  already compiled the source `0x558030` file.

The existing `OriginalMovementPostStep558030` unit regression
separately exercises source terminal status 2, parent `+0x70` store
and original `0x314C44` status-0 behavior.

**Verification in this pass:** GitHub read-back confirmed the
source and six lifecycle CMake linkage additions. No original
ELF or source data were added to the repo. The complete project
test suite, Windows/Unreal build, and original executable runtime
were **not run**. These new tests have been committed and need
compilation/execution before their outcomes can be claimed.

## Remaining connected blockers

1. Recover original `0x557FC8..0x55802C` collision and actor
   methods and original `0x30C0A0/0x276538` inputs, removing
   the requirement for an external verified continuation owner.
2. Complete iterative `0x36FB70/0x36FC18/0x36F8C8`
   movement, PS2 COP1/VU0 rounding and tagged-address
   branches without silently substituting IEEE host behavior.
3. Bind original player/monkey actor owners and animation to a
   native Windows stage update and actually execute the retail
   game frame. **Not playable yet.**
