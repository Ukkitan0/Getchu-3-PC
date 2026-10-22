# Original VU0 Mathematics, Collision & Movement — specialist handoff

**Dedicated branch:** `parallel/vu0-physics`
**Starting commit:** `d483436416c14927851853f01c6ad66fe4cfbbf2`
**Integration target:** `native/original-elf-engine-recovery-20261020` (integrator only)
**Status:** Genuine fallback VU0 vector operations implemented and locally tested. No full Windows application build or gameplay boot claimed.

## Completed source recovery and commits

| Commit | Delivered change |
|---|---|
| `cf94d593f047436d40695fc9d3091c7645ab122a` | Recover both original candidate VU0 instruction blocks as direct native C++ and propagate original upper/lower packed fallback GPR bytes through selection/target results; use the production native transform in actor fallback when the original RNG/owner/Q are supplied |
| `0be4b445a095c20347e0a6ad9414f9608bdb42cf` | Add connected native actor-route fallback regression |
| `2ae2becd8d53015c5ab69b52c7777ab71590ad68` | Ensure connected regression starts from independent route state |
| `68b72d599ef03657fdbc6654b481a2bf2d6e10a3` | Add owner-private original ELF instruction / register audit tool |
| `df51478dd21ef081dd2a3d957195bbef64d81cb7` | Correct binary ELF magic check in the audit tool |
| `487e732f133b920aba8f73f8683681958d9826cd` | Forward genuine original VU0 R-side effects into actor/stage interfaces, and preserve the following original DMTC2 low-doubleword semantics |
| `c7634f60524f6ea0af6466c90169b1fa355b90e6` | Strengthen native test to check exact W-lane output, two source-score vector W lanes, and cleared x/y lanes |

**New files (category-owned):**
- `native/include/ae3/original_actor_fallback_vu0_native.hpp`
- `native/src/original_actor_fallback_vu0_native.cpp`
- `native/tests/original_actor_fallback_vu0_native_test.cpp`
- `native/tools/verify_original_actor_fallback_vu0_native.py`

**Modified files (category-owned only):**
- `native/include/ae3/original_actor_fallback_sampler.hpp`
- `native/src/original_actor_fallback_sampler.cpp`
- `native/include/ae3/original_actor_target_dispatch.hpp`
- `native/src/original_actor_target_dispatch.cpp`
- `native/include/ae3/original_actor_navigation_pipeline.hpp`
- `native/src/original_actor_navigation_pipeline.cpp`
- `native/include/ae3/original_stage_actor_navigation.hpp`
- `native/src/original_stage_actor_navigation.cpp`
- `native/tests/original_actor_fallback_pipeline_test.cpp`

The old injectable callback remains available for *tests only* and for unresolved caller implementations. With the source RNG available, the actual production `OriginalActorFallbackNativeVU0_57ca68` is now selected by default; owner +0x14->+0x40 and VU0 Q inputs remain required, not guessed.

## Genuine executable and resource evidence

- Owner-private `SCUS_975.01` SHA-256: `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63` (**verified locally**).
- Entire original fallback `0x57CA68..0x57CD18`, 173 machine words, SHA-256 `6e33608fffc7f8f6980c8eefaee4f856925ba2b0d60fa7fd095dbc8f9c80c886` (**verified locally**).
- `0x57CB4C..0x57CBB8`: 20 original VU0 macro instructions including unique `VMOVE VF2,VF3` at `0x57CB74`; `0x57CC48..0x57CCB0`: 19 operations, no VMOVE. Implemented native VMULx, VADD, VSUB, VMUL, VADDy (xyw mask), VADDz, VMULq in source order.
- Original `PCPYUD` at `0x57CA78` extracts owner vector A1 upper doubleword to `s7`; `0x57CA88` extracts controller+0x20 A2 upper doubleword to `s6`; `0x57CAB0/CAC8/CAD8` extracts source vertex upper doublewords. These are not interchangeable with source vector low doublewords.
- Original `QMTC2` supplies these transformed packed words to the VU0 VF registers. Score passes `VF2,VF3` through original `0x4748AC` XYZ dot at `0x57CBC4/0x57CCBC`. `VMULq` uses **caller-owned Q** at `0x57CB9C/CBB8` and `0x57CC94/CCB0`; no guessed Q initialization.
- Original `VRINIT/VRXOR` pairs occur twice per candidate. Existing native R-bit-state routines are now caller-accessible via the actor and stage adapter. The source still uses 2 genuine game MT19937 RNG draws for each of 6 candidates, distinct from R state.
- Original winner packing at `0x57CCD4/CCE0` is `PCPYUD …,zero,source`: low 64 EE GPR bits become zero and upper 64 retain the winning candidate's VF.z/w bits. The native selection/result now expose *both* raw doublewords.
- **Corrected original semantics:** `0x57C988 QMTC2 $v0,VF1` and the shared `0x57C948 QMFC2` transfer **all 128 bits**. Source `0x57C894 PCPYUD $a2,$zero,$v0` clears the low GPR doubleword and forwards the source **upper** doubleword as the projection Z/W lanes. The previous statement above interpreting this as low-only DMTC2 was incorrect and is superseded.
- Owner-private `woods_c/navigation.pck` SHA-256: `d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0` (**verified locally**); 59 original polygons (39+20), tested without publishing the resource.

