#pragma once
#include <array>
#include <cstdint>

namespace ae3 {
// Retail original USA 0x274988..0x274A04: EE 128-bit
// PEXTLW/PCPYUD/PCPYLD pack and VU0 vector normalization.
// Original window SHA256:
// 9190ac5270ec67c8803b7b4b08c8276b95c459c4386be2890748b5c79b9474f5
//
// This is a DIRECT NATIVE translation of the original
// exactly representable binary32 subset, NOT a VU emulator.
// Caller supplies the original live VF1 because VADDy.xyw
// preserves its Z lane before VADDz.xyzw updates it.
// No data is manufactured from an Unreal/host transform.
struct OriginalMovementNormalizeInputs_274988 {
    std::array<std::uint32_t,4> originalEEStackA1;
    std::array<std::uint32_t,4> originalLiveVF1Before;
};
struct OriginalMovementNormalizeResult_274988 {
    std::array<std::uint32_t,4> originalVF2Packed;
    std::array<std::uint32_t,4> originalVF1BeforeThreshold;
    std::array<std::uint32_t,4> originalVF1After;
    std::uint32_t originalLengthSquaredBits;
    std::uint32_t originalLengthBits;
    // Absent when original epsilon compare refuses normalization.
    // Source 2749E0 loads +1.0, then 2749EC DIV.S f2=1/length.
    bool originalPerformedNormalize;
    std::uint32_t originalReciprocalBits;
};
// Matches ALL original MMI/VU0 lane operations and the COP1
// threshold/normalization branch if each arithmetic instruction
// produces an exact ordinary finite binary32 result.
// Rejects unverified PS2 non-IEEE/rounded/special behaviors;
// does not publish VF1 or change any game-owned register itself.
OriginalMovementNormalizeResult_274988
originalMovementNormalizeExact_274988(
    const OriginalMovementNormalizeInputs_274988& realOriginalRegisters);
} // namespace ae3
