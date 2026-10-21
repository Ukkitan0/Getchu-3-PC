#pragma once
#include "ae3/original_vu0_geometry.hpp"
#include <array>
#include <optional>

namespace ae3 {
// Retail 0x21F3C8..0x21F3FC, where s8 is the output-enable flag.
// Unlike the original optional stored 16-byte vector, this native
// source-level recovery returns an optional value by ownership.
// The normal retail navrange path passes t0=0 (source 0x21F754),
// so it returns no vector. It still returns F20 verbatim as F0.
struct OriginalNavrangeEligibilityFinish {
    float sourceScalarF0;
    std::optional<OriginalVU0Vec4> optionalResult;
};

inline OriginalNavrangeEligibilityFinish originalNavrangeFinish_21f3c8(
    float originalAccumF20, bool requestedOutput,
    const std::array<OriginalVU0Vec4,4>& originalInverseColumns,
    const OriginalVU0Vec4& preparedSourceVector360,
    const OriginalVU0Vec4& originalHeaderVector20) noexcept {
    if (!requestedOutput)
        return {originalAccumF20, std::nullopt};
    // Retail VMULAx at 0x21F3D4, VMADDAy at +0x21F3DC,
    // VMADDAz at 0x21F3E4, and VMADDw at 0x21F3EC:
    // the four source columns are loaded from sp+0x00..+0x30.
    OriginalVU0Vec4 product{};
    const auto& c=originalInverseColumns;
    const auto& v=preparedSourceVector360;
    product.x=c[0].x*v.x;
    product.y=c[0].y*v.x;
    product.z=c[0].z*v.x;
    product.w=c[0].w*v.x;
    product.x=product.x+c[1].x*v.y;
    product.y=product.y+c[1].y*v.y;
    product.z=product.z+c[1].z*v.y;
    product.w=product.w+c[1].w*v.y;
    product.x=product.x+c[2].x*v.z;
    product.y=product.y+c[2].y*v.z;
    product.z=product.z+c[2].z*v.z;
    product.w=product.w+c[2].w*v.z;
    product.x=product.x+c[3].x*v.w;
    product.y=product.y+c[3].y*v.w;
    product.z=product.z+c[3].z*v.w;
    product.w=product.w+c[3].w*v.w;
    // 0x21F3F0 QMTC2 s7,VF2; +0x21F3F4 VADD.xyzw VF1,VF1,VF2;
    // +0x21F3F8 SQC2 VF1,0(s8). s7 is resource header+0x20.
    product.x=product.x+originalHeaderVector20.x;
    product.y=product.y+originalHeaderVector20.y;
    product.z=product.z+originalHeaderVector20.z;
    product.w=product.w+originalHeaderVector20.w;
    return {originalAccumF20,product};
}
} // namespace ae3
