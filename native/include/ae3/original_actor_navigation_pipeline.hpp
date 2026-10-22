#pragma once
#include "ae3/original_actor_route_follower.hpp"
#include "ae3/original_actor_target_dispatch.hpp"
#include "ae3/original_vu0_geometry.hpp"
#include "ae3/original_actor_random_triangle.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace ae3 {
// Three genuine VU0 dependencies are not yet recovered. No synthesized
// collision, projection or steering is substituted for the retail code.
class OriginalActorUntranslatedMath_57c7cc {
public:
    virtual ~OriginalActorUntranslatedMath_57c7cc()=default;
    virtual const OriginalSpatialRecord70* call_original_4c1b3c_projection(
        const OriginalSpatialRecord70& actorStagePolygon,
        const OriginalVU0Vec4& originalControllerPoint)=0;
    virtual std::uint64_t call_original_57ca68_fallback(
        const OriginalSpatialRecord70& originalTarget)=0;
    virtual std::uint64_t call_original_57c98c_random_vertex_transform(
        const OriginalSpatialRecord70& originalTarget,
        const std::array<std::array<std::byte,16>,3>& originalVertices)=0;
};

// Connected original route advancement -> target dispatch -> recovered
// original 0x21B788 VU0 plane eligibility on the actual stage polygons.
// Inputs are decoded source stage records and original source vertices,
// never emulated EE pointers or hand-authored collision geometry.
class OriginalActorNavigationPipeline_57c7cc final:
    public OriginalActorRouteFollowerCalls_57c7cc,
    public OriginalActorTargetGeometryCalls_57c8ec {
public:
    OriginalActorNavigationPipeline_57c7cc(
        std::span<const OriginalSpatialRecord70> originalStageRecords,
        std::span<const OriginalSpatialSourcePolygon> originalStagePolygons,
        std::array<std::byte,16> originalControllerVector20,
        bool originalControllerVariant30,
        OriginalActorUntranslatedMath_57c7cc& unresolvedMath,
        OriginalGameRandom_36f3c0* originalSharedRandom=nullptr);

    OriginalActorRouteFollowResult_57c7cc advance(
        OriginalActorRouteFollowerState_57c7cc& originalActorRoute);
    std::span<const OriginalActorTargetResult_57c8ec> sourceTargetResults() const noexcept {
        return targetResults_;
    }
    const OriginalVU0Vec4& originalControllerPoint() const noexcept {return point_;}

private:
    const OriginalSpatialRecord70* call_original_4c1b3c_project(
        const OriginalSpatialRecord70*) override;
    void call_original_57c8ec_update_target(const OriginalSpatialRecord70*) override;
    bool call_original_21b788_eligibility(
        const OriginalSpatialRecord70&,float originalF12) override;
    std::uint64_t call_original_57ca68_geometry(
        const OriginalSpatialRecord70&) override;
    std::uint64_t call_original_57c98c_three_vertex_transform(
        const OriginalSpatialRecord70&,
        const std::array<std::array<std::byte,16>,3>&) override;
    std::size_t sourceIndex(const OriginalSpatialRecord70&) const;

    std::span<const OriginalSpatialRecord70> records_;
    std::span<const OriginalSpatialSourcePolygon> polygons_;
    std::array<std::byte,16> controllerVector20_{};
    OriginalVU0Vec4 point_{};
    bool variant30_=false;
    OriginalActorUntranslatedMath_57c7cc& math_;
    OriginalGameRandom_36f3c0* originalSharedRandom_=nullptr;
    std::vector<OriginalActorTargetResult_57c8ec> targetResults_;
};
} // namespace ae3
