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

## EE 64-bit sign extension of original parent +0x14

The retail `0x557E68` instruction is
`LW s4,0x14(s1)` (word `0x8E340014`). It loads an
**original signed 32-bit word**, sign-extended into the EE's
64-bit GPR `s4`. Later source `0x557FF0` is
`DADDU a0,s4,zero` (word `0x0280202D`): the full
signed-extended 64-bit register is passed to original
`0x276538`, not a zero-extended `uint32_t`.

The persistent child lifecycle now explicitly sign-extends the
original parent word before providing it to the `0x557F54`
tagged-branch check and to the native `0x557FC8` post-tail.
The post-tail's parent-register argument and
`call_original_276538` owner interface now use `uint64_t`.
An original parent word like `0x81234000` therefore reaches
the source EE callback as `0xFFFFFFFF81234000`. The
tagged-pointer branch of `0x557F90` remains **unrecovered**:
the native post-vector implementation refuses its ordinary
mode path rather than mistakenly accepting a zero-extended
high-bit parent value.

The updated post-vector tests reject both zero-extended and
sign-extended unsupported tagged-parent inputs when the
source tagged comparison is selected. The post-tail regression
uses source child mode `+8 = 0`, which legitimately skips
`0x557F90`, to test preservation of the signed-extended
`0x276538` argument through the original EE DADDU path.
`native/tools/verify_original_movement_handoff_557fc8_558030.py`
pins the original `LW` and `DADDU` words and validates
the native wide interface.

**Still unverified:** a full compiled CTest run and the native
Windows game. These changes correct documented register
semantics; they do not fabricate the unported
`0x276538` engine implementation.

## Source-verified 557ED8 normal-setup EE register ABI

The original normal-setup path precedes the movement metric and
post-tail. The source-verifier map for
`0x317B9C..0x317BD4` and `0x557ED8..0x557F40`
provides these original EE instructions:

| Retail source | Original opcode | Operation |
| --- | --- | --- |
| `317BB0/BB8/BC0/BC4` | `25A201C0/25A20220/25AF0120/25A20140` | 32-bit `ADDIU` resource addresses, sign-extended in 64-bit EE v0 |
| `557EF4` | `0040282D` | `DADDU a1,v0,zero`: complete 64-bit resource address for `276B68` |
| `557EF8` | `0280202D` | `DADDU a0,s4,zero`: complete sign-extended parent word for `276B68` |
| `557F08` | `10400004` | `BEQZ v0`: tests the complete 64-bit result from `276B68` |
| `557F0C/557F14/557F18` | `0280202D` | `DADDU a0,s4,zero`: same 64-bit parent word to `2767F4` and `2773D4` |

The previous native normal-owner callbacks narrowed s4 and
the returned collision v0 to `uint32_t`; they also passed the
resource selection's low 32 bits to `276B68` without
original EE `ADDIU` sign extension.

`OriginalMovementNormalCalls_557ed8` now preserves:
- complete signed 64-bit s4 in all three original manager calls;
- complete signed 64-bit a1 resource address in `276B68`;
- complete original 64-bit `276B68` return through the
  genuine `557F08 BEQZ` branch.

The reusable `originalManagerSelectResource_317b9c`
continues to expose the 32-bit resource-address result, with
original `ADDIU` wrapping; source `557ED8` explicitly
sign-extends that low word for its **64-bit EE argument**.
Both normal setup and `557FC8` post-tail refuse noncanonical
zero-extended high-bit parent s4 inputs before publishing
game-child or owner side effects. The `557E30` lifecycle
receives a low word from genuine parent memory, and performs
the required signed extension before dispatch.

The source-normal-begin CTest now covers: actual first-frame
callback order; high-bit sign-extended parent arguments; a
`276B68` return whose **only nonzero bits are in the upper
32 bits** (which must call `2767F4`); sign-extended high-bit
resource addresses; `ADDIU` low-word wrap; and refusal
of impossible zero-extended caller-provided s4 values with
unchanged child status/flags. Existing movement lifecycle
CTest callbacks were updated to match the new genuine ABI.

A local independent instruction-decoder test of the pinned
opcodes passed **4 ADDIU + 5 DADDU + 1 BEQZ** register/opcode
assertions. The source verifier now also checks these opcodes,
the wide interface signatures and canonical source-data
guards. **The full updated C++ CTest suite and native
Windows game were not executed in this pass.**
