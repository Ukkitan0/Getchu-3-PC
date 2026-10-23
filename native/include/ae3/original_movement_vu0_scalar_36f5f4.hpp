#pragma once
#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <span>

namespace ae3 {
// Source-local decoding of original 0x36F5F4..0x36F664 VU0 macro
// operations, specifically COP1 f1 at 0x36F698. Original GPR
// a1/a2/a3 are true 128-bit source EE registers packed by 0x57C140,
// 0x57C144, and 0x57C14C. This is NOT a VU emulator or a replacement
// for original VF3/VF4/VF5 side effects and other branch destinations.
struct OriginalMovementVU0ScalarSource_36f5f4 {
    std::array<std::byte,16> originalA1;
    std::array<std::byte,16> originalA2;
    std::array<std::byte,16> originalA3;
};
struct OriginalMovementVU0ScalarResult_36f5f4 {
    // Original VF1.x DMFC2 -> COP1 f1 at 0x36F638, compared at
    // 0x36F640 with source 1.0f. First VU vector pair A2 - A1.
    std::uint32_t originalFirstLengthSquaredBits;
    // Original VF2.x DMFC2 -> COP1 f1 at 0x36F664, compared at
    // 0x36F698 with source 1.0f. Second pair A3 - A1.
    std::uint32_t originalSecondLengthSquaredBits;
};
// The input vectors must be genuine game-source 128-bit EE values.
// Handles finite ordinary input lanes that do not require PS2 VU0
// denormal/overflow/emulated flag behavior. Rejects values outside
// this supported source arithmetic range; never guesses a status.
OriginalMovementVU0ScalarResult_36f5f4 originalMovementVU0Scalars_36f5f4(
    const OriginalMovementVU0ScalarSource_36f5f4& genuineSource);

// Source 0x36F64C..0x36F68C VU0 vector side effects for ordinary
// source float lanes. VF7 is zeroed by 0x36F668/0x36F66C PPACW
// on original t5=0 before 0x36F674 QMTC2. The source branch
// chooses A2 (large first squared distance) or (A1+A3)*0.5.
// Original VF1/VF2/VF3/VF4 are returned as four raw float words.
// This has a stricter input contract than the scalar-only helper:
// all four VU lanes, including W, must be valid ordinary floats.
struct OriginalMovementVU0VectorFrame_36f5f4 {
    OriginalMovementVU0ScalarResult_36f5f4 scalar;
    std::array<std::uint32_t,4> originalVF1;
    std::array<std::uint32_t,4> originalVF2;
    std::array<std::uint32_t,4> originalVF3;
    std::array<std::uint32_t,4> originalVF4;
    // Original 0x36F614 explicitly loads a3 into VF5 and
    // 0x36F604 loads the same a3 into VF6; neither is overwritten
    // on the 0x57C148 first-return path. Source 0x36F674 writes
    // a full zero VF7. These must survive for an actor VU owner.
    std::array<std::uint32_t,4> originalVF5;
    std::array<std::uint32_t,4> originalVF6;
    std::array<std::uint32_t,4> originalVF7;
    bool originalFirstLengthUnderOne;
};
OriginalMovementVU0VectorFrame_36f5f4 originalMovementVU0VectorFrame_36f5f4(
    const OriginalMovementVU0ScalarSource_36f5f4& genuineSource);

// Complete, source-bounded 0x57C148 -> 0x36F5F4 first-return
// vector/scalar path on ordinary source EE float lanes. It preserves
// original VF1/VF2/VF3/VF4 outputs and publishes only the four
// scratch words that 0x36F5F4 actually writes. Source stack bytes
// not proven initialized remain UNKNOWN in sourceKnownBytes; the
// retail 0x30C57C constructor must preserve this provenance.
OriginalMovementVU0VectorFrame_36f5f4
originalMovementVU0FirstReturn_36f5f4(
    std::span<std::byte,0x110> sourceScratch,
    std::bitset<0x110>& sourceKnownBytes,
    std::uint64_t originalGuestStackA0,
    const OriginalMovementVU0ScalarSource_36f5f4& genuineSource);

// Source callsite 0x57C148, end-to-end supported scalar subpath:
// genuine EE a1/a2/a3 -> original COP2 Vf2.x -> COP1 f1 ->
// original 0x36F6A8 conditional 0x110 scratch tail. The complete
// VU VF3/VF4/VF5 pipeline still requires separate source recovery.
// On unsupported numeric/callsite inputs, original scratch remains
// unchanged. Source a0 is the original GUEST stack VA, not host ptr.
void originalMovementScratchFromVU0Scalar_36f5f4(
    std::span<std::byte,0x110> sourceScratch,
    std::uint64_t originalGuestStackA0,
    const OriginalMovementVU0ScalarSource_36f5f4& genuineSource);
} // namespace ae3
