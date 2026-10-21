#include "ae3/original_actor_route_follower.hpp"
#include <stdexcept>

namespace ae3 {
OriginalActorRouteFollowResult_57c7cc originalActorAdvanceRoute_57c7cc(
    OriginalActorRouteFollowerState_57c7cc& state,
    OriginalActorRouteFollowerCalls_57c7cc& calls) {
    OriginalActorRouteFollowResult_57c7cc result{};
    if(!state.originalStagePolygonAtActor30)
        throw std::invalid_argument("Original actor stage polygon+0x30 unavailable");
    // 0x57C7FC conditional source LBU controller+0x1C.
    if(!state.sourceHasMultiplePolygonsAtController1c) {
        // 0x57C8D8..0x57C8E0: owner+0x30 directly becomes target+0x50.
        state.activePolygonAtController50=state.originalStagePolygonAtActor30;
        calls.call_original_57c8ec_update_target(state.activePolygonAtController50);
        ++result.sourceTargetsApplied;
        return result;
    }
    if(state.originalRoutePolygonIndices.size()<2)
        throw std::out_of_range("Original actor controller+0x1C requires route[1]");
    const auto at=[&](std::size_t index)->const OriginalSpatialRecord70*{
        const auto sourceIndex=state.originalRoutePolygonIndices[index];
        if(sourceIndex>=state.sourceRecords.size())
            throw std::out_of_range("Original actor route entry outside source polygon registry");
        return &state.sourceRecords[sourceIndex];
    };
    // 0x57C804..0x57C828: only project current actor position when the
    // controller already has a target. A matching pointer ends the function.
    if(state.activePolygonAtController50) {
        const auto* projected=calls.call_original_4c1b3c_project(state.originalStagePolygonAtActor30);
        ++result.sourceProjectionsPerformed;
        if(projected==state.activePolygonAtController50) {
            result.returnedEarlyBecauseCurrentTargetMatched=true;
            return result;
        }
    }
    // 0x57C82C..0x57C844: route[1] is unconditionally selected and
    // 0x57C8EC called. The source does not project it first.
    state.activePolygonAtController50=at(1);
    calls.call_original_57c8ec_update_target(state.activePolygonAtController50);
    ++result.sourceTargetsApplied;
    // 0x57C848..0x57C8B4: source signed index starts at 2. Every entry
    // is applied BEFORE the original 0x4C1B3C geometry projection.
    // A projection mismatch returns immediately without selecting entry.
    for(std::size_t index=2;index<state.originalRoutePolygonIndices.size();++index) {
        const auto* candidate=at(index);
        calls.call_original_57c8ec_update_target(candidate);
        ++result.sourceTargetsApplied;
        ++result.followingRouteIndicesTested;
        const auto* projected=calls.call_original_4c1b3c_project(state.originalStagePolygonAtActor30);
        ++result.sourceProjectionsPerformed;
        if(projected!=candidate)break;
        state.activePolygonAtController50=candidate;
    }
    return result;
}
} // namespace ae3
