#include "ae3/original_nav_actor_binding.hpp"
#include <stdexcept>
#include <utility>

namespace ae3 {
void OriginalNavActorRouteBinding::originalUseStageRoute_2207d0(
    OriginalNavStageRouteSnapshot route) {
    if(!originalNavStageRouteCurrent(owners_,route))
        throw std::invalid_argument("Original actor stage route source owner is stale");
    // An explicit original 0x2207D0 destination of -1 gives an
    // empty path; no source route exists for 0x57C7CC to follow.
    // Do not synthesize a one-polygon route from actor+0x30.
    if(route.originalForwardSourcePolygons.empty()) {
        originalClearForStageTransition();
        return;
    }
    // Commit only after validating incoming source identity.
    plainRoute_=std::move(route);
    groupRoute_.reset();
    groupOwner_=nullptr;
    groupLifetime_.reset();
    currentTargetAt50_=nullptr;
}
void OriginalNavActorRouteBinding::originalUseGroupRoute_2207d0(
    const OriginalNavStageGroupSession& group,
    OriginalNavStageGroupedRoute route) {
    if(!group.originalGroupedRouteCurrent(route) ||
       !originalNavStageRouteCurrent(owners_,route.sourceRoute))
        throw std::invalid_argument("Original actor navrange route identity is stale");
    if(route.sourceRoute.originalForwardSourcePolygons.empty()) {
        originalClearForStageTransition();
        return;
    }
    groupRoute_=std::move(route);
    plainRoute_.reset();
    groupOwner_=&group;
    groupLifetime_=group.originalLifetimeToken();
    currentTargetAt50_=nullptr;
}
bool OriginalNavActorRouteBinding::originalRetargetStage_2207d0(
    std::size_t slot,OriginalNavDispatchRequest request,
    const OriginalNavListOwner* membership) {
    // Generate into temporary first. This preserves the old binding
    // when original admission, type-1 cost or fallback resolution fails.
    auto next=originalNavStageDispatch_2207d0(owners_,slot,request,membership);
    originalUseStageRoute_2207d0(std::move(next));
    return originalRouteCurrent();
}
bool OriginalNavActorRouteBinding::originalRetargetGroup_2207d0(
    const OriginalNavStageGroupSession& group,
    const OriginalNavGroupKey20& key,OriginalNavDispatchRequest request) {
    auto next=group.originalBoundRouteByKey_2207d0(key,request);
    originalUseGroupRoute_2207d0(group,std::move(next));
    return originalRouteCurrent();
}
bool OriginalNavActorRouteBinding::originalRouteCurrent() const noexcept {
    if(groupRoute_)
        return groupOwner_ && !groupLifetime_.expired() &&
            groupOwner_->originalGroupedRouteCurrent(*groupRoute_) &&
            originalNavStageRouteCurrent(owners_,groupRoute_->sourceRoute);
    return plainRoute_ && originalNavStageRouteCurrent(owners_,*plainRoute_);
}
bool OriginalNavActorRouteBinding::originalDropInvalidRoute() noexcept {
    if((plainRoute_||groupRoute_) && !originalRouteCurrent()) {
        originalClearForStageTransition();
        return true;
    }
    return false;
}
void OriginalNavActorRouteBinding::originalClearForStageTransition() noexcept {
    plainRoute_.reset();
    groupRoute_.reset();
    groupOwner_=nullptr;
    currentTargetAt50_=nullptr;
}
const OriginalSpatialRecord70*
OriginalNavActorRouteBinding::originalCurrentTargetAt50() const noexcept {
    // Never hand out a dangling source polygon when its graph or
    // original navrange membership has been torn down/replaced.
    return originalRouteCurrent() ? currentTargetAt50_ : nullptr;
}
std::size_t OriginalNavActorRouteBinding::originalPathLength() const noexcept {
    if(!originalRouteCurrent())return 0;
    return groupRoute_ ? groupRoute_->sourceRoute.originalForwardSourcePolygons.size()
                       : plainRoute_->originalForwardSourcePolygons.size();
}
bool OriginalNavActorRouteBinding::originalHasMultiplePolygonsAtController1c() const noexcept {
    // Original constructors 0x57BA98/0x57BAF8 use signed count>=2.
    return originalPathLength()>=2;
}
const OriginalNavStageRouteSnapshot&
OriginalNavActorRouteBinding::currentRouteOrThrow() const {
    if(!originalRouteCurrent())
        throw std::logic_error("Original actor navigation route is unavailable or invalidated");
    return groupRoute_ ? groupRoute_->sourceRoute : *plainRoute_;
}
OriginalActorRouteEdgeDecision_57bd88
OriginalNavActorRouteBinding::originalCurrentEdge_57bd88(std::size_t segment) const {
    return originalNavStageRouteEdge_57bd88(owners_,currentRouteOrThrow(),segment);
}
std::optional<OriginalActorType2Target_57bdb8>
OriginalNavActorRouteBinding::originalInitialType2Target_57bdb8() const {
    return originalNavStageType2Target_57bdb8(owners_,currentRouteOrThrow());
}
OriginalActorRouteFollowResult_57c7cc OriginalNavActorRouteBinding::originalAdvance_57c7cc(
    std::uint32_t polygon,OriginalActorRouteFollowerCalls_57c7cc& calls) {
    const auto& route=currentRouteOrThrow();
    return originalNavStageAdvanceRoute_57c7cc(
        owners_,route,polygon,currentTargetAt50_,calls);
}
} // namespace ae3
