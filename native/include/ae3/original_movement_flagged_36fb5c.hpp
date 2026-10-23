#pragma once
#include <array>
#include <cstdint>
#include <optional>

namespace ae3 {

// Source 0x36FB5C..0x36FC24 positive movement, with original
// child scratch +0xA8 bit1 already set. Does NOT approximate
// original 0x36F8C8 or alternate 0x36FDB0 tagged-pointer path.
struct OriginalMovementFlaggedInputs_36fb5c {
    std::uint64_t originalChildScratchGuestVA;
    std::uint32_t originalChildStatusA4Bits;
    std::uint32_t originalChildFlagsA8;
    std::uint32_t originalMovementF12Bits;
    std::array<std::uint32_t,4> originalLiveVF1;
};
struct OriginalMovementFlaggedPass_36fb5c {
    std::array<std::uint32_t,4> originalVF1After;
    std::uint32_t originalStatusA4AfterBits;
    std::uint32_t originalFlagsA8After;
    // Actual 36FBF4/36FB4C COP1 f0 return when complete.
    // Absent ONLY if 36FC18 needs genuine 36F8C8 regeneration.
    std::optional<std::uint32_t> originalReturnF0Bits;
    std::uint32_t originalRemainingF20Bits;
    bool requiresOriginal36F8C8;
};

// Pure source transition plan. No game memory is modified until
// the actual game owner publishes VF1 and commits the two source
// child writes. For paths requiring 36F8C8, caller MUST dispatch
// the original full method rather than treating the partial pass
// as a successful movement update.
//
// Restriction: source operands and ALL intermediate COP1/VU0
// arithmetic must be exactly representable as finite binary32;
// PS2 flush/overflow/rounding special modes are not approximated.
OriginalMovementFlaggedPass_36fb5c originalMovementFlaggedPass_36fb5c(
    const OriginalMovementFlaggedInputs_36fb5c& source);
} // namespace ae3
