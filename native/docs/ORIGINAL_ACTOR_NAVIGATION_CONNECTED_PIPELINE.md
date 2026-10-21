# Original Ape Escape 3 actor navigation: CONNECTED native route pipeline

This recovers a connected game code path rather than a standalone
navigation helper. The input is the original decoded source stage
`OriginalSpatialRecord70` array and its original `OriginalSpatialSourcePolygon`
source vertex spans, produced by the existing owner-asset loader.

## Call chain connected in C++

```text
Original 0x57C7CC route advancement
  |-- original controller+0x1C single/multi-route branch
  |-- route[1] initial target; route[2..] candidate scan
  |-- original 0x57C8EC target calculation per selected polygon
  |     |-- variant controller+0x30
  |     |-- original controller+0x20 LQ in a1 at 0x57C928
  |     |-- original 0x21B788 plane eligibility, exact f12=0.1
  |     |     '-- ALREADY RECOVERED native originalVU0PolygonEligible_21b788
  |     |-- eligible -> original controller+0x20 low64 passthrough
  |     |-- ineligible -> still-untranslated 0x57CA68 VU0 transform
  |     '-- variant zero -> source 3 vertices and unresolved 0x57C98C transform
  '-- next-polygon acceptance -> still-untranslated 0x4C1B3C projection
```

This new `OriginalActorNavigationPipeline_57c7cc` connects the previously
isolated original actor route follower, target-vector dispatch,
and fully recovered original `0x21B788` VU0 polygon eligibility.
The source controller+0x20 vector is decoded without altering its
floating-point bits, and is the original query vector passed to
`0x21B788` by original machine instruction `LQ a1,0x20(s0)`
at `0x57C928` (raw `0x7A050020`).

Previous passes modeled the `0x21B788` eligibility as an unresolved
callback inside the actor target dispatcher. **This pass eliminates
that callback for any actor bound to actual original stage polygon
records.** It reuses the already source-backed function
`originalVU0PolygonEligible_21b788`. The high-fidelity source vertex
and query layout are preserved.

Three genuine, still-untranslated VU0 dependencies are deliberately
exposed through `OriginalActorUntranslatedMath_57c7cc`:
- `0x4C1B3C`: project current actor against next polygon.
- `0x57CA68`: fallback vector geometry after eligibility fails.
- `0x57C98C`: random three-vertex target selection for variant-zero.

Their temporary test callbacks return fixed results **only** for testing
the source branch and pointer ordering. The actual native game MUST
supply fully recovered original implementations, and may not silently
use this test code as gameplay logic.

## End-to-end tests on genuine stage navigation resources

`native/tests/original_actor_navigation_pipeline_test.cpp`
verifies multi-target advancement and 0x21B788 pass/fail branches,
original direct stored-vector passthrough, the explicit fallback
and three-vertex unresolved calls, stage pointer ownership, and
correct ordering of target calculations and projections.

When configured with the owner's private
`AE3_RETAIL_WOODS_C_NAVIGATION_PCK`, it decodes the **real two
woods_c navigation graphs** (39+20 polygons), uses their actual 16-byte
source vertices, and runs the connected original actor route ->
eligibility -> target dispatch on **all 137 real navigation edges**,
including the 15 original type-2 action links.

The owned USA `SCUS_975.01` SHA-256 remains:
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
Original `woods_c/navigation.pck` SHA-256 remains:
`d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`.

Local GCC Release, Clang Release, and UBSan tests of this
connected native call graph passed (local test linked the recovered
original source eligibility math in isolation).

## Remaining native game milestone

Reimplement original 0x4C1B3C, 0x57C98C and 0x57CA68 VU0 math;
connect the source controller result to original actor state update,
animation, and rendering. Full Windows boot is still absent.
This change is a step toward source engine integration, not a
substitute gameplay engine, MIPS emulator, or synthetic game demo.
No original copyrighted assets are committed and no GitHub
Actions were manually dispatched.
