#pragma once
#include "ae3/original_spatial_index.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace ae3 {
// Actual VF register data: four 32-bit lanes, original 16-byte LQC2.
struct alignas(16) OriginalVU0Vec4 {
    float x=0.0f,y=0.0f,z=0.0f,w=0.0f;
};
static_assert(sizeof(OriginalVU0Vec4)==16);

// 0x21B468..0x21B4DC: zero VF2; VADD.xyzw every point;
// DIV.S of 1.0f by uint16 point count; VMULx.xyzw the sum.
OriginalVU0Vec4 originalVU0Mean_21b468(
    std::span<const OriginalVU0Vec4> originalPoints);

// 0x21B210..0x21B234 and 0x21D950/0x21D9FC/0x21DA98:
// VSUB.xyzw, VMUL.xyzw, VADDy, VADDz, SQRT.S of X+Y+Z.
float originalVU0SquaredDistanceXYZ_21b210(
    const OriginalVU0Vec4&,const OriginalVU0Vec4&) noexcept;
float originalVU0DistanceXYZ_21b234(
    const OriginalVU0Vec4&,const OriginalVU0Vec4&) noexcept;
float originalVU0PolylineLength_21b194(
    std::span<const OriginalVU0Vec4> originalPoints) noexcept;

// Actual 0x21B788 VU0 plane loop: preserves source PEXTLW,
// PCPYUD, VOPMULA/VOPMSUB and dot-product reduction.
bool originalVU0PolygonEligible_21b788(
    std::span<const OriginalVU0Vec4> originalRing,
    const OriginalVU0Vec4& originalQuery,
    float originalMarginF12=0.0f) noexcept;

// The original 0x70-byte registry records refer to these resolved
// 16-byte source polygon vertices. No procedural replacement geometry.
struct OriginalSpatialSourcePolygon {
    std::span<const OriginalVU0Vec4> sourcePoints;
};
class OriginalSpatialOriginalEligibility {
public:
    virtual ~OriginalSpatialOriginalEligibility()=default;
    virtual bool call_original_21b788_planeEligibility(
        const OriginalSpatialRecord70&) = 0;
};

// Reuse actual original spatial selector order and strict distance test.
// One overload retains unresolved external eligibility; the overload
// without it runs the fully recovered 0x21B788 VU0 plane decision.
const OriginalSpatialRecord70* originalSpatialNearestWithVU0_21d898(
    std::span<const OriginalSpatialRecord70> originalRecords,
    std::span<const OriginalSpatialSourcePolygon> sourcePolygons,
    OriginalSpatialGridHits originalGridHits,
    const OriginalVU0Vec4& originalQuery,
    OriginalSpatialOriginalEligibility& originalEligibility);
const OriginalSpatialRecord70* originalSpatialNearestGeometry_21d898(
    std::span<const OriginalSpatialRecord70> originalRecords,
    std::span<const OriginalSpatialSourcePolygon> sourcePolygons,
    OriginalSpatialGridHits originalGridHits,
    const OriginalVU0Vec4& originalQuery);
const OriginalSpatialRecord70* originalSpatialNearestAllWithVU0_21da2c(
    std::span<const OriginalSpatialRecord70> originalRecords,
    std::span<const OriginalSpatialSourcePolygon> sourcePolygons,
    const OriginalVU0Vec4& originalQuery);
} // namespace ae3
