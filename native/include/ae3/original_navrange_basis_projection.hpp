#pragma once
#include "ae3/original_vu0_geometry.hpp"
#include <array>
#include <cmath>

namespace ae3 {
// Source-derived VF0.w/sqrt(VFn.x) normalization in the three fixed
// 0x21EF3C projection subgraphs. Input VFn is a PREPARED source vector,
// not the raw navrange +0x40 header or a substituted axis.
// Each norm uses original VMUL, VADDy.xyw, VADDz.xyzw and VRSQRT.
struct OriginalNavrangeNormalizedBasis {
    OriginalVU0Vec4 value{};
    float squaredXYZ=0.f;
    float reciprocalLength=0.f;
};
inline OriginalNavrangeNormalizedBasis originalNavrangeNormalizeBasis_21f024(
    OriginalVU0Vec4 source) noexcept {
    const float xx=source.x*source.x;
    const float yy=source.y*source.y;
    const float zz=source.z*source.z;
    float squared=xx+yy; // VADDy.xyw on square vector, VF.x
    squared=squared+zz;  // VADDz.xyzw, VF.x
    const float q=1.0f/std::sqrt(squared); // VRSQRT Q, VF0.w, VF.x
    return {{source.x*q,source.y*q,source.z*q,source.w*q},squared,q};
}
// Native equivalent of the original componentwise VMUL followed by
// VADDy.xyw and VADDz.xyzw, selecting the resulting x lane.
inline float originalNavrangeProjectionXYZ_21f0b0(
    const OriginalVU0Vec4& localCoordinate,
    const OriginalVU0Vec4& normalizedBasis) noexcept {
    const float x=localCoordinate.x*normalizedBasis.x;
    const float y=localCoordinate.y*normalizedBasis.y;
    const float z=localCoordinate.z*normalizedBasis.z;
    const float xy=x+y;
    return xy+z;
}
struct OriginalNavrangePreparedProjection3 {
    std::array<float,3> axisProjections{};
    std::array<OriginalNavrangeNormalizedBasis,3> bases{};
};
// Original 0x21F024..0x21F1B8 has three normalization+dot stages.
// This function accepts the VF5/VF5/VF3 input vectors as they were
// prepared by separate original MMI packing stages. That preceding
// source-register preparation is still unresolved; do not use this
// as a production replacement for full 0x21EF3C eligibility.
inline OriginalNavrangePreparedProjection3 originalNavrangePreparedBasisProjections_21f024_21f1b8(
    const OriginalVU0Vec4& originalLocalCoordinate,
    const std::array<OriginalVU0Vec4,3>& preparedBasisVectors) noexcept {
    OriginalNavrangePreparedProjection3 result{};
    for(std::size_t i=0;i<3;++i) {
        result.bases[i]=originalNavrangeNormalizeBasis_21f024(preparedBasisVectors[i]);
        result.axisProjections[i]=originalNavrangeProjectionXYZ_21f0b0(
            originalLocalCoordinate,result.bases[i].value);
    }
    return result;
}
} // namespace ae3
