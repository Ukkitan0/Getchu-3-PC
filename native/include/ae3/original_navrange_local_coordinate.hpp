#pragma once
#include "ae3/original_navrange_source_frame.hpp"
#include <array>
#include <bit>

namespace ae3 {
// Retail 0x21F00C..0x21F074: VF3 = source +0x20, VF1 = centroid,
// 0x21F020 VSUB.xyzw forms (centroid - source +0x20).
// Retail 0x21F030/+0x21F04C/+0x21F060/+0x21F070 read the four
// output columns of the inverse quaternion matrix from V0.
// The original VMULAx, VMADDAy, VMADDAz and VMADDw produce a
// four-lane homogeneous local vector in VF2. No normalization or
// eligibility decision is made by this function.
inline OriginalVU0Vec4 originalNavrangeLocalCoordinate_21f020_21f074(
    const OriginalNavrangeMatrix4& originalInverse,
    const OriginalVU0Vec4& originalPolygonCentroid,
    const OriginalVU0Vec4& originalSource20) noexcept {
    const OriginalVU0Vec4 delta{
        originalPolygonCentroid.x-originalSource20.x,
        originalPolygonCentroid.y-originalSource20.y,
        originalPolygonCentroid.z-originalSource20.z,
        originalPolygonCentroid.w-originalSource20.w};
    const auto& c=originalInverse;
    // VU ACC updates in original instruction order per lane.
    OriginalVU0Vec4 transformed{
        c[0].x*delta.x,c[0].y*delta.x,
        c[0].z*delta.x,c[0].w*delta.x};
    transformed.x+=c[1].x*delta.y;
    transformed.y+=c[1].y*delta.y;
    transformed.z+=c[1].z*delta.y;
    transformed.w+=c[1].w*delta.y;
    transformed.x+=c[2].x*delta.z;
    transformed.y+=c[2].y*delta.z;
    transformed.z+=c[2].z*delta.z;
    transformed.w+=c[2].w*delta.z;
    transformed.x+=c[3].x*delta.w;
    transformed.y+=c[3].y*delta.w;
    transformed.z+=c[3].z*delta.w;
    transformed.w+=c[3].w*delta.w;
    return transformed;
}
inline OriginalVU0Vec4 originalNavrangeLocalCentroidFromFrame_21efa4_21f074(
    const OriginalNavrangeResourceFrame& frame,
    const OriginalVU0Vec4& originalPolygonCentroid) noexcept {
    const auto& raw=frame.argumentA0_20;
    const OriginalVU0Vec4 origin{
        std::bit_cast<float>(raw[0]),std::bit_cast<float>(raw[1]),
        std::bit_cast<float>(raw[2]),std::bit_cast<float>(raw[3])};
    return originalNavrangeLocalCoordinate_21f020_21f074(
        originalNavrangeFrameInverse_21ef9c_21efdc(frame),
        originalPolygonCentroid,origin);
}
} // namespace ae3
