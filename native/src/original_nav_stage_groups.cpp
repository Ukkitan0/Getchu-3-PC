#include "ae3/original_nav_stage_groups.hpp"
#include <span>
#include <stdexcept>

namespace ae3 {
namespace {
std::span<const std::byte> originalNavBytes(
    const OriginalNavStageOwners_21ed30::Slot& slot) {
    return std::as_bytes(std::span<const std::uint8_t>(slot.originalSourceAllocation));
}
} // namespace

OriginalNavStageGroupSession::OriginalNavStageGroupSession(
    const OriginalNavStageOwners_21ed30& owners,std::size_t slot)
    :owners_(owners),slot_(slot),
     ownerToken_(owners.originalHostOwnerToken()),
     ownerRevision_(owners.originalSlotRevision(slot)) {
    if(!ownerRevision_ || !owners.originalAt(slot).active())
        throw std::logic_error("Original navrange graph owner is not active");
}
bool OriginalNavStageGroupSession::originalCurrentSourceGraph() const noexcept {
    return ownerRevision_!=0 && ownerToken_==owners_.originalHostOwnerToken() &&
           ownerRevision_==owners_.originalSlotRevision(slot_);
}
void OriginalNavStageGroupSession::originalClear_4e3fec() noexcept {
    registry_.originalClear_4e3fec();
    advanceGroupRevision();
}
void OriginalNavStageGroupSession::originalRebindSourceGraph() {
    // First release pointers from the old source. Rebinding does not
    // silently restore source group registrations or stale routes.
    originalClear_4e3fec();
    const auto revision=owners_.originalSlotRevision(slot_);
    if(ownerToken_!=owners_.originalHostOwnerToken() ||
       !revision || !owners_.originalAt(slot_).active()) {
        ownerRevision_=0;
        throw std::logic_error("Original navrange cannot bind an inactive source");
    }
    ownerRevision_=revision;
}
OriginalNavGroupBuildResult OriginalNavStageGroupSession::originalRegisterRange_4e4060(
    const OriginalNavGroupSource& range) {
    if(!originalCurrentSourceGraph())
        throw std::logic_error("Stale stage graph: rebind navrange before registration");
    const auto& live=owners_.originalAt(slot_);
    if(!live.active())
        throw std::logic_error("Original navrange source graph released");
    const auto result=originalNavBuildGroupsRecovered_4e4060(
        registry_,range,live.decodedGraph->records,originalNavBytes(live));
    if(result.ownersCreated||result.sourcePolygonsInserted)
        advanceGroupRevision();
    return result;
}
OriginalNavStageRouteSnapshot OriginalNavStageGroupSession::originalRouteByKey_2207d0(
    const OriginalNavGroupKey20& key,OriginalNavDispatchRequest request) const {
    if(!originalCurrentSourceGraph())
        throw std::logic_error("Stale stage graph: original navrange route invalidated");
    const auto& live=owners_.originalAt(slot_);
    if(!live.active())
        throw std::logic_error("Original navrange source graph released");
    const auto* group=registry_.originalFind_36d6f4(key);
    if(!group)
        throw std::out_of_range("Original navrange group key not registered");
    // Original 0x2207D0 still performs admission and predecessor
    // search, including its own normal/fallback linked list selection.
    return originalNavStageDispatch_2207d0(owners_,slot_,request,group);
}
OriginalNavStageGroupedRoute OriginalNavStageGroupSession::originalBoundRouteByKey_2207d0(
    const OriginalNavGroupKey20& key,OriginalNavDispatchRequest request) const {
    OriginalNavStageGroupedRoute bound{};
    bound.sourceRoute=originalRouteByKey_2207d0(key,request);
    bound.originalGroupKey=key;
    bound.hostGroupSessionToken=groupSessionToken_;
    bound.hostGroupRevision=groupRevision_;
    return bound;
}
bool OriginalNavStageGroupSession::originalGroupedRouteCurrent(
    const OriginalNavStageGroupedRoute& route) const noexcept {
    return originalCurrentSourceGraph() &&
           route.hostGroupSessionToken==groupSessionToken_ &&
           route.hostGroupRevision==groupRevision_ &&
           route.hostGroupRevision!=0 &&
           originalNavStageRouteCurrent(owners_,route.sourceRoute);
}
} // namespace ae3
