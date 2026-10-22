#pragma once
#include "ae3/original_vu0_geometry.hpp"
#include "ae3/original_spatial_index.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace ae3 {
// Original polygon header+0x02, +0x04 and +0x0A via
// 0x21B3F8, 0x21B404 and 0x21B444.
struct OriginalPolygonLink_21bd3c {
    std::uint32_t neighborSourceIndex=0;
    std::uint16_t originalActionKind=0;
};
struct OriginalPolygonLinks_21bd3c {
    std::array<OriginalPolygonLink_21bd3c,3> edges{};
    std::uint16_t edgeCount=0;
};

// Source 0x21BD7C / 0x21BD90: mutable bounds packet passed
// to original 0x21EE00 for three consecutive edge tests.
struct OriginalPolygonRayDistanceBounds_21bd3c {
    float originalLowerAt20=0.0f;
    float originalUpperAt24=0.0f;
};
class OriginalPolygonEdgeIntersection_21ee00 {
public:
    virtual ~OriginalPolygonEdgeIntersection_21ee00()=default;
    // Original edge-plane intersection at 0x21EE00 is NOT
    // reconstructed by this routine. It must be genuinely ported.
    virtual bool call_original_21ee00(
        const OriginalVU0Vec4& originalEdgeFrom,
        const OriginalVU0Vec4& originalEdgeTo,
        const OriginalVU0Vec4& originalRayOrigin,
        const OriginalVU0Vec4& originalRayDirection,
        OriginalPolygonRayDistanceBounds_21bd3c& originalBounds)=0;
};
struct OriginalPolygonTraverseResult_21bd3c {
    std::optional<OriginalVU0Vec4> sourceHitPoint;
    const OriginalSpatialRecord70* originalLastAcceptedPolygon=nullptr;
    std::size_t sourcePolygonsTested=0;
    std::size_t sourceEdgeTests=0;
    std::size_t sourceZeroKindRecursions=0;
};

// Exact original 0x21BD3C..0x21BEA4 loop / edge-test dispatch,
// source ray progress, recursion and kind-zero neighbor filtering.
// The separate 0x21EE00 intersection arithmetic is a dependency,
// not a substitute or fabricated game geometry.
OriginalPolygonTraverseResult_21bd3c originalPolygonTraverse_21bd3c(
    std::span<const OriginalSpatialRecord70> originalPolygons,
    std::span<const OriginalPolygonLinks_21bd3c> originalHeaderLinks,
    std::uint32_t originalStartPolygonIndex,
    const OriginalVU0Vec4& originalRayOrigin,
    const OriginalVU0Vec4& originalNormalizedRay,
    float originalRemainingRayLength,
    OriginalPolygonEdgeIntersection_21ee00& originalIntersection,
    std::size_t maxOriginalCalls=4096);
} // namespace ae3
