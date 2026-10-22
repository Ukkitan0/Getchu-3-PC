#pragma once
#include "ae3/original_nav_stage_route.hpp"
#include "ae3/original_navrange_recovered_eligibility.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace ae3 {

// Host-native lifetime for routes selected from a mutating original group.
struct OriginalNavStageGroupedRoute {
    OriginalNavStageRouteSnapshot sourceRoute;
    OriginalNavGroupKey20 originalGroupKey;
    std::uint64_t hostGroupSessionToken=0;
    std::uint64_t hostGroupRevision=0;
};

// Native bridge between original 0x4E4060 navrange group registration,
// original 0x21EF3C polygon eligibility, live 0x21ED30 graph owners,
// and 0x2207D0 source route dispatch. This is NOT a new retail object
// or an alternative pathfinding engine. One binding has exactly one
// original graph owner slot so registered list pointers remain valid.
class OriginalNavStageGroupSession final {
public:
    OriginalNavStageGroupSession(
        const OriginalNavStageOwners_21ed30& sourceOwners,
        std::size_t sourceOwnerSlot);
    OriginalNavStageGroupSession(const OriginalNavStageGroupSession&)=delete;
    OriginalNavStageGroupSession& operator=(const OriginalNavStageGroupSession&)=delete;
    OriginalNavStageGroupSession(OriginalNavStageGroupSession&&)=delete;
    OriginalNavStageGroupSession& operator=(OriginalNavStageGroupSession&&)=delete;

    // Original group source keys come from header+0x54/+0x5C, while
    // centroid geometry is read from the ACTIVE decoded original .nav.
    // The existing 0x21EF3C mathematical recovery is not bit-exact
    // VU0; stage-specific retail comparisons are still required.
    OriginalNavGroupBuildResult originalRegisterRange_4e4060(
        const OriginalNavGroupSource& originalRange);

    // The caller supplies the group key selected by original stage/
    // script control flow. An absent key is an explicit failure, not
    // guessed membership or an invented alternative destination.
    OriginalNavStageRouteSnapshot originalRouteByKey_2207d0(
        const OriginalNavGroupKey20& originalKey,
        OriginalNavDispatchRequest originalRequest) const;

    // Validates source graph AND navrange membership version.
    OriginalNavStageGroupedRoute originalBoundRouteByKey_2207d0(
        const OriginalNavGroupKey20& key,OriginalNavDispatchRequest request) const;
    bool originalGroupedRouteCurrent(
        const OriginalNavStageGroupedRoute& route) const noexcept;

    // Source graph reattachment/release invalidates prior group list
    // pointers. Native rebinding MUST erase the registry first.
    bool originalCurrentSourceGraph() const noexcept;
    void originalClear_4e3fec() noexcept;
    void originalRebindSourceGraph();
    std::size_t originalRegisteredGroupCount() const noexcept {
        return registry_.size();
    }
    const OriginalNavGroupRegistry& originalGroups() const noexcept {
        return registry_;
    }
private:
    const OriginalNavStageOwners_21ed30& owners_;
    std::size_t slot_;
    std::uint64_t ownerToken_;
    std::uint64_t ownerRevision_;
    void advanceGroupRevision() noexcept {
        ++groupRevision_;
        if(!groupRevision_)++groupRevision_;
    }
    inline static std::atomic<std::uint64_t> nextSessionToken_{1};
    const std::uint64_t groupSessionToken_=
        nextSessionToken_.fetch_add(1,std::memory_order_relaxed);
    std::uint64_t groupRevision_=1;
    OriginalNavGroupRegistry registry_{};
};
} // namespace ae3
