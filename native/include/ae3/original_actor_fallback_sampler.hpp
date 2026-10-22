#pragma once
#include "ae3/original_actor_random_triangle.hpp"
#include "ae3/original_actor_fallback_vu0_random.hpp"
#include "ae3/original_actor_ray_geometry.hpp"
#include "ae3/original_spatial_index.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
namespace ae3 {
// Original 0x57CA68: sample one candidate before 0x57CBBC,
// then repeat the trial body 5 more times (s2=4..0).
inline constexpr std::size_t kOriginalFallbackCandidates_57ca68=6;

struct OriginalActorFallbackCandidate_57ca68 {
    // The original candidate VU0 code is not yet decoded. Its
    // actual source vectors feed original 0x4748AC dot evaluation.
    OriginalVU0Vec4 originalScoreVectorA{};
    OriginalVU0Vec4 originalScoreVectorB{};
    std::uint64_t originalPackedTargetLow64=0;
    // 0x57CCE0 PCPYUD returns the chosen VF vector in EE GPR[127:64];
    // callers must not silently discard these source Z/W bits.
    std::uint64_t originalPackedTargetHigh64=0;
};
class OriginalActorFallbackVU0Transform_57ca68 {
public:
    virtual ~OriginalActorFallbackVU0Transform_57ca68()=default;
    virtual OriginalActorFallbackCandidate_57ca68 call_original_57ca68_candidate_VU0(
        const std::array<std::array<std::byte,16>,3>& originalSourcePolygonVertices,
        const std::array<std::byte,16>& originalInputA1OwnerVector,
        const std::array<std::byte,16>& originalInputA2Controller20,
        std::uint32_t originalVU0QBits,
        const OriginalActorTriangleWeights_57c98c& sourceRandomWeights,
        std::size_t sourceCandidateNumber)=0;
};
struct OriginalActorFallbackSelection_57ca68 {
    std::size_t sourceCandidatesEvaluated=0;
    std::size_t sourceWinningCandidate=0;
    float originalBestXYZDot=0;
    std::uint64_t originalWinningPackedTargetLow64=0;
    std::uint64_t originalWinningPackedTargetHigh64=0;
};
// Source 0x57CAA8..0x57CACC gets original three polygon vertices.
// Source 0x57CADC..0x57CB44 / 0x57CBD8..0x57CC40 draws two floats
// per candidate, mixing weights with original float constants.
// Source 0x57CBC4 and 0x57CCBC invoke the FULLY recovered original
// XYZ dot; original C.OLT.S replaces best only for strictly larger.
// Source VU transforms at 0x57CB4C..0x57CBB8 and 0x57CC48..0x57CCB0
// remain explicit original-specific dependencies, not guesses.
OriginalActorFallbackSelection_57ca68 originalActorFallbackSelect_57ca68(
    const OriginalSpatialRecord70& originalPolygon,
    const std::array<std::byte,16>& originalInputA1OwnerVector,
    const std::array<std::byte,16>& originalInputA2Controller20,
    std::uint32_t originalVU0QBits,
    OriginalGameRandom_36f3c0& originalSharedRNG,
    OriginalActorFallbackVU0Transform_57ca68& originalVU0,
    OriginalActorFallbackVu0R_57ca68* originalVu0R=nullptr);
} // namespace ae3
