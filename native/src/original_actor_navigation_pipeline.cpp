#include "ae3/original_actor_navigation_pipeline.hpp"
#include <cstring>
#include <stdexcept>

namespace ae3 {
OriginalActorNavigationPipeline_57c7cc::OriginalActorNavigationPipeline_57c7cc(
    std::span<const OriginalSpatialRecord70> records,
    std::span<const OriginalSpatialSourcePolygon> polygons,
    std::array<std::byte,16> controllerVector20,bool variant30,
    OriginalActorUntranslatedMath_57c7cc& math,
    OriginalGameRandom_36f3c0* originalSharedRandom)
    :records_(records),polygons_(polygons),controllerVector20_(controllerVector20),
     variant30_(variant30),math_(math),originalSharedRandom_(originalSharedRandom) {
    if(records_.size()!=polygons_.size())
        throw std::invalid_argument("Original stage record/header polygon count mismatch");
    static_assert(sizeof(point_)==16);
    std::memcpy(&point_,controllerVector20_.data(),16);
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
    return math_.call_original_4c1b3c_projection(*actorPolygon,point_);
}
void OriginalActorNavigationPipeline_57c7cc::call_original_57c8ec_update_target(
    const OriginalSpatialRecord70* target) {
    if(!target)throw std::invalid_argument("Original target polygon is null");
    sourceIndex(*target);
    targetResults_.push_back(originalActorTargetVector_57c8ec(
        *target,variant30_,controllerVector20_,*this));
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
    return originalActorAdvanceRoute_57c7cc(route,*this);
}
} // namespace ae3
