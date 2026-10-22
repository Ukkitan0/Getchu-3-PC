#pragma once
#include "ae3/original_polygon_traversal.hpp"

namespace ae3 {
// Fully recovered original 0x21EE00..0x21EF3C source half-plane
// intersection: five calls to source 0x4748AC, original 1e-6
// parallel tolerance, and source MAX.S/MIN.S bounds updating.
bool originalPolygonClipEdge_21ee00(
    const OriginalVU0Vec4& originalEdgeStart,
    const OriginalVU0Vec4& originalEdgeEnd,
    const OriginalVU0Vec4& originalRayStart,
    const OriginalVU0Vec4& originalRayDirection,
    OriginalPolygonRayDistanceBounds_21bd3c& originalBounds) noexcept;

// Connects the recovered original edge clip arithmetic directly
// to source 0x21BD3C recursive polygon traversal.
class OriginalPolygonNativeEdgeClip_21ee00 final:
  public OriginalPolygonEdgeIntersection_21ee00 {
public:
    bool call_original_21ee00(
        const OriginalVU0Vec4& originalEdgeStart,
        const OriginalVU0Vec4& originalEdgeEnd,
        const OriginalVU0Vec4& originalRayStart,
        const OriginalVU0Vec4& originalRayDirection,
        OriginalPolygonRayDistanceBounds_21bd3c& originalBounds) override {
        return originalPolygonClipEdge_21ee00(
            originalEdgeStart,originalEdgeEnd,
            originalRayStart,originalRayDirection,originalBounds);
    }
};
} // namespace ae3
