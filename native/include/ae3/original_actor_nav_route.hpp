#pragma once
#include <cstddef>
#include <cstdint>
#include <span>

namespace ae3 {
// Source-native, typed view of the original 0x221104 route-edge lookup.
// The original route stores 32-bit pointers. Native callers pass stable
// source polygon indexes obtained from the original decoded .nav graph.
// Source scans slots 0,1,2, calls 0x21B404 for each, and returns the first
// matching slot; the source's no-match result is ALSO zero.
std::uint32_t originalActorRouteEdgeSlot_221104(
    std::span<const std::uint32_t> sourceRoute,
    std::size_t segmentIndex,
    std::span<const std::byte> sourceNav);

// Original 0x21B444 reads the action kind at descriptor+0x0A+2*slot.
std::uint16_t originalActorRouteEdgeKind_21b444(
    std::uint32_t sourcePolygonIndex,
    std::uint32_t edgeSlot,
    std::span<const std::byte> sourceNav);

// 0x57BD88 invokes 0x221104 then 0x21B444, and treats kind==2 as
// the special original source-referenced action case. This describes
// a *real* edge decision, not an invented actor movement update.
struct OriginalActorRouteEdgeDecision_57bd88 {
    std::uint32_t sourcePolygonIndex=0;
    std::uint32_t destinationPolygonIndex=0;
    std::uint32_t sourceEdgeSlot=0;
    std::uint16_t originalActionKind=0;
    bool originalKindTwoBranch=false;
};
OriginalActorRouteEdgeDecision_57bd88 originalActorRouteEdgeDecision_57bd88(
    std::span<const std::uint32_t> sourceRoute,
    std::size_t segmentIndex,
    std::span<const std::byte> sourceNav);

// Source 0x575EC0: when route identity changes, destroy the old route's
// inner allocation through 0x220A78, free its outer allocation through
// 0x41A3B0, THEN replace the stored pointer. Same pointers are retained.
// The original allocator functions are explicit external dependencies.
class OriginalActorRouteLifetimeCalls_575ec0 {
public:
    virtual ~OriginalActorRouteLifetimeCalls_575ec0() = default;
    virtual void call_original_220a78(void* originalRoute) = 0;
    virtual void call_original_41a3b0(void* originalRoute) = 0;
};
void originalActorReplaceRoute_575ec0(
    void*& storedOriginalRoute,
    void* incomingOriginalRoute,
    OriginalActorRouteLifetimeCalls_575ec0& lifetime);
} // namespace ae3
