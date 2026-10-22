#pragma once
#include "ae3/original_actor_ray_geometry.hpp"
#include "ae3/original_polygon_edge_clip.hpp"
namespace ae3 {
// Native connected retail call path 0x4C1B3C ->
// 0x21BD3C -> 0x21EE00 -> 0x4748AC.
// Both caller vectors are separate source inputs:
// the actor's incoming $a1 and its controller+0x40 $a2.
struct OriginalPolygonRaycastResult_4c1b3c {
    OriginalActorProjectionRay_4c1b3c originalRay{};
    OriginalPolygonTraverseResult_21bd3c originalTraversal{};
};
OriginalPolygonRaycastResult_4c1b3c originalPolygonRaycast_4c1b3c(
    std::span<const OriginalSpatialRecord70> originalPolygonRegistry,
    std::span<const OriginalPolygonLinks_21bd3c> originalAdjacency,
    std::uint32_t originalSourcePolygonIndex,
    const OriginalVU0Vec4& originalCallerRayStart,
    const OriginalVU0Vec4& originalCallerRayEnd,
    std::size_t nativeMaximumRecursions=4096);
} // namespace ae3
