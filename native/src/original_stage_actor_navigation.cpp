#include "ae3/original_stage_actor_navigation.hpp"
#include <stdexcept>

namespace ae3 {
OriginalStageActorNavigation_57c7cc::OriginalStageActorNavigation_57c7cc(
    const OriginalStageSpatialSource& source,
    const OriginalStageSpatialLoaded& stage,
    std::array<std::byte,16> controller20,
    bool variant30,
    OriginalActorUntranslatedMath_57c7cc& pending,
    OriginalGameRandom_36f3c0* sharedRandom)
    :stage_(stage),
     sourceGeometry_(stage.records,stage.polygonHeaders),
     adjacency_(originalStagePolygonLinks_21bd3c(source,stage)),
     pipeline_(stage.records,sourceGeometry_.polygons(),controller20,
               variant30,pending,sharedRandom,adjacency_) {
    // Source geometry/polygon spans are owned by this instance, and
    // original stage record identity remains stable while it lives.
}
void OriginalStageActorNavigation_57c7cc::setOriginalMovementVectors_57c7cc(
    const OriginalVU0Vec4& a1,const OriginalVU0Vec4& originalController40) {
    pipeline_.setOriginalProjectionOperands_57c7cc(a1,originalController40);
}
void OriginalStageActorNavigation_57c7cc::setOriginalTargetAt50(
    const OriginalSpatialRecord70* target) {
    if(target){
        bool belongs=false;
        for(const auto& source:stage_.records)if(&source==target){belongs=true;break;}
        if(!belongs)throw std::invalid_argument("Original actor current target belongs to different stage");
    }
    currentTargetAt50_=target;
}
OriginalActorRouteFollowResult_57c7cc OriginalStageActorNavigation_57c7cc::advance(
    std::span<const std::uint32_t> sourceRoute,
    std::uint32_t sourceActorPolygon,
    bool originalHasMultiple) {
    if(sourceActorPolygon>=stage_.records.size())
        throw std::out_of_range("Original actor current stage polygon ID invalid");
    OriginalActorRouteFollowerState_57c7cc state{
        stage_.records,sourceRoute,&stage_.records[sourceActorPolygon],
        currentTargetAt50_,originalHasMultiple
    };
    const auto result=pipeline_.advance(state);
    currentTargetAt50_=state.activePolygonAtController50;
    return result;
}
} // namespace ae3
