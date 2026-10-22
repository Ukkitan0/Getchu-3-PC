#pragma once
#include "ae3/original_actor_fallback_sampler.hpp"
namespace ae3 {
// Native translation of the original 0x57CA68 fixed VU0 candidate blocks.
// No EE interpreter, synthetic steering or external game state substitution.
class OriginalActorFallbackNativeVU0_57ca68 final
    : public OriginalActorFallbackVU0Transform_57ca68 {
public:
    OriginalActorFallbackCandidate_57ca68 call_original_57ca68_candidate_VU0(
        const std::array<std::array<std::byte,16>,3>& originalSourcePolygonVertices,
        const std::array<std::byte,16>& originalInputA1OwnerVector,
        const std::array<std::byte,16>& originalInputA2Controller20,
        std::uint32_t originalVU0QBits,
        const OriginalActorTriangleWeights_57c98c& sourceRandomWeights,
        std::size_t sourceCandidateNumber) override;
};
} // namespace ae3
