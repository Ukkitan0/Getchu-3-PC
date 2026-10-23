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


## October 23 continuation — full original 557FC8 tail integrated

A separate contributor recovered the **26-word original
`0x557FC8..0x558030` control block**, source hash
`aecf9edaedeeea8a27a5987bed707c47e9210d57ca411f95bfd1503fec0df6b1`,
plus the source `0x558084..0x558090` flag0 alternative.
`native/tools/verify_original_movement_post_tail_557fc8.py` pins
the original MIPS instructions and the three 128-bit POR operations.
The native translation is
`native/src/original_movement_post_tail_557fc8.cpp`.

The persistent `OriginalMovementChildLifecycle_557e30` now provides
an **optional preferred direct-tail owner**
`OriginalMovementPostTailOwner_557fc8`, after successful publication
of source `0x557F54` VU0 results. The original `0x557FC0`
early status1 bypasses this tail; otherwise the direct path:

1. Reads the **genuine original EE guest scratch address** from the
   original owner, not from the host pointer allocation.
2. Executes original `0x557FC8` bit0 dispatch, including
   `0x558084 -> 0x36F83C` VU0 generation when bit0 is clear.
   Its actual 128-bit source return goes into EE a1; if bit0 is
   already set, source `LQ child+0x90` supplies EE a1 instead.
3. Calls original `0x30C0A0`, which supplies **actual original s5**,
   then original `0x276538` with the entire original 128-bit s3
   from `0x557F8C DMFC2 VF2`.
4. Reproduces the original `0x6354E7 / 0x6354E0` conditional
   gate and optionally invokes original `0x36FC28`.
5. Executes the native source `0x558030` return using the
   exact `f20` resource metric from `0x557F48` and
   `f21` movement return from `0x557F60`.
   It no longer requires an independent, caller-invented s5.

The previous `OriginalMovementPostStepContinuation_558030` path
is retained **only for compatibility** with genuine external
owners that already execute the intermediate original call chain;
it is not run if a direct-tail owner has been supplied. Its
source f20/f21 must match the exact earlier native registers
or the state is refused without a controller status write.

The direct-tail integration is compiled into the core native
actor-manager source set and registered in the **six pre-existing lifecycle CTest targets**, plus a
new dedicated two-frame original `0x557FC8` lifecycle regression,
for **seven** lifecycle-linked targets total.
Extended lifecycle regression source includes first-frame
flag0 regeneration, repeated nonterminal status0 frames, the
original optional `36FC28` gate, rejected invalid guest scratch
VA, and prohibition of duplicate legacy execution. Additional
static verifier:
`native/tools/verify_original_movement_handoff_557fc8_558030.py`
checks prior source opcode pins, actual callsites and all seven
lifecycle-linked CMake targets.

**Boundary:** `0x30C0A0`, `0x276538`, `0x36FC28` and
`0x314C44` are still **external original-engine methods**
implemented only through required owner interfaces. Their
game logic and collision behavior must be recovered next.
The full changed C++ CTest and native Windows executable are
**not verified** in this handoff; no proprietary original bytes
were committed.

## 64-bit EE `s5` source-register ABI fix

The original source contains two *64-bit* EE `DADDU` instructions,
not 32-bit `ADDU` operations:

| Retail PC | Original word | Exact register transfer |
| --- | --- | --- |
| `0x557FEC` | `0x0040A82D` | `DADDU s5,v0,zero` |
| `0x558070` | `0x02A0282D` | `DADDU a1,s5,zero` |

Before this correction, the native owner signature
`call_original_30c0a0` returned `uint32_t`, and the
`OriginalMovementPostStep_558030::originalS5From30C0A0` /
`call_original_314c44` interfaces consumed `uint32_t`.
That **truncated the upper 32 bits of source EE s5**, potentially
corrupting the real original second argument to `0x314C44`.

These interfaces now use `std::uint64_t` end-to-end. Source
`0x558054` separately reads the first argument with an actual
32-bit `LW`, so that callback field **remains** `uint32_t`.
The original `a1` 128-bit `POR` argument to `0x30C0A0`
also remains a full four-lane array; `DADDU` then consumes
the low 64 bits of that function's **returned** `v0`.

Extended high-bit regression fixtures return
`0x0012345678450000` through `0x30C0A0`, then verify
the complete value arrives at `0x314C44` without narrowing.
Tests were extended in the standalone `558030`, standalone
`557FC8` and both persistent-lifecycle CTest programs.
The source verifier
`native/tools/verify_original_movement_handoff_557fc8_558030.py`
decodes both original opcodes as 64-bit `DADDU`, checks the
actual source/destination register numbers, and fails if the
native signatures regress to 32 bits.

The source instruction-word decoder was checked directly in this
session: `0x557FEC` yields opcode `0`, rs `v0` `2`,
rd `s5` `21`, function `0x2D`; `0x558070` yields
rs `s5` `21`, rd `a1` `5`, function `0x2D`.
The updated C++ tests still require an actual compile and CTest
run. No private game binary or original resource data were
committed.
