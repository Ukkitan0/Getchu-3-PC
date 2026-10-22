#pragma once
#include "ae3/original_actor_navigation_pipeline.hpp"
#include "ae3/original_stage_polygon_raycast.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace ae3 {
// The original actor movement pipeline consumes source record pointers,
// their owner-provided polygon headers/vertices, and real header links.
// This class owns the geometry/header views until the stage is unloaded,
// avoiding separate manually fabricated graph and polygon arguments.
class OriginalStageActorNavigation_57c7cc {
public:
    OriginalStageActorNavigation_57c7cc(
        const OriginalStageSpatialSource& originalSource,
        const OriginalStageSpatialLoaded& originalStage,
        std::array<std::byte,16> originalControllerVector20,
        bool originalControllerVariant30,
        OriginalActorUntranslatedMath_57c7cc& remainingSourceMath,
        OriginalGameRandom_36f3c0* originalGameRandom=nullptr,
        OriginalActorFallbackVU0Transform_57ca68* originalFallbackTransform=nullptr);

    OriginalStageActorNavigation_57c7cc(const OriginalStageActorNavigation_57c7cc&)=delete;
    OriginalStageActorNavigation_57c7cc& operator=(const OriginalStageActorNavigation_57c7cc&)=delete;

    // These are the TWO original arguments to 0x4C1B3C at 0x57C814.
    // The +0x20 vector is intentionally NOT used for this ray.
    void setOriginalMovementVectors_57c7cc(
        const OriginalVU0Vec4& incomingA1,
        const OriginalVU0Vec4& originalController40);

    // Original fallback a1 is actor owner->+0x14->+0x40,
    // not actor movement input or controller+0x20.
    void setOriginalFallbackOwnerVector_57ca68(
        const std::array<std::byte,16>& originalOwner14Vector40);
    void setOriginalFallbackQBits_57ca68(std::uint32_t rawOriginalQ);

    // The route span belongs to the caller and must remain valid
    // through advance(). Returned polygon identities belong to stage.
    OriginalActorRouteFollowResult_57c7cc advance(
        std::span<const std::uint32_t> originalRoutePolygonIndices,
        std::uint32_t originalActorStagePolygonIndex,
        bool originalControllerHasMultipleRoutePolygons);

    const OriginalSpatialRecord70* currentOriginalTargetAt50() const noexcept {
        return currentTargetAt50_;
    }
    void setOriginalTargetAt50(const OriginalSpatialRecord70* previousTarget);

    std::span<const OriginalActorTargetResult_57c8ec> originalTargetResults() const noexcept {
        return pipeline_.sourceTargetResults();
    }
    std::span<const OriginalPolygonLinks_21bd3c> originalSourceAdjacency() const noexcept {
        return adjacency_;
    }

private:
    const OriginalStageSpatialLoaded& stage_;
    OriginalResolvedSpatialGeometry sourceGeometry_;
    std::vector<OriginalPolygonLinks_21bd3c> adjacency_;
    OriginalActorNavigationPipeline_57c7cc pipeline_;
    const OriginalSpatialRecord70* currentTargetAt50_=nullptr;
};
} // namespace ae3
