#pragma once
#include "ae3/original_vu0_geometry.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace ae3 {
// Original 0x57C080..0x57C0E8 native movement-distance predicate.
// Inputs are the original EE s3/s4 128-bit values supplied by the
// caller's 0x21B854/0x57C274 paths. Neither is reconstructed here.
struct OriginalActorXZProximity_57c080 {
    OriginalVU0Vec4 originalDifferenceVF1{};
    OriginalVU0Vec4 originalPackedVF3{};
    float originalSquaredDistanceF0=0.0f;
    float originalDistanceF0=0.0f;
    bool originalWithin2Units=false;
};
OriginalActorXZProximity_57c080 originalActorXZProximity_57c080(
    const std::array<std::byte,16>& originalSavedS3,
    const std::array<std::byte,16>& originalSavedS4) noexcept;

// Only the original route-present case, 0x57C07C PCPYUD s3,zero,v0.
// The result of 0x21B854 is required; it is not synthesized here.
std::array<std::byte,16> originalActorPack21B854Return_57c07c(
    const std::array<std::byte,16>& originalResultV0) noexcept;
}
