#include "ae3/original_actor_navigation_pipeline.hpp"
#include <bit>
#include <cstring>
#include <stdexcept>

namespace ae3 {
OriginalActorNavigationPipeline_57c7cc::OriginalActorNavigationPipeline_57c7cc(
    std::span<const OriginalSpatialRecord70> records,
    std::span<const OriginalSpatialSourcePolygon> polygons,
    std::array<std::byte,16> controllerVector20,bool variant30,
    OriginalActorUntranslatedMath_57c7cc& math,
    OriginalGameRandom_36f3c0* originalSharedRandom,
    std::span<const OriginalPolygonLinks_21bd3c> originalSourceAdjacency,
    OriginalActorFallbackVU0Transform_57ca68* originalFallbackTransform)
    :records_(records),polygons_(polygons),controllerVector20_(controllerVector20),
     variant30_(variant30),math_(math),originalSharedRandom_(originalSharedRandom),
     originalFallbackTransform_(originalFallbackTransform),
     originalSourceAdjacency_(originalSourceAdjacency) {
    if(!originalSourceAdjacency_.empty()&&originalSourceAdjacency_.size()!=records_.size())
        throw std::invalid_argument("Original source actor adjacency/registry count mismatch");
    if(records_.size()!=polygons_.size())
        throw std::invalid_argument("Original stage record/header polygon count mismatch");
    static_assert(sizeof(point_)==16);
    std::memcpy(&point_,controllerVector20_.data(),16);
}
void OriginalActorNavigationPipeline_57c7cc::setOriginalProjectionOperands_57c7cc(
    const OriginalVU0Vec4& incomingA1,const OriginalVU0Vec4& sourceController40) {
    originalIncomingRayA1_=incomingA1;
    originalController40_=sourceController40;
}
void OriginalActorNavigationPipeline_57c7cc::setOriginalFallbackOwnerVector_57ca68(
    const std::array<std::byte,16>& sourceOwnerVector) {
    originalOwner14Vector40_=sourceOwnerVector;
}
void OriginalActorNavigationPipeline_57c7cc::setOriginalFallbackQBits_57ca68(
    std::uint32_t originalQ) {
    originalFallbackQBits_=originalQ;
}
std::size_t OriginalActorNavigationPipeline_57c7cc::sourceIndex(
    const OriginalSpatialRecord70& polygon) const {
    // Use equality instead of undefined host pointer-range comparisons.
    for(std::size_t i=0;i<records_.size();++i)
        if(&records_[i]==&polygon)return i;
    throw std::invalid_argument("Actor polygon does not belong to this original stage");
}
bool OriginalActorNavigationPipeline_57c7cc::call_original_21b788_eligibility(
    const OriginalSpatialRecord70& target,float originalF12) {
    // 0x57C928 loads original $a1 from controller+0x20, and
    // 0x57C930 calls the already recovered native 0x21B788.
    const auto index=sourceIndex(target);
    return originalVU0PolygonEligible_21b788(
        polygons_[index].sourcePoints,point_,originalF12);
}
std::uint64_t OriginalActorNavigationPipeline_57c7cc::call_original_57ca68_geometry(
    const OriginalSpatialRecord70& target) {
    sourceIndex(target);
    // Native fixed-instruction 0x57CB4C/0x57CC48 transform by default.
    // The original EE RNG and caller-owned owner/Q are mandatory.
    latestFallbackHigh64_=0;
    if(originalSharedRandom_) {
        if(!originalOwner14Vector40_)
            throw std::invalid_argument(
                "Original 0x57CA68 owner+0x14->+0x40 vector missing; cannot reuse controller+0x20");
        if(!originalFallbackQBits_)
            throw std::invalid_argument("Original VU0 Q input not supplied for VMULq fallback transform");
        auto& sourceTransform=originalFallbackTransform_?
            *originalFallbackTransform_:nativeFallbackTransform_;
        const auto selected=originalActorFallbackSelect_57ca68(
            target,*originalOwner14Vector40_,controllerVector20_,
            *originalFallbackQBits_,*originalSharedRandom_,sourceTransform,
            originalFallbackR_);
        latestFallbackHigh64_=selected.originalWinningPackedTargetHigh64;
        return selected.originalWinningPackedTargetLow64;
    }
    return math_.call_original_57ca68_fallback(target);
}
std::uint64_t OriginalActorNavigationPipeline_57c7cc::call_original_57c98c_three_vertex_transform(
    const OriginalSpatialRecord70& target,
    const std::array<std::array<std::byte,16>,3>& vertices) {
    sourceIndex(target);
    // Original source 0x57C98C is now fully recovered when the
    // actor owns its original game-global RNG state.
    if(originalSharedRandom_)
        return originalActorRandomTriangleLow64_57c98c(vertices,*originalSharedRandom_);
    return math_.call_original_57c98c_random_vertex_transform(target,vertices);
}
const OriginalSpatialRecord70* OriginalActorNavigationPipeline_57c7cc::call_original_4c1b3c_project(
    const OriginalSpatialRecord70* actorPolygon) {
    if(!actorPolygon)throw std::invalid_argument("Original actor stage polygon is null");
    sourceIndex(*actorPolygon);
    if(!originalSourceAdjacency_.empty()){
        if(!originalIncomingRayA1_||!originalController40_)
            throw std::invalid_argument("Original 0x57C7CC ray operands unavailable");
        // 0x57C814 uses controller+0x40 for the first projection.
        // 0x57C890 instead uses packed result from 0x57C8EC
        // after selecting the next route target.
        const auto& sourceEnd=latestTargetRayA2_?*latestTargetRayA2_:*originalController40_;
        const auto ray=originalPolygonRaycast_4c1b3c(
            records_,originalSourceAdjacency_,
            static_cast<std::uint32_t>(sourceIndex(*actorPolygon)),
            *originalIncomingRayA1_,sourceEnd);
        return ray.originalTraversal.originalLastAcceptedPolygon;
    }
    return math_.call_original_4c1b3c_projection(*actorPolygon,point_);
}
void OriginalActorNavigationPipeline_57c7cc::call_original_57c8ec_update_target(
    const OriginalSpatialRecord70* target) {
    if(!target)throw std::invalid_argument("Original target polygon is null");
    sourceIndex(*target);
    const auto output=originalActorTargetVector_57c8ec(
        *target,variant30_,controllerVector20_,*this);
    targetResults_.push_back(output);
    // Source 0x57C894 builds $a2 from original 64-bit $v0,
    // zeroing the high EE GPR half. Do not treat +0x20 as this
    // generated target; source 0x57C8EC owns its packed result.
    OriginalVU0Vec4 packed{};
    const auto bits=output.originalVectorLow64;
    packed.x=std::bit_cast<float>(std::uint32_t(bits));
    packed.y=std::bit_cast<float>(std::uint32_t(bits>>32));
    // 0x57C988 DMTC2 consumes only the low 64 bits of the original GPR.
    // Do not reinterpret the returned PCPYUD upper half as target z/w.
    // 0x57C894 also zero-extends the original $v0 low doubleword to $a2.
    latestTargetRayA2_=packed;
}
OriginalActorRouteFollowResult_57c7cc OriginalActorNavigationPipeline_57c7cc::advance(
    OriginalActorRouteFollowerState_57c7cc& route) {
    if(route.sourceRecords.data()!=records_.data()||
       route.sourceRecords.size()!=records_.size())
        throw std::invalid_argument("Actor route stage registry mismatch");
    if(!route.originalStagePolygonAtActor30)
        throw std::invalid_argument("Original actor stage polygon+0x30 is null");
    sourceIndex(*route.originalStagePolygonAtActor30);
    targetResults_.clear();
    latestTargetRayA2_.reset();
    return originalActorAdvanceRoute_57c7cc(route,*this);
}
} // namespace ae3
