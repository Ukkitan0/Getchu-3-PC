# Ape Escape 3 USA — original 0x57CA68 fallback RNG and best-of-six selection

## Scope: actual game routine, not replacement AI

The owner-original USA `SCUS_975.01` SHA-256 is
`5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
The owner-original `woods_c/navigation.pck` SHA-256 is
`d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`.

The original actor target dispatcher at `0x57C97C` calls
`0x57CA68` if the variant-1 `0x21B788` plane predicate
fails. This routine constructs a sequence of random candidate
targets, scores them using original VU0 vectors and
original `0x4748AC` XYZ dot, and returns the best.

Source `0x57CA68..0x57CD18` contains **173 original
machine instructions**.

## Newly recovered original operations

1. Three original source `0x21B458` vertex reads at
   `0x57CAA8`, `0x57CAB8`, `0x57CACC` obtain the
   original polygon's three 16-byte game vertices.
2. The FIRST candidate uses `0x36F4D4` at `0x57CADC`
   and `0x36F548` at `0x57CB00`, drawing two values
   from the **real recovered game RNG**. The original FPU
   constants at `0x708A70/74/78` are exactly
   `1.0f / 0.8f / 0.066666663f` and match the already
   recovered `originalActorTriangleWeights_57c98c`.
3. Original `0x57CBC4` scores the initial candidate via
   `0x4748AC` and `0x57CBCC` initializes the best score.
4. Source `0x57CBBC` initializes signed `s2=4`.
   Loop `0x57CBD4..0x57CCDC` decrements at
   `0x57CCB4` and executes while `s2>=0`.
   Therefore it makes **five more** candidates, not four:
   **six candidates and twelve total RNG calls**.
5. Each additional candidate uses source calls at
   `0x57CBD8` / `0x57CBFC`, then original
   `0x4748AC` scoring at `0x57CCBC`.
   Original `0x57CCC4` uses `C.OLT.S(best,score)`:
   replacement happens only on **strictly greater** scores.
   Ties and unordered (NaN) comparisons preserve the
   earlier best. The candidate pointer/packed result is
   updated at `0x57CCD4`.

The native `originalActorFallbackSelect_57ca68` now
implements ALL of this source selection/control flow:
original three-vertex reads, exact original RNG stream,
6 candidate loop, source XYZ dot, strict best selection,
and packed winner.

## Integration

`OriginalActorNavigationPipeline_57c7cc` now accepts an
optional `OriginalActorFallbackVU0Transform_57ca68` with
the original shared RNG. On failed source eligibility, the
native stage actor pipeline runs the real recovered
best-of-six selection directly; its output is forwarded to
the original `0x57C8EC` branch result. The stage-owner
wrapper `OriginalStageActorNavigation_57c7cc` forwards
the same original-specific transform when supplied.

**One major dependency remains unresolved:** the original
VU0 candidate transforms spanning
`0x57CB4C..0x57CBB8` and
`0x57CC48..0x57CCB0` are NOT translated. They transform
the random triangle weights, source vertices, and actor
reference vector into the original scored candidate VU
registers. This implementation explicitly requires the
`call_original_57ca68_candidate_VU0` interface for that
operation. Its test implementation supplies known scores
**only for validating original branching and RNG behavior**,
not as a replacement for the game's movement math.
Do NOT wire the test callback into gameplay.

When that source-specific implementation is absent,
the existing unresolved geometry callback remains
visible and cannot silently become synthetic steering.

## Verifications

`native/tools/verify_original_actor_fallback_sampler.py`
SHA-verifies the entire **173-instruction** original
fallback body, checks 12 independent opcodes/constants,
nine direct source call edges, and the original five-loop
branch destination. It independently audits the genuine
59 polygon / 137 edge / 15 kind-two-link source data.

`native/tests/original_actor_fallback_sampler_test.cpp`
checks exact six-candidate RNG call ordering and weights
for multiple seeds, strict score replacement vs ties and
NaNs, and the complete original woods_c polygon set.

`native/tests/original_actor_fallback_pipeline_test.cpp`
also verifies the variant-1 actor target branch actually
enters the recovered native fallback selector and forwards
the best result, instead of invoking the older placeholder
callback. GCC, Clang, AddressSanitizer and
UndefinedBehaviorSanitizer isolated tests passed.

This is source-backed native recovery toward the actual
original engine. The full Windows game remains unbooted;
the complete CMake project remains unverified. No game
ELF/ISO/Lua/assets or emulator code are committed. No
GitHub Actions were manually run.