## Actual native tests run

**Local isolated focused native candidate computation** using an API-compatible minimal header shim and the direct candidate-transform translation: this is NOT a full checkout or full game build. Source mathematical and packed-lane checks covered 6 candidates × 3 Q patterns × all 59 original PCK triangles plus one synthetic triangle. Compared native Z/W scores and return bit patterns against independently evaluated source-ordered scalar expressions.

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Werror -ffp-contract=off -fno-fast-math \
    -Iinclude src/original_actor_fallback_vu0_native.cpp \
    tests/original_actor_fallback_vu0_native_test.cpp -o test
./test /private/woods_c/navigation.pck

clang++ -std=c++20 -O1 -g -Wall -Wextra -Werror \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -ffp-contract=off -fno-fast-math -Iinclude \
    src/original_actor_fallback_vu0_native.cpp \
    tests/original_actor_fallback_vu0_native_test.cpp -o test_asan
ASAN_OPTIONS=detect_leaks=1 ./test_asan /private/woods_c/navigation.pck
```

- **GCC 14.2.0:** passed focused test, all private PCK triangles.
- **Clang 17 ASan+UBSan:** passed focused test, all private PCK triangles; no sanitizer finding.
- **Private original instruction audit:** passed executable SHA, fallback body SHA, all 39 original VU0 macro names/orders, 7 PCPYUD register encodings, 6 QMFC2 transfers, 4 masked VADDy operations, 4 Q multipliers. Branch audit tool `verify_original_actor_fallback_vu0_native.py` requires user-supplied original ELF as its argument.
- **Full repository native CMake/CTest:** NOT run (no full working-tree checkout and integration-owner-only global CMake changes).
- **Connected actor regression with full category dependencies:** committed for integrator, NOT executed in complete application.
- **Windows executable boot / gameplay:** NOT run; cannot claim native gameplay, collision or level traversal is complete.

Precise FP accuracy caveat: compile without `-ffast-math` and disable contraction. Host float32 underflow, VU denormal/flush behavior, NaN normalization, and instruction-pipeline effects may still differ from the original PS2. No bit-exact hardware comparison across actual dynamic game actor states has been completed.

## Integration request — Windows Runtime owner only

In **`native/CMakeLists.txt`**, add **`native/src/original_actor_fallback_vu0_native.cpp`** to the same library/executable containing `original_actor_fallback_sampler.cpp` / `original_actor_navigation_pipeline.cpp`. Register new focused test `native/tests/original_actor_fallback_vu0_native_test.cpp` and its link dependencies alongside other original-actor tests. No extra third-party dependencies.

Application/actor owner must supply each actor's actual `owner+0x14->+0x40` packed source vector via `setOriginalFallbackOwnerVector_57ca68(...)`, plus actual global/current Q bits through `setOriginalFallbackQBits_57ca68(...)`. If engine-global VI[R] tracking is available, bind its *live* `OriginalActorFallbackVu0R_57ca68` via `setOriginalFallbackRState_57ca68(pointer)` (both pipeline and stage wrapper now expose it). Existing original shared game RNG must be passed to the constructor. None of these original-state dependencies should be replaced with synthetic input or constants.

The full `OriginalActorTargetResult_57c8ec` carries both source-GPR doublewords. Original `0x57C894` uses `originalVectorHigh64` for projection Z/W; the low two lanes are zero. Integrators should invoke `originalActorTargetProjectionOperand_57c894` instead of using the lower 64 bits as a projection target.

## Unfinished original behavior / blockers

1. Verify direct native source math under actual actor-update execution with original Q provenance and original owner/controller memory records, not just private stage navigation vertices.
2. Audit original target projection movement loop, complete stage polygon collision/edge cases and player/monkey/environment hit physics against original executable/data; **no replacement collision, synthetic steering or arbitrary substitute**.
3. Float underflow/NaN rules and VU pipeline cycles need comparison against original machine captures, not guessed.
4. Full native Windows build, full CTest, stage load and game boot require integration owner and other parallel workstreams. Not attempted from this branch.

## Suggested next connected pass

Use genuine actor memory snapshots for `0x57C8EC` and `0x57CA68`, then trace original `0x4C1B3C` projection / `0x21BD3C` polygon traversal / `0x21EE00` collision selection into the actual player and monkey actor update. Validate scoring, source winner, raw packed result and next-ray operands against original execution, including zero/NaN/denormal cases. Navigation route search and stage format parsing remain exclusively owned by their respective workstreams.

No original game assets, ELF, ISO, private PCK or Lua are committed. No GitHub Actions triggered. No writes to shared CMake, shared docs, navigation branch, integration/master branch or default main.

## Continuation: source-correct 128-bit actor target transfer

**New commits:**
- `ecd5a8b98c10ad3de6450ff056048acb52fd9f28`: fix all three source `0x57C8EC` targets to carry the full QMFC2 result; original `0x57C98C` vertex shuffle clears X/Y and blends Z/W; source `0x57C894` forwards Z/W into the next polygon ray.
- `524b63405abfee165227cebd57fa591f90347c66`: new public `originalActorTargetProjectionOperand_57c894` and 128-bit target regression coverage.
- `fccb9dd272856d9b6937773da2759e72c915130f`: typed full 128-bit `originalActorRandomTrianglePacked_57c98c`, called only once per sample to preserve the original RNG stream.

**Source instruction audit:** owner-private SHA-verified ELF; 14 exact words independently checked at `0x57C944`, `0x57C948`, `0x57C984`, `0x57C988`, `0x57C894`, `0x57C994`, `0x57C9A8`, `0x57C9B8`, `0x57CA28`, `0x57CA40`, `0x57CA54`, `0x57CA5C`, `0x57CA64` and `0x57CA60`. The last branches to shared full-width return `0x57C948`. The earlier Q/R fallback body SHA + all 59 private woods_c geometry cases were also rechecked locally. Full CMake/CTest and native Windows boot were not run.

**External ISA cross-check:** PCSX2 `pcsx2/MMI.cpp` PCPYUD explicitly sets `RD.UD[0]=RS.UD[1]`, `RD.UD[1]=RT.UD[1]`; `pcsx2/VU0.cpp` QMFC2/QMTC2 copy *both* 64-bit halves. This is evidence for direct native translation, **not** a dependency on emulation.

**Integration:** Existing sources/headers are already under source ownership; only the new `original_actor_fallback_vu0_native.cpp` needs integrator-controlled CMake addition, along with registering its focused test. The new actor-target projection helper is available to original gameplay integration. Do not cherry-pick the previous fallback commit without these follow-on corrections.


## Continuation: first/subsequent actor projection math recovered

- Commit `1672842d5c2cd644589d309b06136b79a9f64b70`: expanded original executable auditor to verify **30 actual VU0 arithmetic register/operand/mask tuples** beyond the two 39-operation opcode sequences and source branch/transfer sites; all 30 verified locally on private original ELF.
- Commit `9921e54713ca46e87ee0b24138677641907619d6`: original source PCpyud/target propagation assertions expanded for vertex packing and later ray calls.
- Commit `0e736112572f29ede6edc10f3c3aaa6c8ec890f0`: native `originalActorPackedSubsequentRayStart_57c88c` and distinct initial/follow-up A1 path in actor pipeline. Exact original sources `0x57C7E4`, `0x57C814`, `0x57C88C`, `0x57C890`, `0x57C894` audited; local focused GCC and Clang+ASan/UBSan tests both passed on the 59 source woods_c polygons.

**Commands exercised against private PCK with SHA `d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`:**

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Werror -ffp-contract=off -fno-fast-math -Iinclude \\
 src/original_actor_fallback_vu0_native.cpp tests/original_actor_128bit_chain_test.cpp -o test_128
./test_128 /private/woods_c/navigation.pck
clang++ -std=c++20 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined \\
 -fno-omit-frame-pointer -ffp-contract=off -fno-fast-math -Iinclude \\
 src/original_actor_fallback_vu0_native.cpp tests/original_actor_128bit_chain_test.cpp -o test_128_asan
ASAN_OPTIONS=detect_leaks=1 ./test_128_asan /private/woods_c/navigation.pck
```

