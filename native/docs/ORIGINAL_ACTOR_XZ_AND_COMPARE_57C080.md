# Ape Escape 3 USA original actor X/Z proximity and raw vector comparison

**Workstream:** VU0 Mathematics, Movement & Collision  
**Branch:** `parallel/vu0-physics`  
**Source:** Owner-private USA `SCUS_975.01`  
**Original ELF SHA-256:** `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`

These are fixed original instruction translations into native C++, **not** an EE/VU emulator or replacement movement engine. The input vectors must come from the original actor/controller state and original helper results.

## Original 0x57C07C–0x57C0E8 X/Z movement proximity

The complete 28-instruction machine-code range SHA-256 is
`b8bd2a93fc986aa2b848782734ea73fb0d3842f51bd7056e06e6fa35eeba171a`.

| Source address | Source effect and native translation |
|---|---|
| `0x57C07C` | Original route-present `PCPYUD s3,zero,v0` after external `0x21B854`: saved S3 low doubleword = 0, saved high doubleword = V0 high |
| `0x57C080/0x57C088` | Full 128-bit `QMTC2 s3,VF1` and `QMTC2 s4,VF2` |
| `0x57C090` | `VSUB.xyzw VF1,VF1,VF2` in the original operation order |
| `0x57C094–0x57C0A4` | `QMFC2`, `PEXTLW t6,t6,t7` with original `t6=0`, `PCPYUD t7,t7,t7`, `PCPYLD t7,t7,t6`, `QMTC2 t7,VF3` -> full source VF3 is **(difference.x, +0, difference.z, difference.w)** |
| `0x57C0A8` | `VMUL.xyzw VF1,VF3,VF3`; retains squared W in the source VF1 but final distance does not use it |
| `0x57C0B8/0x57C0BC` | `VADDy.xyw` then `VADDz.xyzw`: final X lane is `(dx*dx + 0.0f) + dz*dz` |
| `0x57C0C0–0x57C0D8` | `QMFC2` low FPU operand, `MTC1`, `SQRT.S`, compare strict `C.OLT.S` against original float `0x708A44=0x40000000` (2.0f) |
| `0x57C0DC–0x57C0E8` | `BC1FL` conditionally zeroes the original register flag when comparison is false; `ANDI`/branch choose the original near/far continuation |

New functions:

```cpp
OriginalActorXZProximity_57c080 originalActorXZProximity_57c080(
    const std::array<std::byte,16>& originalSavedS3,
    const std::array<std::byte,16>& originalSavedS4) noexcept;
std::array<std::byte,16> originalActorPack21B854Return_57c07c(
    const std::array<std::byte,16>& originalResultV0) noexcept;
```

The source value `s3` has two distinct origins. With the original controller `+0x1C` route flag, source `0x57C070/74` first calls the as-yet-unrecovered original `0x21B854` and applies the `0x57C07C` pack. Without the route, `0x57C274` supplies `s3` from current original `VF3`. **Do not pretend these are interchangeable, infer a missing `0x21B854` result, or pass the controller+0x20 vector in its place.** The caller must pass actual `s3` and `s4` source values. The new native code does not call either unrecovered path or invent side effects of the subsequent `0x316174` and `0x441F40` calls.

## Original 0x57C1D4–0x57C210 controller vector equality return

The 16-instruction block SHA-256 is
`1c2f033d74ea08980d19bd8a54cd5f7ddc0d1eba4861af8b774df17fced955c3`.

- Source `0x57C1D4/0x57C1D8` checks controller variant flag `+0x30`. If zero, the source returns code **2** without comparing vectors.
- Source `0x57C1E0/E4` loads all 128 bits of both controller+0x20 and controller+0x40.
- Source `0x57C1E8 PCEQW` performs **four unsigned/raw word equalities** producing 0xFFFFFFFF or 0 in each original 32-bit lane.
- Source `0x57C1EC PPACH` packs low 16 bits of all four comparison results into the lower GPR doubleword; `0x57C1F0 NOR` and `0x57C1F4 SLTIU` return truth only when all four original 32-bit words are identical.
- `0x57C1F8 BEQZL` with its delay-slot behavior and `0x57C210 MOVZ` produce original result code **2** when equal and **1** when different.

New function:

```cpp
OriginalActorControllerCompare_57c1d4 originalActorControllerCompare_57c1d4(
    bool originalControllerVariant30,
    const std::array<std::byte,16>& originalControllerVector20,
    const std::array<std::byte,16>& originalControllerVector40) noexcept;
```

This is a **bitwise comparison**, not float32 `==` (which would incorrectly conflate +0/-0 and treat bitwise-identical NaNs as different). The original caller reaches this conditional only after its original `0x3161B8` dependency succeeds, its action-kind condition at `0x57C1A4` is 2, and the separate multi-polygon branch at `0x57C1AC` does not take control. Do **not** unconditionally call this helper for every actor tick. No original gameplay transition beyond this source block is claimed.

## Verification

Private original instruction audit:
`native/tools/verify_original_actor_xz_and_compare_57c080.py` checks the original ELF SHA, both complete source block SHA-256s, 38 exact opcode sites, five branch destinations, the original constant, four masked VU0 operand/register patterns and five MMI register/operand patterns.

The owner-private `woods_c/navigation.pck` SHA-256 is
`d2f6809cb9414cf8edfe52bc4c209d526b6247c003fd60856a796f114b422df0`.
Both native tests execute against all **59 real original polygons**, using their authentic 16-byte vertices. The XZ test checks the genuine masked lanes, SQRT/threshold, source 0x57C07C return packing and many original polygon vertex pairs. The controller test checks all sixteen source bytes, comparisons of each original polygon's vertices, signed zero, raw NaN patterns and original variant-disabled return code; its independent `PCEQW/PPACH/NOR` oracle does not use the production comparison's `memcmp` result.

Executed locally with:

```bash
# from a native source-root containing the original files
g++ -std=c++20 -O2 -Wall -Wextra -Werror -ffp-contract=off -fno-fast-math \
 -Iinclude src/original_actor_xz_proximity.cpp \
 tests/original_actor_xz_proximity_test.cpp -o xz_test
./xz_test /private/woods_c/navigation.pck

clang++ -std=c++20 -O1 -g -Wall -Wextra -Werror \
 -fsanitize=address,undefined -fno-omit-frame-pointer \
 -ffp-contract=off -fno-fast-math -Iinclude \
 src/original_actor_controller_vector_compare.cpp \
 tests/original_actor_controller_vector_compare_test.cpp -o compare_asan
ASAN_OPTIONS=detect_leaks=1 ./compare_asan /private/woods_c/navigation.pck
```

Both focused regressions passed under **GCC 14** and **Clang 17 ASan+UBSan**. This is a focused native-source test, **not** an integrated Windows game boot.

## Integration-only CMake request and blockers

Only Windows/runtime integration owner should add these already-owned files to the original actor math target:

- `native/src/original_actor_xz_proximity.cpp`
- `native/src/original_actor_controller_vector_compare.cpp`

Register:
- `native/tests/original_actor_xz_proximity_test.cpp`
- `native/tests/original_actor_controller_vector_compare_test.cpp`

No third-party libraries, new runtime, or emulation dependencies are required.

**Outstanding original dependencies:** `0x21B854` origin/transform; `0x316174` and `0x441F40` external movement state updates; `0x3161B8` type-2 engine transition; actual live controller+0x20/+0x40 and saved EE s3/s4 snapshots; original end-to-end actor update and collision world; PS2 VU unusual NaN/denormal behavior; full Windows CMake/CTest/game boot.

Neither the original executable nor original resources are committed; no GitHub workflow is triggered.
