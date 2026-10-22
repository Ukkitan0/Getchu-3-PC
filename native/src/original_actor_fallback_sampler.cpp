#include "ae3/original_actor_fallback_sampler.hpp"
#include <limits>
namespace ae3 {
OriginalActorFallbackSelection_57ca68 originalActorFallbackSelect_57ca68(
    const OriginalSpatialRecord70& originalPolygon,
    const std::array<std::byte,16>& originalInputA1,
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
        // A callback here is the actual *unrecovered* 0x57CA68 VU0
        // candidate transform. The algorithm cannot claim native
        // completion until that source routine is ported.
        const auto candidate=originalVU0.call_original_57ca68_candidate_VU0(
            originalVertices,originalInputA1,originalWeights,sample);
        // Original source emits two VU0 VRINIT(VF0.w) / VRXOR(VF0.x)
        // pairs per candidate, distinct from the two EE game RNG draws.
        // Their final state is observable after the candidate transform;
        // the transform itself still needs a faithful native translation.
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
        }
        ++result.sourceCandidatesEvaluated;
    }
    return result;
}
} // namespace ae3
