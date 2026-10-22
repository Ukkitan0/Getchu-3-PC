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

## Latest continuation — complete original type-two actor movement transform

The **original `0x57BDCC–0x57BED4` native VU0 transform is now recovered** as a connected operation. The source is the owner's private USA `SCUS_975.01` (SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`). The 67-instruction body has SHA-256 `7974c5e0c777b060f0e55c0c99c8fca7bb0d1df91b84772486d8c49dbe76136b`.

### Committed changes

- `58862811068524a1cd84d0e6165aca98021139b3`: extend the **existing original 0x21EE00 polygon clipping regression** with genuine float32 epsilon `0x358637BD` boundary, sign-of-zero under negative division, strict clipped interval equality and W-lane invariance across all original 177 directed-edge probes. No collision algorithm replacement.
- `c6e1568719324f58140d886f49f181d1a42a5407`: new category-owned `native/include/ae3/original_actor_type2_vu0_target.hpp`, `native/src/original_actor_type2_vu0_target.cpp`, `native/tests/original_actor_type2_vu0_target_test.cpp` and `native/tools/verify_original_actor_type2_vu0_57bdcc.py`. Original 0x57BDCC/BDDC/BDF0 three vertex reads, 0x57BDD4/BDEC/BDFC PCPYUD extraction, original 0x36F4D4/36F548 two RNG draws, original 0x57BE8C/BEAC/BEC0 three VMULx and 0x57BEC8/BECC two VADD. Original 0x57BED0 branch executes **0x57BED4 SQC2 VF3,+0x40(s4)** in the delay slot, writing the entire result to the original actor controller vector.
- `8c6942f7915029eae0f63168bae4e3e3511459ef`: source instruction and integration documentation `native/docs/ORIGINAL_ACTOR_TYPE2_VU0_57BDCC.md`.
- `f3636311a0a277d889c4ca5fdf6fed3b18245fea`: extend the new native regression to feed the actual navigation-workstream `originalActorType2Target_57bdb8` *read-only* polygon selector into native physics writer. Checks actual second route record identity, unmodified original vertex bytes, exact 16-byte stored controller+0x40 and correct game RNG cursor on all **15 kind-two links**.

### Why native reuse is original, not invented

The source VU0 arithmetic in `0x57BE8C–0x57BECC` and original constants `0x708A24/28/2C` are the same as the already recovered native `0x57C98C` random-three-vertex transform and constants `0x708A64/68/6C`. The original code uses the same two RNG calls and the same sequence of three VMULx + two VADD. The new adapter calls `originalActorRandomTriangle_57c98c` **once**, then writes all 16 bytes of the resulting VU0 vector to caller-owned controller storage. No simulator, procedural steering, duplicate RNG draws, or generic physics.

**New native engine interface:**

```cpp
OriginalVU0Vec4 originalActorType2VU0WriteController40_57bdcc(
    const std::array<std::array<std::byte,16>,3>& originalSourcePolygonVertices,
    OriginalGameRandom_36f3c0& originalSharedGameRandom,
    std::array<std::byte,16>& originalControllerVector40) noexcept;
```

Navigation-owned `native/src/original_actor_type2_target.cpp` remains unchanged. On original source action-kind == 2, the gameplay/integration owner obtains `OriginalActorType2Target_57bdb8::rawOriginalVertices` from the existing navigation API and passes these to the VU0 writer with the genuine shared game RNG, and the live controller+0x40 vector. This does **not** establish a whole actor tick or gameplay loop.

### Genuine instruction/resource evidence and executed tests

- ELF original SHA verified; 67-instruction block SHA verified; original action-kind == 2 gate `0x57BDA4/A8`; 5 original direct JAL destinations; 3 PCPYUD source vertex extracts; 5 VU0 operation/register/mask encodings; 6 source float constant words and exact SQC2 destination/store at `0x57BED4` audited from owner-private source. `native/tools/verify_original_actor_type2_vu0_57bdcc.py` provides reproducible audits requiring only the owner's private original ELF.
- Source `woods_c/navigation.pck` owner-private SHA `d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`. Focused original-native source-equivalent GCC 14.2 `-std=c++20 -O2 -Wall -Wextra -Werror -ffp-contract=off -fno-fast-math` test and Clang 17 `-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer` test both **passed**. Evaluated all **59 polygons**, **137 source links**, **15 original kind-two action links** with four deterministic RNG seeds; verified independent float32 scalar Z/W coefficient calculation, source X/Y zeros, two RNG draws and exact full-width SQC2 controller storage.
- An additional connected focused C++ harness included original navigation workstream's unmodified `originalActorType2Target_57bdb8` source with the native VU0 writer; GCC and Clang with ASan/UBSan both **passed** on all 15 genuine type-two source links.
- Original 0x21EE00 native edge clip plus 0x21BD3C polygon traversal source-equivalent focused GCC and Clang ASan/UBSan tests also **passed** on 59 interior polygon rays, 177 directed-edge exterior rays, and all 137 source links. Exact IEEE signed zero, retail epsilon boundary, strict interval equality and W-lane invariance focused checks passed, with the extra repository test now committed.
- These are focused native C++ builds with source-equivalent, API-compatible local dependency headers and owner-private resources. **Full repository CMake/CTest and original native Windows gameplay boot were not performed and must not be claimed.** PS2 VU denormal/NaN hardware differential traces and live full actor-controller state remain unverified.

