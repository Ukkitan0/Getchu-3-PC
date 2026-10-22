#include "ae3/original_actor_fallback_sampler.hpp"
#include <limits>
namespace ae3 {
OriginalActorFallbackSelection_57ca68 originalActorFallbackSelect_57ca68(
    const OriginalSpatialRecord70& originalPolygon,
    const std::array<std::byte,16>& originalInputA1OwnerVector,
    const std::array<std::byte,16>& originalInputA2Controller20,
    std::uint32_t originalVU0QBits,
    OriginalGameRandom_36f3c0& sourceRNG,
    OriginalActorFallbackVU0Transform_57ca68& originalVU0,
    OriginalActorFallbackVu0R_57ca68* originalVu0R) {
    std::array<std::array<std::byte,16>,3> originalVertices{};
    for(std::size_t i=0;i<3;++i)
        originalVertices[i]=originalSpatialVector_21b458(originalPolygon,i);
    OriginalActorFallbackSelection_57ca68 result{};
    for(std::size_t sample=0;sample<kOriginalFallbackCandidates_57ca68;++sample){
        // Source first candidate 0x57CADC/0x57CB00, then five more
        // repetitions from original 0x57CBD8/0x57CBFC.
        const auto originalWeights=originalActorTriangleWeights_57c98c(sourceRNG);
        // Virtual source interface dispatches the production native
        // 0x57CB4C/0x57CC48 implementation (or focused test verifier).
        const auto candidate=originalVU0.call_original_57ca68_candidate_VU0(
            originalVertices,originalInputA1OwnerVector,
            originalInputA2Controller20,originalVU0QBits,
            originalWeights,sample);
        // Original source emits two VU0 VRINIT(VF0.w) / VRXOR(VF0.x)
        // pairs per candidate, distinct from the two EE game RNG draws.
        // Their final state is observable through caller-bound VU0 R state.
        if(originalVu0R)
            originalFallbackVu0RandomCandidate_57ca68(*originalVu0R);
        // Source 0x57CBC4/0x57CCBC: actual 0x4748AC result.
        const auto sourceScore=originalVU0DotXYZ_4748ac(
            candidate.originalScoreVectorA,candidate.originalScoreVectorB);
        // At 0x57CCC4 source C.OLT.S compares previous best
        // with new score: ties and NaN do NOT replace the best.
        if(sample==0 || result.originalBestXYZDot<sourceScore){
            result.originalBestXYZDot=sourceScore;
            result.sourceWinningCandidate=sample;
            result.originalWinningPackedTargetLow64=candidate.originalPackedTargetLow64;
            result.originalWinningPackedTargetHigh64=candidate.originalPackedTargetHigh64;
        }
        ++result.sourceCandidatesEvaluated;
    }
    return result;
}
} // namespace ae3
