#pragma once
#include "ae3/original_spatial_index.hpp"
#include <cstddef>
#include <cstdint>
#include <span>

namespace ae3 {
// Native form of controller+0x1C, controller+0x18, and controller+0x50.
// Source polygon identities are stable pointers into decoded retail
// spatial records, not emulated PlayStation virtual addresses.
struct OriginalActorRouteFollowerState_57c7cc {
    std::span<const OriginalSpatialRecord70> sourceRecords;
    std::span<const std::uint32_t> originalRoutePolygonIndices;
    const OriginalSpatialRecord70* originalStagePolygonAtActor30=nullptr;
    const OriginalSpatialRecord70* activePolygonAtController50=nullptr;
    bool sourceHasMultiplePolygonsAtController1c=false;
};

// Original game-dependent geometry (0x4C1B3C) and controller target
// calculation (0x57C8EC) are explicit: they have VU0 operations not yet
// translated. The callbacks must faithfully compute those operations;
// neither operation is faked or replaced by a new path-finding algorithm.
class OriginalActorRouteFollowerCalls_57c7cc {
public:
    virtual ~OriginalActorRouteFollowerCalls_57c7cc()=default;
    virtual const OriginalSpatialRecord70* call_original_4c1b3c_project(
        const OriginalSpatialRecord70* originalActorStagePolygon)=0;
    virtual void call_original_57c8ec_update_target(
        const OriginalSpatialRecord70* originalPolygon)=0;
};

struct OriginalActorRouteFollowResult_57c7cc {
    bool returnedEarlyBecauseCurrentTargetMatched=false;
    std::size_t sourceTargetsApplied=0;
    std::size_t sourceProjectionsPerformed=0;
    // Original loop starts at route index 2; zero when absent.
    std::size_t followingRouteIndicesTested=0;
};

// Source 0x57C7CC..0x57C8E8: selects original route[1], applies
// subsequent entries while original 0x4C1B3C returns matching source
// polygon identity, and stops on first mismatch. In the no-multiple-
// polygon branch it instead selects actor-stage+0x30 immediately.
OriginalActorRouteFollowResult_57c7cc originalActorAdvanceRoute_57c7cc(
    OriginalActorRouteFollowerState_57c7cc& state,
    OriginalActorRouteFollowerCalls_57c7cc& originalCalls);
} // namespace ae3
