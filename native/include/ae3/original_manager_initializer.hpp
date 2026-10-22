#pragma once
#include "ae3/original_manager_child.hpp"
#include "ae3/original_actor_nav_route.hpp"
#include "ae3/original_actor_type2_target.hpp"
#include "ae3/original_actor_random_triangle.hpp"

namespace ae3 {
// Native typed ownership view, not an EE memory layout. Original constructor
// fields remain in the existing lifecycle object; vectors/polygon references
// must be owned by the real actor/decoded stage, not synthesized by this port.
struct OriginalManagerInitState_57bcb8 {
    OriginalManagerChild70& lifecycle;
    OriginalVU0Vec4 controller20{};
    OriginalVU0Vec4 target40{};
    const OriginalSpatialRecord70* target50=nullptr;
    OriginalVU0Vec4 midpoint60{};
};
struct OriginalManagerInitSource_57bcb8 {
    OriginalVU0Vec4 ownerPosition40{}; // parent+0x14 -> +0x40
    OriginalVU0Vec4 ownerDirection50{}; // parent+0x14 -> +0x50
    const OriginalSpatialRecord70* actorPolygon30=nullptr;
    std::span<const OriginalSpatialRecord70> stageRecords;
    std::span<const std::uint32_t> routePolygonIds; // context+0 -> route
    std::span<const std::byte> originalNavigation;
};
class OriginalManagerInitCalls_57bcb8 {
public:
    virtual ~OriginalManagerInitCalls_57bcb8()=default;
    // General branch 0x57BF00: retain the actual original packed-vector
    // return as a typed full vector. No invented projection is permitted.
    virtual OriginalVU0Vec4 call_original_21b854(
        const OriginalSpatialRecord70&,const OriginalVU0Vec4& start,
        const OriginalVU0Vec4& end)=0;
    // Existing native route follower/pipeline is the implementation seam;
    // its actual owner/ray operands must be provided by the actor manager.
    virtual void call_original_57c7cc(OriginalManagerInitState_57bcb8&,
                                    const OriginalVU0Vec4& originalIncomingA1)=0;
};
enum class OriginalManagerInitBranch_57bcb8 { nearOwnerReturn, typeTwoTarget, generalProjection };

// Entire source control flow 0x57BCB8..0x57BF58. Reuses already recovered
// native route edge lookup, type-2 polygon lookup and random triangle math.
// The projection and following actor update remain explicit dependencies.
OriginalManagerInitBranch_57bcb8 originalManagerInitialize_57bcb8(
    OriginalManagerInitState_57bcb8&,const OriginalManagerInitSource_57bcb8&,
    OriginalGameRandom_36f3c0&,OriginalManagerInitCalls_57bcb8&);
} // namespace ae3
