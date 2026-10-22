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
- **Important follow-on semantics:** Source `0x57C988 DMTC2 $v0,VF1` transfers the **low** GPR doubleword, and `0x57C894` constructs projection operand `$a2` from the source low doubleword. High 64 bits are retained in the native diagnostic/result object, but **must not be interpreted as motion vector Z/W**. The actor pipeline's next ray uses exactly its original low-doubleword source, with upper lanes zero.
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

The result `OriginalActorTargetResult_57c8ec::originalVectorLow64` retains the original post-DMTC2 motion/transfer semantics. The new `originalVectorHigh64` retains packed fallback provenance only; integrator must not blindly concatenate it into the next ray direction.

## Unfinished original behavior / blockers

1. Verify direct native source math under actual actor-update execution with original Q provenance and original owner/controller memory records, not just private stage navigation vertices.
2. Audit original target projection movement loop, complete stage polygon collision/edge cases and player/monkey/environment hit physics against original executable/data; **no replacement collision, synthetic steering or arbitrary substitute**.
3. Float underflow/NaN rules and VU pipeline cycles need comparison against original machine captures, not guessed.
4. Full native Windows build, full CTest, stage load and game boot require integration owner and other parallel workstreams. Not attempted from this branch.

## Suggested next connected pass

Use genuine actor memory snapshots for `0x57C8EC` and `0x57CA68`, then trace original `0x4C1B3C` projection / `0x21BD3C` polygon traversal / `0x21EE00` collision selection into the actual player and monkey actor update. Validate scoring, source winner, raw packed result and next-ray operands against original execution, including zero/NaN/denormal cases. Navigation route search and stage format parsing remain exclusively owned by their respective workstreams.

No original game assets, ELF, ISO, private PCK or Lua are committed. No GitHub Actions triggered. No writes to shared CMake, shared docs, navigation branch, integration/master branch or default main.
