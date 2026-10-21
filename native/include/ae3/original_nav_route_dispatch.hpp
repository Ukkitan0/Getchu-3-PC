#pragma once
#include "ae3/original_nav_route_admission.hpp"
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>
namespace ae3 {
// Original 0x4E3DEC traverses a circular list and selects the first
// nearest member using strict float comparison, starting at FLT_MAX.
const OriginalSpatialRecord70* originalNavListNearest_4e3dec(
 std::span<const OriginalSpatialRecord70> records,
 std::span<const std::byte> originalNavBytes,
 std::span<const OriginalSpatialRecord70* const> orderedOriginalList,
 const OriginalVU0Vec4& reference);
// Original 0x2207D0: $7 is the optional list owner, $5 the seed
// polygon, $8 the explicit search index, and $9 the route mode.
// Seed outside $7's list invokes 0x21F8B8 / 0x4E3DEC fallback.
// Original context+8 stores that same owner.
struct OriginalNavDispatchRequest {
 std::uint32_t sourceSeedIndex=0; // original $5 remains the seed in fallback
 OriginalVU0Vec4 originalReference{};
 float maximumDistance=0;       // original F12
 std::uint32_t routeMode=1;     // original $9: 0 farthest, 1 nearest, else $8
 std::int32_t explicitDestinationSearchIndex=-1; // original $8
 bool sourceMembershipListExists=false; // original $7 owner non-null
 std::span<const OriginalSpatialRecord70* const> orderedOriginalMembershipList;
 // 0x22085C writes $7 into context+8; fallback reads that same owner.
};
struct OriginalNavDispatchResult {
 bool fallbackUsed=false;
 const OriginalSpatialRecord70* fallbackPolygon=nullptr;
 std::int32_t selectedEndpoint=-1;
 std::vector<std::uint32_t> originalAdmittedRecordIndices;
 std::vector<const OriginalSpatialRecord70*> forwardPath;
};
OriginalNavDispatchResult originalNavRouteDispatch_2207d0(
 std::span<const OriginalSpatialRecord70> records,
 std::span<const std::byte> navBytes,
 const OriginalNavDispatchRequest& request,
 OriginalNavRouteSplineCalls* unresolvedOriginalType1=nullptr);
// Original 0x220A4C/0x220A78 free original workspace[+0] through
// 0x41A3D8 only when a real allocation exists.
class OriginalNavRouteReleaseCalls {
public:
 virtual ~OriginalNavRouteReleaseCalls()=default;
 virtual void call_original_41a3d8(void* originalAllocation)=0;
};
void originalNavRouteRelease_220a4c(void*,OriginalNavRouteReleaseCalls&);
void originalNavRouteRelease_220a78(void*,OriginalNavRouteReleaseCalls&);
} // namespace ae3
