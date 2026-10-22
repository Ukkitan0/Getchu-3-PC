#pragma once
#include "ae3/original_game_random.hpp"
#include "ae3/original_vu0_geometry.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
namespace ae3 {
// Original USA 0x57C98C..0x57CA64. Two sequential draws from the actual
// game RNG state. No replacement RNG, geometry, or trial-and-error walk.
struct OriginalActorTriangleWeights_57c98c {
    float firstDraw=0.0f, secondDraw=0.0f, remainder=0.0f;
    std::array<float,3> originalWeightedCoefficients{};
};
OriginalActorTriangleWeights_57c98c originalActorTriangleWeights_57c98c(
    OriginalGameRandom_36f3c0& originalRandom) noexcept;

// Implements original VMULx/VADD VU0 sequence using original vertex order
// and the authentic 0x708A68 (0.8) / 0x708A6C (~1/15) constants.
OriginalVU0Vec4 originalActorRandomTriangle_57c98c(
    const std::array<OriginalVU0Vec4,3>& sourceVertices,
    OriginalGameRandom_36f3c0& originalRandom) noexcept;
std::uint64_t originalActorRandomTriangleLow64_57c98c(
    const std::array<std::array<std::byte,16>,3>& sourceVertexBytes,
    OriginalGameRandom_36f3c0& originalRandom) noexcept;
} // namespace ae3