Also ran the native ray helper and original 0x4C1B3C XYZ math against all 59 original polygons using GCC/Clang ASan+UBSan with the same flags. All focused runs passed. These local test harnesses use isolated API-compatible headers and are **not** equivalent to the full repository native build. Existing repository regression `original_actor_ray_geometry_test.cpp` was also extended with private original per-polygon A1 mask checks. The wider gameplay, stage integration, complete CMake/CTest and full Windows executable still require integration-owner testing.

**Integration:** `originalActorPackedSubsequentRayStart_57c88c` lives in the existing source `original_actor_ray_geometry.cpp` (no new CMake file). `original_actor_navigation_pipeline.cpp` now uses it for follow-up rays. New single-file CMake request remains `original_actor_fallback_vu0_native.cpp`. Do not substitute packed A1 for the initial ray: original first ray retains full caller A1.

## Continuation: mandatory VU0 R side-effect retention

- Commit `13d2ddbc16ad85717dd11561d33f4fa74f85dbff`: pipeline always performs original RINIT/VRXOR for the two fixed source pairs in each of six candidates. When a global caller-owned VU0 R state is supplied, operations affect that shared object. When it is not supplied, a pipeline-owned source VI[R] snapshot retains all effects instead of silently dropping them. `sourceFallbackRState_57ca68()` exposes the current state for integration/diagnostics. The original RINIT from VF0.w overrides the incoming R mantissa, making internal default data irrelevant to the final R bits of this function. External binding is still necessary for actual whole-engine global VU0 R lifecycle.
- The repository `native/tests/original_actor_fallback_pipeline_test.cpp` now checks **both** shared R pointer aliasing and the source 12/12 RINIT/VRXOR operations with no external binding. This connected regression code is committed, but full-tree CTest is not claimed.
- Local-only C++ focused harnesses named `original_actor_128bit_chain_test.cpp` and `original_actor_projection_lane_test.cpp` were compiled and run from an isolated API-compatible test workspace, **not checked into GitHub**. All six test commands (GCC optimized and Clang 17 ASan/UBSan for candidate, 128-bit selection, and two-phase ray) passed using all 59 original private polygon records. The original private ELF SHA and PCK SHA were verified. To run repository-owned test sources, integration must include the new `original_actor_fallback_vu0_native.cpp` in the existing CMake targets.

