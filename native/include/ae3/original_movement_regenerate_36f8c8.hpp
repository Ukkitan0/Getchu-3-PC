#pragma once
#include <array>
#include <cstdint>

namespace ae3 {
// Source 0x36F8C8..0x36F954, ONLY original untagged EE
// pointers and exact, finite binary32 VU0/COP1 operations.
struct OriginalMovementRegenerateSource_36f8c8 {
    std::uint64_t originalChildScratchGuestVA;
    std::uint64_t originalResourcePointerRegA0;
    std::uint32_t originalStatusA4Bits;
    std::uint32_t originalFlagsA8;
    // Source VU0 registers VF1,VF2,...VF8 at method entry.
    // Genuine original state; VF4 is overwritten by QMTC2.
    std::array<std::array<std::uint32_t,4>,8> originalVF1ThroughVF8;
};
struct OriginalMovementRegenerateResult_36f8c8 {
    std::array<std::uint32_t,4> originalVF1After;
    std::array<std::uint32_t,4> originalVF4After;
    std::array<std::uint32_t,4> originalACCFirstStage;
    std::array<std::uint32_t,4> originalACCAfter;
    // Original DMFC2 $v0,VF1 in the JR delay slot.
    std::array<std::uint32_t,4> originalReturnV0;
    std::uint32_t originalFlagsA8After;
};
OriginalMovementRegenerateResult_36f8c8 originalMovementRegenerate_36f8c8(
    const OriginalMovementRegenerateSource_36f8c8& source);
} // namespace ae3
