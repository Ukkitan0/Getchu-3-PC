#include "ae3/original_nav_stage_route.hpp"
#include "ae3/original_nav_type1_route.hpp"
#include <limits>
#include <stdexcept>

namespace ae3 {
namespace {
std::uint32_t sourceIndex(
    std::span<const OriginalSpatialRecord70> records,
    const OriginalSpatialRecord70* source) {
    for(std::size_t i=0;i<records.size();++i)
        if(&records[i]==source) {
            if(i>std::numeric_limits<std::uint32_t>::max())
                throw std::out_of_range("Original navigation source index overflow");
            return static_cast<std::uint32_t>(i);
        }
    throw std::invalid_argument("Original navigation polygon belongs to another stage owner");
}
const OriginalNavStageOwners_21ed30::Slot& currentSlot(
    const OriginalNavStageOwners_21ed30& owners,
    const OriginalNavStageRouteSnapshot& route) {
    if(!originalNavStageRouteCurrent(owners,route))
        throw std::logic_error("Original navigation route invalidated by stage owner transition");
    return owners.originalAt(route.sourceOwnerSlot);
}
std::span<const std::byte> sourceBytes(
    const OriginalNavStageOwners_21ed30::Slot& slot) {
    return std::as_bytes(std::span<const std::uint8_t>(slot.originalSourceAllocation));
}
} // namespace

float OriginalNavStageType1Costs_220f18::call_original_21b4e0_21b194(
    const OriginalSpatialRecord70& polygon,std::uint32_t slot) {
    if(slot>=3)throw std::out_of_range("Original navigation source edge slot out of range");
    const auto index=sourceIndex(graph_.records,&polygon);
    return originalNavType1ActionDistance_220f28(
        graph_,index,static_cast<std::uint16_t>(slot));
}

OriginalNavStageRouteSnapshot originalNavStageDispatch_2207d0(
    const OriginalNavStageOwners_21ed30& owners,
    std::size_t ownerSlot,OriginalNavDispatchRequest request,
    const OriginalNavListOwner* member) {
    const auto& slot=owners.originalAt(ownerSlot);
    if(!slot.decodedGraph)
        throw std::logic_error("Original navigation stage slot has no active graph");
    const auto& graph=*slot.decodedGraph;
    const auto bytes=sourceBytes(slot);
    // Do not pass a stale/foreign group list into the source pointer
    // membership predicate 0x4E3DAC or original nearest 0x4E3DEC.
    if(member) {
        if(!member->originalList())
            throw std::logic_error("Original navigation membership owner is not created");
        for(const auto* p:member->originalList()->originalOrderedValues())
            (void)sourceIndex(graph.records,p);
    }
    OriginalNavStageType1Costs_220f18 type1(graph);
    const auto result=member
        ? originalNavRouteDispatchOwned_2207d0(graph.records,bytes,request,*member,&type1)
        : originalNavRouteDispatch_2207d0(graph.records,bytes,request,&type1);
    OriginalNavStageRouteSnapshot snapshot{};
    snapshot.sourceOwnerSlot=ownerSlot;
    snapshot.hostStageOwnerToken=owners.originalHostOwnerToken();
    snapshot.hostOwnerRevision=slot.hostRevision;
    snapshot.originalFallbackUsed=result.fallbackUsed;
    snapshot.originalSelectedSearchIndex=result.selectedEndpoint;
    snapshot.originalAdmittedSourcePolygons=result.originalAdmittedRecordIndices;
    if(result.fallbackPolygon)
        snapshot.originalFallbackSourcePolygon=sourceIndex(graph.records,result.fallbackPolygon);
    snapshot.originalForwardSourcePolygons.reserve(result.forwardPath.size());
    for(const auto* p:result.forwardPath)
        snapshot.originalForwardSourcePolygons.push_back(sourceIndex(graph.records,p));
    return snapshot;
}
bool originalNavStageRouteCurrent(
    const OriginalNavStageOwners_21ed30& owners,
    const OriginalNavStageRouteSnapshot& route) noexcept {
    return route.hostOwnerRevision!=0 && route.hostStageOwnerToken!=0 &&
           owners.originalHostOwnerToken()==route.hostStageOwnerToken &&
           owners.originalSlotRevision(route.sourceOwnerSlot)==route.hostOwnerRevision;
}
const OriginalSpatialRecord70* originalNavStageRoutePolygon(
    const OriginalNavStageOwners_21ed30& owners,
    const OriginalNavStageRouteSnapshot& route,std::size_t index) {
    const auto& slot=currentSlot(owners,route);
    if(index>=route.originalForwardSourcePolygons.size())
        throw std::out_of_range("Original navigation route index outside path");
    const auto source=route.originalForwardSourcePolygons[index];
    if(source>=slot.decodedGraph->records.size())
        throw std::out_of_range("Original navigation path polygon outside source graph");
    return &slot.decodedGraph->records[source];
}
OriginalActorRouteEdgeDecision_57bd88 originalNavStageRouteEdge_57bd88(
    const OriginalNavStageOwners_21ed30& owners,
    const OriginalNavStageRouteSnapshot& route,std::size_t segment) {
    const auto& slot=currentSlot(owners,route);
    return originalActorRouteEdgeDecision_57bd88(
        route.originalForwardSourcePolygons,segment,sourceBytes(slot));
}
OriginalActorRouteFollowerState_57c7cc originalNavStageFollowerState_57c7cc(
    const OriginalNavStageOwners_21ed30& owners,
    const OriginalNavStageRouteSnapshot& route,
    std::uint32_t actorSourcePolygon,const OriginalSpatialRecord70* activeTarget) {
    const auto& slot=currentSlot(owners,route);
    const auto& records=slot.decodedGraph->records;
    if(actorSourcePolygon>=records.size())
        throw std::out_of_range("Original actor current polygon outside stage source");
    if(activeTarget)(void)sourceIndex(records,activeTarget);
    OriginalActorRouteFollowerState_57c7cc state{};
    state.sourceRecords=records;
    state.originalRoutePolygonIndices=route.originalForwardSourcePolygons;
    state.originalStagePolygonAtActor30=&records[actorSourcePolygon];
    state.activePolygonAtController50=activeTarget;
    // 0x57BA98/0x57BAF8 signed route count >= 2 sets +0x1C.
    state.sourceHasMultiplePolygonsAtController1c=
        route.originalForwardSourcePolygons.size()>=2;
    return state;
}
} // namespace ae3
