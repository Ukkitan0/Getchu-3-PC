#pragma once
#include "ae3/original_actor_route_follower.hpp"
#include "ae3/original_actor_target_dispatch.hpp"
#include "ae3/original_vu0_geometry.hpp"
#include "ae3/original_actor_random_triangle.hpp"
#include "ae3/original_actor_fallback_sampler.hpp"
#include "ae3/original_actor_fallback_vu0_native.hpp"
#include "ae3/original_polygon_raycast.hpp"
#include <optional>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace ae3 {
// Legacy callback retains strictly unresolved/injection-only source paths.
// The genuine 0x57CA68 math is now translated natively whenever the
// original caller supplies its shared RNG, owner vector and Q bits.
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
        OriginalGameRandom_36f3c0* originalSharedRandom=nullptr,
        std::span<const OriginalPolygonLinks_21bd3c> originalSourceAdjacency={},
        OriginalActorFallbackVU0Transform_57ca68* originalFallbackTransform=nullptr);

    // Original 0x57C7CC $a1 and controller+0x40. Keep these distinct
    // from +0x20, which is used only by 0x21B788 eligibility.
    // Caller refreshes source position each original actor update.
    void setOriginalProjectionOperands_57c7cc(
        const OriginalVU0Vec4& originalIncomingA1,
        const OriginalVU0Vec4& originalControllerVector40);

    // Original 0x57C978 loads owner[+0x14], and 0x57C980
    // loads packed +0x40 into the fallback's $a1. This is
    // distinct from the $a2 loaded at controller+0x20.
    void setOriginalFallbackOwnerVector_57ca68(
        const std::array<std::byte,16>& originalOwner14Vector40);
    // VU0 Q is caller-owned COP2 state. This original routine invokes
    // VMULq four times in two source blocks but never writes Q itself.
    void setOriginalFallbackQBits_57ca68(std::uint32_t rawOriginalQ);
    // Optional binding to original VU0 R state. VRINIT/VRXOR run twice per
    // candidate, independent from the original EE MT19937 draw stream.
    // Borrowed caller-owned state must outlive each actor advance().
    void setOriginalFallbackRState_57ca68(OriginalActorFallbackVu0R_57ca68* state) noexcept {
        originalFallbackR_=state;
    }
    // Genuine VRINIT/VRXOR effects are retained even if the owning
    // engine has not bound its shared global VU0 R yet. Binding external
    // state is still required for whole-engine VI[R] synchronization.
    const OriginalActorFallbackVu0R_57ca68& sourceFallbackRState_57ca68() const noexcept {
        return originalFallbackR_?*originalFallbackR_:localSourceVu0R_;
    }

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
    std::uint64_t call_original_57ca68_geometry_high64() const override {
        return latestFallbackHigh64_;
    }
    std::uint64_t call_original_57c98c_geometry_high64() const override {
        return latestTriangleHigh64_;
    }
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
    OriginalActorFallbackVU0Transform_57ca68* originalFallbackTransform_=nullptr;
    OriginalActorFallbackNativeVU0_57ca68 nativeFallbackTransform_{};
    std::uint64_t latestFallbackHigh64_=0;
    std::span<const OriginalPolygonLinks_21bd3c> originalSourceAdjacency_;
    std::optional<std::array<std::byte,16>> originalOwner14Vector40_;
    std::optional<std::uint32_t> originalFallbackQBits_;
    OriginalActorFallbackVu0R_57ca68* originalFallbackR_=nullptr;
    OriginalActorFallbackVu0R_57ca68 localSourceVu0R_{};
    std::uint64_t latestTriangleHigh64_=0;
    std::optional<OriginalVU0Vec4> originalIncomingRayA1_;
    std::optional<OriginalVU0Vec4> originalController40_;
    std::optional<OriginalVU0Vec4> latestTargetRayA2_;
    std::vector<OriginalActorTargetResult_57c8ec> targetResults_;
};
} // namespace ae3
