#pragma once
#include "ae3/original_actor_ray_geometry.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace ae3 {
// Original actor controller 0x57BCDC..0x57BD34: optional full 4-lane
// VSUB, 0x4748AC XYZ dot, SQRT.S, strict C.OLT.S against original 2.0f.
// The owner vector is the actual owner(+0x04)->(+0x14)->(+0x40) input.
struct OriginalActorDistanceGate_57bcdc {
    std::optional<float> originalComputedLengthF0;
    bool originalClearsControllerRoute08And0c=false;
};
OriginalActorDistanceGate_57bcdc originalActorDistanceGate_57bcdc(
    bool originalControllerVariant30,
    const std::array<std::byte,16>& originalControllerVector20,
    const std::array<std::byte,16>& originalOwner14Vector40) noexcept;

// Original 0x57BD3C..0x57BD54, executed ONLY when the strict gate passes.
// These are the two original 32-bit fields, not arbitrary physics state.
void originalActorApplyDistanceGate_57bd3c(
    const OriginalActorDistanceGate_57bcdc& originalResult,
    std::uint32_t& originalControllerField08,
    std::uint32_t& originalControllerField0c) noexcept;
} // namespace ae3
