# Original movement child vtable 0x6B0B20: native slot5 and deleting destructor

## Retail source recovered in this pass

The owner-private original USA `SCUS_975.01` executable,
SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`,
contains the actual vtable used by the recovered native
`0x30C57C` movement-child constructor.

The original vtable at **0x6B0B20** gives:

| Original virtual slot | Retail source VA | Role |
| --- | --- | --- |
| 1 | `0x556A0C` | Deleting destructor |
| 5 | `0x557E30` | Child update, invoked through source `0x316174` |

The original slot5 `0x557E30..0x557EB4` first 33 words
are SHA-pinned as
`c19b233ea83358e4af3e6e4e1c17f17e32b27641ae21055c38a051d60785e5e5`.
The original slot1 `0x556A0C..0x556A2C` 8-word
destructor SHA-256 is
`d4c56f66712db7e4411af1dff39c27cd44f4de90eb229dfca65d2258633d9114`.
Its source allocator release at `0x41A3B0..0x41A3D4`
is separately SHA-pinned as
`3cffb07b4b1336059345d14c95001d5bd89ffedb85dd42edfc81b3cd1a6a037d`.

`native/tools/verify_original_movement_child_lifecycle_557e30.py`
checks all three full original windows, vtable slots 0..5,
and individually pinned branch, delay-slot, allocation-release
and destructor instructions against the user's genuine USA ELF.

## Exact source flag-driven update behavior

For source child slot5 `0x557E30`:

1. `0x557E34` checks original `a1 & 1`.
2. Original `0x557E60` loads child `+0x04`, which is the
   **genuine original parent guest address**. The delay slot
   of `0x557E64` always reads parent `+0x14`, **even
   if the bit-zero branch immediately returns**.
   The native code requires an actual original parent-read
   callback instead of skipping or inventing this load.
3. Bit zero set returns **status 1** along the source
   `0x557E80` early-return path.
4. Otherwise `0x557E6C..0x557E78` checks `(a1>>1)&1`;
   bit one set also returns source **status 1**.
5. Neither bit set means the source `0x557E78 BEQZL`
   executes its `0x557E7C` delay-slot read of
   child `+0xC0` and continues into original
   **`0x557EB4+`** normal movement/gameplay behavior.
   That full path is **not** recovered. Native code performs
   the source child field read, then **refuses** instead
   of substituting a fake updated status.

The original deleting destructor `0x556A0C` first
sets base vtable `0x6B0C40`, then tail-jumps to
source `0x41A3B0`, which invokes the original
allocator-release path. Its native typed counterpart uses
the same object-ownership release semantic through
`std::unique_ptr`; no guest VA is interpreted as a
Windows function or allocator pointer.

`native/src/original_movement_child_lifecycle_557e30.cpp`
implements these **source-proven, flag-driven** cases for an
actually owned `OriginalMovementChildD0_30c57c`. It plugs
straight into already translated `0x316174` and
`0x3161B8`: original update global depth increments,
source vtable slot5 returns status one, original
controller `+0x14` stores one, slot1 destroys
the child and controller `+0x10` is cleared.

`OriginalMovementChildLifecycle557E30` is the new
native CTest target exercising both source argument bits,
the required parent-delay-slot load, real child deletion,
zero-flag gameplay refusal, unknown-parent failures,
invalid child ownership and unchanged source update depth
on errors. An isolated equivalent of this path passed
local GCC and Clang C++20 under UndefinedBehaviorSanitizer.

## Remaining blockers

**Not a complete child update:** original `0x557EB4`
and later normal actor processing contains further
source calls (including `0x36F954`, `0x317B9C`),
actor-vector work, animation and collision. They
cannot be replaced with the early-completion logic.
Persistent Player/Monkey creation and a full original
retail stage frame are still not implemented.

The full repository CTest and the complete Windows/Unreal
Editor DLL are unverified. No proprietary original game
files or source binary data are committed.


## Normal zero-flag child completion now recovered

The original `0x557EB4` path previously had no native
implementation when both update flag bits were zero.
The owner-private USA ELF verifies a source-specific normal
completion case that can now execute **without inventing the
unrecovered collision/animation methods**:

1. Original `0x557E78 BEQZL` executes a delay-slot
   read of child byte `+0xC0`. Retail `0x30C57C` initially
   writes this byte to zero.
2. If `+0xC0 == 0`, `0x557EB4 BNEZL` is not taken.
   Original `0x557EC0` passes `child+0x10` to
   `0x557EC8 JAL 0x36F954`, loading exact original
   `f12 = 2.0f` from source `0x704B04` in its delay slot.
3. Source `0x36F96C LWC1(a0+0xA4)` consequently reads
   original **child+0xB4**, which is the `0x36F5F4`
   source scratch value at `+0xA4` transferred through
   the fully recovered `0x30C57C` constructor.
4. Original `0x36F994 C.OLE.S` tests
   `1.0f <= child+0xB4` (threshold from original
   `0x7157B4`). If true, the next source
   `0x36F9A0 C.OLT.S` tests `0.0f < f12`,
   true for this original `f12 = 2.0f`. The original
   helper returns source value **1** at `0x36F9AC`.
5. Original `0x557ED0 BNEZ` takes that success branch;
   its delay slot sets `v0 = 2`, and slot5 returns
   original controller status **2**. The already
   recovered native `0x3161B8` then stores 2 in
   controller `+0x14`, invokes source child slot1
   destructor `0x556A0C`, and clears child `+0x10`.

The native `originalMovementChildTerminal36F954` checks
exactly this source terminal condition and only after proving
the original child `+0xB4` word was initialized. Missing
provenance and unsupported special floating-point values
are rejected. If the source value is below the terminal
threshold, or the original child `+0xC0` is nonzero,
**no synthetic child update is returned**: those original
gameplay branches still require source recovery.

Original source-window SHA-256 fingerprints:

- `0x36F954..0x36F9C4` (28 words):
  `674fae1d67260580966da93648ee600a3c16b706c713eeff15374a519a5bbc66`
- `0x557EB4..0x557ED8` (9 words):
  `679c219dcbf330b0c75dc53ab409783d96787535e07e5168c0b527d50ccad993`

These plus 26 original instruction sites and both source
constants were checked against the user's exact USA ELF.
`native/tools/verify_original_movement_child_early_36f954.py`
performs these checks independently.

New native CTest targets:

- `OriginalNormalChildCompletion36F954` checks zero-flag
  source status2, missing byte provenance, threshold boundary,
  child destruction, unknown lower-status gameplay refusal,
  and original parent-read behavior.
- `OriginalNormalMovementVUChildLifecycle` runs the complete
  already recovered source chain: genuine EE vectors →
  source VF2 scalar → scratch field `+0xA4` →
  source `0x30C57C` child → original `0x36F954`
  terminal condition → original slot5 status2 →
  `0x3161B8` source child deletion. Far-distance
  inputs are correctly rejected as unrecovered gameplay.

Both C++20 native tests passed locally with GCC and Clang,
strict compilation and UndefinedBehaviorSanitizer. The
complete Windows/Unreal runtime remains unverified.
