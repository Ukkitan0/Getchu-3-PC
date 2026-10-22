#include "ae3/original_polygon_raycast.hpp"
namespace ae3 {
OriginalPolygonRaycastResult_4c1b3c originalPolygonRaycast_4c1b3c(
    std::span<const OriginalSpatialRecord70> sourceRecords,
    std::span<const OriginalPolygonLinks_21bd3c> sourceAdjacency,
    std::uint32_t sourcePolygon,
    const OriginalVU0Vec4& sourceRayStart,
    const OriginalVU0Vec4& sourceRayEnd,
    std::size_t nativeMaxRecursions) {
    OriginalPolygonRaycastResult_4c1b3c out{};
    // 0x4C1B4C..0x4C1BA4: the already recovered original
    // scalar/VU0 ray preparation and normalization.
    out.originalRay=originalActorProjectionRay_4c1b3c(sourceRayStart,sourceRayEnd);
    OriginalPolygonNativeEdgeClip_21ee00 sourceEdgeClip;
    // 0x4C1BA8 JAL original 0x21BD3C -> 3 original 0x21EE00
    // calls per polygon, original kind-zero neighbor recursion.
    out.originalTraversal=originalPolygonTraverse_21bd3c(
        sourceRecords,sourceAdjacency,sourcePolygon,
        sourceRayStart,out.originalRay.originalNormalizedRay,
        out.originalRay.originalLengthF12,sourceEdgeClip,nativeMaxRecursions);
    return out;
}
} // namespace ae3
