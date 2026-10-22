#include "ae3/original_nav_actor_binding.hpp"
#include "ae3/original_stage_actor_navigation.hpp"
#include <stdexcept>

namespace ae3 {
OriginalActorRouteFollowResult_57c7cc
OriginalNavActorRouteBinding::originalAdvanceNativeStage_57c7cc(
    OriginalStageActorNavigation_57c7cc& pipeline,
    const OriginalStageSpatialLoaded& sourceGraphUsedByPipeline,
    std::uint32_t actorStagePolygon) {
    // currentRouteOrThrow rejects an invalidated original 0x2207D0
    // source owner or an outdated original navrange group membership.
    const auto& route=currentRouteOrThrow();
    const auto& slot=owners_.originalAt(route.sourceOwnerSlot);
    if(!slot.decodedGraph || &*slot.decodedGraph!=&sourceGraphUsedByPipeline)
        throw std::invalid_argument(
            "Actor pipeline source graph must match active navigation route owner");
    if(actorStagePolygon>=sourceGraphUsedByPipeline.records.size())
        throw std::out_of_range("Original actor polygon outside stage source");
    // The stage pipeline has its own controller+0x50 semantic target;
    // preserve it instead of overwriting that original state with a
    // host route-binding pointer, which may not yet be initialized.
    const auto* existing=pipeline.currentOriginalTargetAt50();
    if(existing) {
        bool belongs=false;
        for(const auto& record:sourceGraphUsedByPipeline.records)
            if(&record==existing){belongs=true;break;}
        if(!belongs)
            throw std::invalid_argument("Original actor target belongs to different graph");
    }
    const auto& path=route.originalForwardSourcePolygons;
    if(path.empty())
        throw std::logic_error("Original route -1 has no actor path to advance");
    for(const auto index:path)
        if(index>=sourceGraphUsedByPipeline.records.size())
            throw std::out_of_range("Original route polygon outside stage graph");
    // This calls the already recovered original 0x57C7CC follower,
    // original VU0 0x4C1B3C stage ray, original 0x57C8EC target
    // dispatch and original game random handling owned by VU0 team.
    const auto result=pipeline.advance(path,actorStagePolygon,path.size()>=2);
    currentTargetAt50_=pipeline.currentOriginalTargetAt50();
    return result;
}
} // namespace ae3
