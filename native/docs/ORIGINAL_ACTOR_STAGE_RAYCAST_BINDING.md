# Ape Escape 3 original actor controller -> native polygon raycast -> stage records

## Source-correct operand recovery

The previous actor pipeline called its unresolved projection callback with
the controller's target-selection vector at **+0x20**. That is not the
vector consumed by original `0x4C1B3C`, so it prevented the recovered
native raycast from being connected correctly.

The original user-owned USA ELF `SCUS_975.01` (SHA-256
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`)
shows three independent inputs:

1. **Incoming `$a1`** to original `0x57C7CC`. At `0x57C7E4`,
   the original source saves this packed register; at
   `0x57C88C` it restores it for later raycasts. This
   is the moving source actor's original incoming ray operand.
2. **Original controller +0x40**. At `0x57C810`, the original
   first `0x4C1B3C` call gets its packed `$a2`
   vector directly from controller offset **+0x40**.
3. **Newly generated target vector**. After calling original
   `0x57C8EC` at `0x57C878`, source `0x57C894` constructs
   the next packed `$a2` vector from the returned **64-bit
   `$v0`**, with original high-half clearing. It is **not**
   the original controller +0x40 vector. It is not controller +0x20.

The same source `0x57C8EC` calls `0x21B788` with the independent
controller **+0x20** vector for its polygon eligibility predicate.
Keeping these distinct resolves a real integration error rather
than inventing an actor steering policy.

## What is connected in native C++

`OriginalActorNavigationPipeline_57c7cc` now accepts actual original
navigation adjacency and the two original ray arguments, and uses the
recovered `originalPolygonRaycast_4c1b3c` directly for projection.
The new method `setOriginalProjectionOperands_57c7cc` updates
source incoming `$a1` and original controller +0x40 each actor tick.
The initial projection uses controller +0x40; later projections
use the original `0x57C8EC` packed output. It resets the
latter state at the start of every new `advance()`.

The legacy callback path remains only when no original source
adjacency is bound, to preserve older isolated tests. When genuine
adjacency is supplied but the two source ray operands are missing,
the native version **throws**, rather than silently using the
wrong +0x20 vector or inventing an origin.

`OriginalStageActorNavigation_57c7cc` connects directly to the
existing source stage loader:

```text
real original stage resource
 -> originalStageBuildSourceSpatial_21d46c
 -> OriginalResolvedSpatialGeometry source vertex/header bindings
 -> originalStagePolygonLinks_21bd3c
 -> OriginalActorNavigationPipeline_57c7cc
      -> originalActorAdvanceRoute_57c7cc
      -> 0x4C1B3C original normalized ray
      -> 0x21BD3C polygon traversal/adjacency recursion
      -> 0x21EE00 edge clipping
      -> 0x4748AC XYZ dot product
      -> 0x57C8EC target-vector dispatch
      -> 0x57C98C original game RNG triangle target (when bound)
```

This wrapper maintains original stage-owned record identities, owns
the original polygon vertex views and adjacency, and stores the
current controller +0x50 target between original actor updates.
It refuses alien target pointers, invalid stage polygon indexes,
and mismatched source ownership. It does not simulate guest RAM or
create any replacement gameplay systems.

## Tests and original ELF audit

The new
`native/tests/original_stage_actor_navigation_test.cpp`
exercises the stage-owned integrated call chain against both
authentic woods_c navigation graphs: all **59 original polygons**,
all **137 original edges** and **15 source type-two edges**. For every
genuine polygon, it verifies two source actor updates: a successful
initial projection that preserves the active route target and skips
RNG work, followed by a mismatching target that updates controller
+0x50 and computes a new vector using genuine source vertices and
the recovered original game RNG.

The isolated native integration test was run with GCC Release,
Clang Release, and AddressSanitizer+UndefinedBehaviorSanitizer
using the owner's private original PCK. The local isolated test
used stage-data decoding adapters implementing the same on-disk
original descriptor/vertex format; the **full current repository
CMake build and Windows program boot remain unverified**.

The verifier `native/tools/verify_original_actor_projection_binding.py`
SHA-checks original source routines at `0x57C7CC`, `0x4C1B3C`
and `0x21BD3C`, totaling **202 original instruction words**,
checks 12 source opcode anchors and six direct original JAL edges,
and verifies genuine private `woods_c` source graph counts.

## Limits

`0x57CA68` still contains original fallback vector geometry
and has not been fully ported. Full actor state/animation/render
updates and booting the original game on native Windows remain
unfinished. The current work is a source-backed original engine
component, **not** proof of a playable native game.

Do not add the copyrighted ELF or PCK to the public repository.
No MIPS/PS2 emulator was implemented, and no GitHub Actions
were manually triggered.
