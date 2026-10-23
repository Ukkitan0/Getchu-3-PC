#pragma once
#include "ae3/original_vu0_geometry.hpp"
namespace ae3 {
// Original retail 0x4748AC..0x4748CC: VMUL, VADDy, VADDz.
// Source VU0 arithmetic includes X/Y/Z and deliberately omits W.
float originalVU0DotXYZ_4748ac(const OriginalVU0Vec4& first,
                              const OriginalVU0Vec4& second) noexcept;
struct OriginalActorProjectionRay_4c1b3c {
    OriginalVU0Vec4 originalDisplacement{};
    float originalLengthF12=0.0f;
    OriginalVU0Vec4 originalNormalizedRay{};
};
// In original route advance, only FOLLOW-UP projection uses
// 0x57C7E4 PCPYUD s5,zero,a1 and 0x57C88C PCPYUD a1,zero,s5.
// First ray at 0x57C814 uses original caller a1 unchanged.
OriginalVU0Vec4 originalActorPackedSubsequentRayStart_57c88c(
    const OriginalVU0Vec4& originalIncomingA1) noexcept;

// Source 0x4C1B3C..0x4C1BA4: VSUB.xyzw, XYZ squared dot,
// SQRT.S, 1.0f/length and VMULx.xyzw. This is the verified
// preparation for original 0x21BD3C polygon intersection, which
// has not yet been completely recovered.
OriginalActorProjectionRay_4c1b3c originalActorProjectionRay_4c1b3c(
    const OriginalVU0Vec4& originalStart,
    const OriginalVU0Vec4& originalEnd) noexcept;
} // namespace ae3
