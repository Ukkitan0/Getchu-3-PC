#pragma once
#include "ae3/original_movement_child_30c57c.hpp"
#include <cstdint>
#include <optional>

namespace ae3 {
// Native source 0x36FB18..0x36FB58 original direct-return gates.
// a0 is genuine movement child+0x10, f12 is the original parent
// scale * 0x317B34 metric. Returns exact source COP1 f0 bits only
// on proven early returns; otherwise the real movement step needs
// original 0x36FB5C..0x36FC24 gameplay recovery.
std::optional<std::uint32_t> originalMovementStepFast_36fb18(
    const OriginalMovementChildD0_30c57c& originalChild,
    std::uint32_t originalF12Bits);
} // namespace ae3
