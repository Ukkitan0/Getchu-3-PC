#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace ae3 {
// Original 0x57C1D4..0x57C210: the *terminal* return decision on the
// already-selected type-2 actor branch when controller+0x1C is zero.
// LQ vectors at controller+0x20/+0x40, PCEQW, PPACH, NOR, SLTIU,
// BEQZL/MOVZ. All four 32-bit lanes must have IDENTICAL bits.
// Original caller reaches this only after 0x3161B8 succeeded and
// original action-kind==2; neither external call is implemented here.
struct OriginalActorControllerCompare_57c1d4 {
    bool originalBitwiseEqual=false;
    std::uint32_t originalReturnV0=0;
};
OriginalActorControllerCompare_57c1d4 originalActorControllerCompare_57c1d4(
    bool originalControllerVariant30,
    const std::array<std::byte,16>& originalControllerVector20,
    const std::array<std::byte,16>& originalControllerVector40) noexcept;
}