**End-of-pass branch audit:** `parallel/vu0-physics` is 17 commits ahead of starting `d4834364`, zero behind, with no changes to navigation-team code, global CMake, original/master integration branch, launch scripts or GitHub Actions. No game resources were published. Full native Windows gameplay is unverified.

## Source-native six-candidate winner regression (continuation)

- Commit `9a1f815a538186a931122405f2026291f9562e7f` adds `native/tests/original_actor_fallback_native_selection_test.cpp`, a new repository-owned connected regression. It combines the production `OriginalActorFallbackNativeVU0_57ca68`, the genuine `OriginalGameRandom_36f3c0` stream and `originalActorFallbackSelect_57ca68`. An independent scalar oracle applies each original 0x57CB/CC VU0 Z/W source coefficient in order, Q scaling, original 0x4748AC XYZ scoring and strictly greater winner selection. It checks exact packed high/low winner bits, source RNG cursor and original 12 RINIT/12 RXOR operations over synthetic plus all 59 owner-original woods_c polygon records. No original resource bytes are checked in.
- **Integration-owner CMake request:** add `native/tests/original_actor_fallback_native_selection_test.cpp` as a focused test linked to the existing native actor fallback, VU0 transform, original RNG, source triangle-weight, spatial-vector accessor, and VU0 dot/R-state implementations. The owner-private `woods_c/navigation.pck` is an optional argument; no checkout fixture may contain its bytes.
- **Validation distinction:** previous local focused native candidate and ray/packing math passed GCC and Clang+ASan/UBSan across all 59 original polygons. This newly added complete selector regression has not been run against the entire repository in the current restricted checkout. Its full native integration, CMake/CTest and Windows gameplay remain to be confirmed by the integrator. Do not treat this unexecuted test source as proof of successful integration.
