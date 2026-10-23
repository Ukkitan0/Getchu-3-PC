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

// Implements source PCPYUD of each vertex's HIGH 64 bits followed by
// 0x57CA28/40/54 VMULx and 0x57CA5C/64 VADD in source order.
// VF.x/y are zero; VF.z/w hold the weighted original z/w coordinates.
OriginalVU0Vec4 originalActorRandomTriangle_57c98c(
    const std::array<OriginalVU0Vec4,3>& sourceVertices,
    OriginalGameRandom_36f3c0& originalRandom) noexcept;
// 0x57C948 QMFC2 transfers all 128 bits. These two GPR halves
// are the *full* source actor target returned to 0x57C894.
struct OriginalActorRandomTrianglePacked_57c98c {
    std::uint64_t originalLow64=0;
    std::uint64_t originalHigh64=0;
};
OriginalActorRandomTrianglePacked_57c98c originalActorRandomTrianglePacked_57c98c(
    const std::array<std::array<std::byte,16>,3>& originalSourceVertexBytes,
    OriginalGameRandom_36f3c0& originalSharedRandom) noexcept;
// Compatibility function: source low 64 bits are zero after PCPYUD.
std::uint64_t originalActorRandomTriangleLow64_57c98c(
    const std::array<std::array<std::byte,16>,3>& sourceVertexBytes,
    OriginalGameRandom_36f3c0& originalRandom) noexcept;
} // namespace ae3
