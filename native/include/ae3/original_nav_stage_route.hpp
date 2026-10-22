#pragma once
#include "ae3/original_nav_stage_owners.hpp"
#include "ae3/original_nav_linked_owner.hpp"
#include "ae3/original_actor_nav_route.hpp"
#include "ae3/original_actor_route_follower.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace ae3 {

// Native adapter for original 0x220F18 -> 0x21B4E0 -> 0x21B194.
// A source polygon identity and edge slot select the graph-owned
// original type-1 child. No invented spline or alternative path cost.
class OriginalNavStageType1Costs_220f18 final : public OriginalNavRouteSplineCalls {
public:
    explicit OriginalNavStageType1Costs_220f18(const OriginalStageSpatialLoaded& sourceGraph)
        : graph_(sourceGraph) {}
    float call_original_21b4e0_21b194(
        const OriginalSpatialRecord70& sourcePolygon,
        std::uint32_t sourceLinkSlot) override;
private:
    const OriginalStageSpatialLoaded& graph_;
};

// Host-owned route snapshot. The source route is still created by
// 0x2207D0/0x220AA4/0x220D5C/0x220994; only the returned pointer
// identities are converted to stable polygon indices. These values are
// NOT a fabricated guest route structure or original actor state.
struct OriginalNavStageRouteSnapshot {
    std::size_t sourceOwnerSlot=0;
    std::uint64_t hostOwnerRevision=0;
    bool originalFallbackUsed=false;
    std::optional<std::uint32_t> originalFallbackSourcePolygon;
    std::int32_t originalSelectedSearchIndex=-1;
    std::vector<std::uint32_t> originalAdmittedSourcePolygons;
    std::vector<std::uint32_t> originalForwardSourcePolygons;
};

// Connect the real 0x21ED30/0x4F0A5C owner, typed 0x70 records,
// 0x21D70C source child vectors, original route dispatcher, and
// 0x57C7CC / 0x57BD88 actor route interfaces. The owner must outlive
// all calls; snapshots never retain a pointer to released record memory.
OriginalNavStageRouteSnapshot originalNavStageDispatch_2207d0(
    const OriginalNavStageOwners_21ed30& owners,
    std::size_t sourceOwnerSlot,
    OriginalNavDispatchRequest sourceRequest,
    const OriginalNavListOwner* originalMembershipOwner=nullptr);

// Native lifetime check: a released, replaced, or reinitialized stage
// owner invalidates its prior routes without invalidating other slots.
bool originalNavStageRouteCurrent(
    const OriginalNavStageOwners_21ed30& owners,
    const OriginalNavStageRouteSnapshot& route) noexcept;

// Retrieve original record pointers only while the owning slot is live.
const OriginalSpatialRecord70* originalNavStageRoutePolygon(
    const OriginalNavStageOwners_21ed30& owners,
    const OriginalNavStageRouteSnapshot& route, std::size_t routeIndex);

// Recovered 0x221104 / 0x21B444 / 0x57BD88 original action selection
// on consecutive source polygon indices (including type-2 branch).
OriginalActorRouteEdgeDecision_57bd88 originalNavStageRouteEdge_57bd88(
    const OriginalNavStageOwners_21ed30& owners,
    const OriginalNavStageRouteSnapshot& route, std::size_t segmentIndex);

// Build the exact inputs for the recovered 0x57C7CC follower.
// Returned spans borrow the route snapshot and stage; callers must not
// cache them across a stage transition or route replacement.
OriginalActorRouteFollowerState_57c7cc originalNavStageFollowerState_57c7cc(
    const OriginalNavStageOwners_21ed30& owners,
    const OriginalNavStageRouteSnapshot& route,
    std::uint32_t originalActorStagePolygonIndex,
    const OriginalSpatialRecord70* activeControllerTarget=nullptr);
} // namespace ae3
