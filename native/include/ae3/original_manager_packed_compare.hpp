#pragma once
#include "ae3/original_manager_child.hpp"
#include <array>
#include <cstddef>
#include <stdexcept>

namespace ae3 {
// Shared source semantics of four-lane PCEQW + PPACB/SLTIU in
// 0x57C1E0, 0x57C508 and 0x57C5D4. PCEQW compares raw 32-bit
// lane patterns; do NOT use floating-point == or epsilon tests.
// In particular +0.0 != -0.0 and unrelated NaN payloads differ.
inline bool originalManagerPackedPceqwEqual_57c1e0(
    const std::array<std::byte,16>& original20,
    const std::array<std::byte,16>& original40) noexcept {
    for (std::size_t lane=0; lane<4; ++lane) {
        for (std::size_t byte=0; byte<4; ++byte) {
            const auto offset=lane*4+byte;
            if (original20[offset]!=original40[offset])
                return false;
        }
    }
    return true;
}

// Both operands must be genuine ORIGINAL controller SQ values.
// 0x57BA98 variant zero does not initialize controller+0x20.
// +0x40 must be published by the native original initializer
// or by the real game controller owner, never synthesized as zero.
inline bool originalManagerPackedPceqwEqual_57c1e0(
    const OriginalManagerChild70& controller) {
    if (!controller.controller20 || !controller.controller40)
        throw std::invalid_argument(
            "original controller +0x20/+0x40 packed vectors unavailable");
    return originalManagerPackedPceqwEqual_57c1e0(
        *controller.controller20,*controller.controller40);
}
} // namespace ae3