### Integrator-only build request

Add **`native/src/original_actor_type2_vu0_target.cpp`** to the existing original actor math library/executable in `native/CMakeLists.txt` (integrator only), alongside the earlier pending `native/src/original_actor_fallback_vu0_native.cpp`. Register both `native/tests/original_actor_type2_vu0_target_test.cpp` and `native/tests/original_actor_fallback_native_selection_test.cpp` as test targets, linking the genuine `original_actor_random_triangle.cpp`, `original_game_random.cpp`, spatial selector accessor, original navigation-owned `original_actor_type2_target.cpp`, source fallback/math and dependent VU code. Never fabricate owner/controller state or change navigation graph-search code to accommodate the native physics interface.

**Remaining:** actual original actor tick wiring, full native collision world and player/monkey/environment interactions, original live Q/R and source actor-vector capture, float32 VU edge-case differential, full Windows native build and game boot. No proprietary game files were committed, no global CMake/master/navigation modifications or GitHub Actions triggered.


## 2026-10-22 continuation: original actor 2-unit distance gate plus type-2 branch

**Source evidence**: Original private `SCUS_975.01` SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`. Instruction block `0x57BCDC..0x57BD54` = 31 words, SHA-256 `c93c29dad7891ab3e2af3a5ad39e9cccc2eebd5d89604e762f043e167dc9fcf5`. Original `0x708A20` float32 constant = exactly `0x40000000` (2.0f). Checked branch sources/destinations, original `LQC2` register operands, `VSUB.xyzw`, both `QMFC2` inputs to `0x4748AC` dot, `SQRT.S`, strict `C.OLT.S`, and the original two controller field-zero stores. Full body plus 20 independently verified opcode words and source branch targets pass `native/tools/verify_original_actor_distance_gate_57bcdc.py` locally on the original private ELF.

**Code committed**:
- `native/src/original_actor_distance_gate.cpp`, matching header/test: original conditional controller+0x30 distance check, full owner+0x14→+0x40 / controller+0x20 source vectors, original XYZ dot, sqrt and exact strict threshold; source 0x57BD3C zeroing of controller+0x08 and +0x0C. The variant-disabled path returns no computed length instead of a fabricated result.
- `native/src/original_actor_type2_movement_bridge.cpp`, matching header/test: original distance gate → conditional early reset → original controller+0x1C route gate → kind-2 selection via **read-only navigation-owned** `originalActorType2Target_57bdb8` → original source record at +0x50 → original VU0 target output at controller+0x40. Other source actor branches explicitly return `OriginalOtherBranchUnrecovered`; they are NOT replaced.
- `native/tools/verify_original_actor_distance_gate_57bcdc.py`, original SHA/instruction auditor.
- `native/docs/ORIGINAL_ACTOR_DISTANCE_GATE_57BCDC.md`, full instruction-by-instruction provenance and integration contract.

**Actual tests executed in local focused C++ source workspace**: GCC optimized and Clang 17 ASan+UBSan passed for both new tests, against all 59 original `woods_c` triangles and all 15 kind-2 source links (PCK SHA-256 `d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`). Gate tests strict equality and near-threshold values, NaN, disabled variant and W independence. Connected test validates early reset/no RNG use, other-branch no mutation, and original type-2 action under both variant-enabled/far and variant-disabled cases. Both read genuine private resources only at test runtime. `original_actor_type2_movement_bridge_test.cpp` uses the actual already-recovered navigation source accessor without modifying its code. Original source auditor also passed. **Full native CMake/CTest and Windows gameplay were not run.**

**Integrator-only CMake and API request**: Compile `native/src/original_actor_distance_gate.cpp` and `native/src/original_actor_type2_movement_bridge.cpp` with the original actor physics objects; register `native/tests/original_actor_distance_gate_test.cpp` and `native/tests/original_actor_type2_movement_bridge_test.cpp`. Link original `original_actor_ray_geometry`, `original_actor_type2_vu0_target`, `original_actor_random_triangle`, `original_game_random`, `original_actor_type2_target` (navigation-owned) and original spatial accessor. Pass real controller+0x30/+0x20/+0x1C/+0x08/+0x0C/+0x40/+0x50 state, original owner+0x14→+0x40, original action kind/route, and evolving global game RNG. Do not invent owner state or silently emulate unrecovered actor paths.

No original ELF/PCK/ISO/game assets published, no cross-branch edits, no GitHub Actions dispatched. Native Windows gameplay and original VU0 hardware bit-exact edge cases remain unverified.

## 2026-10-22 continuation — two original actor movement branches recovered

**Source:** owner-private USA `SCUS_975.01`, SHA-256 `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`. The genuine original `0x57C07C–0x57C0E8` (28 words) SHA-256 is `b8bd2a93fc986aa2b848782734ea73fb0d3842f51bd7056e06e6fa35eeba171a`, and `0x57C1D4–0x57C210` (16 words) SHA-256 is `1c2f033d74ea08980d19bd8a54cd5f7ddc0d1eba4861af8b774df17fced955c3`. The private ELF is SHA-verified and the 44 original instruction words, 38 explicit opcodes, source branch targets, original 2.0f constant, four VU0 register/mask patterns and five distinct MMI patterns were audited. No source executable bytes are committed.

**Implemented native originals:**

- `native/include/ae3/original_actor_xz_proximity.hpp` + `native/src/original_actor_xz_proximity.cpp`: source route-present `0x57C07C PCPYUD` pack and source `0x57C080–0x57C0E8` full 128-bit QMTC2, VSUB, QMFC2, `PEXTLW → PCPYUD → PCPYLD` X/Z projection (VF3 = `{diff.x,0,diff.z,diff.w}`), VMUL/VADDy/VADDz source distance reduction, SQRT.S and strict comparison to actual `0x708A44=0x40000000`. The Y difference and W lane are preserved as original intermediate data but do not contribute to the source squared XZ distance. The result exposes the genuine movement proximity boolean, squared distance and original source packed vector.
- `native/include/ae3/original_actor_controller_vector_compare.hpp` + `native/src/original_actor_controller_vector_compare.cpp`: source `0x57C1D4–0x57C210` controller variant+0x30 check, raw 128-bit `LQ` at +0x20/+0x40, `PCEQW → PPACH → NOR → SLTIU → BEQZL/MOVZ` exact bitwise equality return. Original return code = **2** on variant-disabled or variant-enabled/all 4 words equal; **1** on variant-enabled/any source word differs. No floating-point equality substitution.

**Commits:**
- `73577196b0187f9f2d0aa7a1f01e7e629c525494`: new source math, original EE pack and original controller comparison.
- `bd7d4ad7c977892226204d77ce9e553ef961e623`: new focused native polygon regressions for both functions.
- `2c40e8e7a560c57b000e0c312e1d9070d2d826be`: dedicated SHA/opcode/branch original-ELF audit `native/tools/verify_original_actor_xz_and_compare_57c080.py`.
- `09f028004ab73dd6b0b9741a01d9d697c61b0ee4`: independently reconstruct 4× PCEQW word results and PPACH packing for the controller test oracle; no tautological `memcmp` in the oracle.
- `c607c632d1c4e1df35464a2aeacb0d432c8904c9` and `55179f15fd13e955380ef62c460265fc3c57e534`: exact tested-source formatting and naming cleanups.
- `42d88947f319b532bcf6bb5b248e667fdb869c70`: detailed original source and integration document `native/docs/ORIGINAL_ACTOR_XZ_AND_COMPARE_57C080.md`.

**Actual focused test runs in the current private environment:** Both `original_actor_xz_proximity_test.cpp` and `original_actor_controller_vector_compare_test.cpp` were compiled directly from matching original native source with GCC `-std=c++20 -O2 -Wall -Wextra -Werror -ffp-contract=off -fno-fast-math` and Clang `-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -ffp-contract=off -fno-fast-math`. Both binaries ran successfully against the original owner-private `woods_c/navigation.pck` SHA-256 `d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`: 59 original polygons; four original source-vertex pairs per XZ polygon, 16 byte-mutation checks per original vertex in the raw-equality test, source signed zeros, two NaN patterns, exact 2.0f boundary and variant-disabled behavior. The independent packed-word equality oracle additionally passed both compiler/sanitizer modes. This is **focused native code**, not full repository CTest or original Windows game execution.

**Integration-owner CMake request:** Add `native/src/original_actor_xz_proximity.cpp` and `native/src/original_actor_controller_vector_compare.cpp` to existing native actor-math target, register the two dedicated tests and call their interfaces only at their actual original source sites. Both new source units have no third-party dependencies. No changes to navigation graph/search code, global CMake, master, other specialists, workflow or private resources.

**Crucial unrecovered ownership inputs:** Original `0x57C07C` path requires the genuine output of unrecovered `0x21B854` (or source original VF3 saved by the alternative `0x57C274` no-route path) and the actual 128-bit `s4`. The raw controller comparison only occurs after original `0x3161B8` success, action-kind 2 and its original controller+0x1C conditional. Neither dependency is invented by these functions. Follow-up movement/collision side effects at `0x316174`, `0x441F40`, `0x30C57C`, `0x21DB64` remain unrecovered. Native Windows CMake/CTest and boot, actual live actor/controller snapshots and PS2 VU denormal/NaN differential are **not verified**. No claim of playable native game.
