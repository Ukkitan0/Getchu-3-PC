#pragma once
#include "ae3/original_navrange_local_coordinate.hpp"
#include "ae3/original_navrange_basis_projection.hpp"
#include "ae3/original_navrange_axis_score.hpp"
#include <array>
#include <bit>
#include <cstdint>

namespace ae3 {
// Original 0x21EFFC/0x21F094/0x21F15C MTSAB and QFSRV/PPACW
// preparations select +0x40 XYZ extents into distinct VU0 axes.
// Only original word bits are moved; the other lanes are integer zero.
inline std::array<OriginalVU0Vec4,3> originalNavrangeExtentBasis_21effc_21f178(
    const OriginalNavrangeGpr128& originalHeader40) noexcept {
    const float x=std::bit_cast<float>(originalHeader40[0]);
    const float y=std::bit_cast<float>(originalHeader40[1]);
    const float z=std::bit_cast<float>(originalHeader40[2]);
    return {{{x,0.f,0.f,0.f},{0.f,y,0.f,0.f},{0.f,0.f,z,0.f}}};
}

struct OriginalNavrangeRecoveredF20 {
    OriginalVU0Vec4 localCentroid{};
    std::array<float,3> originalHalfExtents{};
    std::array<float,3> projectedCoordinates{};
    float f20=0.f;
};

// Source-derived native mathematical path, no guest instruction runtime:
// +0x20 position, +0x30 full XYZW quaternion, +0x40 XYZ extents;
// original inverse rotation, centroid projection, three normalized basis
// directions, and three COP1 outside-bound accumulation paths.
// A result equal to zero allows a polygon into the retail owner list.
// This is NOT yet bit-exact to PS2 VU0 rounding, NaNs or zero-length
// VRSQRT, and has not been compared with live retail navrange allocations.
inline OriginalNavrangeRecoveredF20 originalNavrangeScore_21ef3c(
    const OriginalNavrangeResourceFrame& frame,
    const OriginalVU0Vec4& originalCentroid) noexcept {
    OriginalNavrangeRecoveredF20 result{};
    result.localCentroid=originalNavrangeLocalCentroidFromFrame_21efa4_21f074(
        frame,originalCentroid);
    const auto& ext=frame.argumentA1_40;
    result.originalHalfExtents={std::bit_cast<float>(ext[0]),
        std::bit_cast<float>(ext[1]),std::bit_cast<float>(ext[2])};
    const auto bases=originalNavrangeExtentBasis_21effc_21f178(ext);
    const auto projected=originalNavrangePreparedBasisProjections_21f024_21f1b8(
        result.localCentroid,bases);
    result.projectedCoordinates=projected.axisProjections;
    result.f20=originalNavrangeAxisScore_21f20c(
        result.projectedCoordinates,result.originalHalfExtents);
    return result;
}
} // namespace ae3
