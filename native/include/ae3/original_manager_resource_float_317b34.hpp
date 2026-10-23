#pragma once
#include <cstdint>
#include <optional>

namespace ae3 {
// Original 0x317B34..0x317B9C is 26 MIPS32 words.
// This source implementation reproduces only its TWO
// complete non-VU return branches. Parent+0x08 is an original
// guest table VA, NEVER a host pointer. The owner supplies
// genuine original table words only when their source offset
// is actually read. Status/coord computation for nonzero
// mode with parent+0x40!=1 remains original VU0 work.
struct OriginalManagerResourceFloatFields_317b34 {
    std::uint32_t originalParentWord08;
    std::uint32_t originalParentWord40;
    std::optional<std::uint32_t> originalFloatAt374Bits;
    std::optional<std::uint32_t> originalFloatAt3B4Bits;
};

// Returns the RAW original COP1 f0 bit pattern (no host float
// conversion). Source parent+0x40==1 -> table+0x374;
// otherwise mode low byte zero -> table+0x3B4.
// Nonzero mode requiring original VU0 must throw.
std::uint32_t originalManagerResourceFloatBits_317b34(
    const OriginalManagerResourceFloatFields_317b34& original,
    std::uint32_t originalA1Mode);
} // namespace ae3
