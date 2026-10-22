#include "ae3/original_actor_ray_geometry.hpp"
#include <cmath>
namespace ae3 {
float originalVU0DotXYZ_4748ac(const OriginalVU0Vec4& a,
                              const OriginalVU0Vec4& b) noexcept {
    // Original 0x4748B4 VMUL.xyzw VF1,VF1,VF2;
    // 0x4748B8 VADDy with source mask 0xD leaves Z intact;
    // 0x4748BC VADDz folds original Z into X.
    const float productX=a.x*b.x;
    const float productY=a.y*b.y;
    const float productZ=a.z*b.z;
    return (productX+productY)+productZ;
}
OriginalActorProjectionRay_4c1b3c originalActorProjectionRay_4c1b3c(
    const OriginalVU0Vec4& start,const OriginalVU0Vec4& end) noexcept {
    OriginalActorProjectionRay_4c1b3c out{};
    // 0x4C1B4C: VSUB.xyzw VF2=VF2-VF1
    out.originalDisplacement={end.x-start.x,end.y-start.y,
                              end.z-start.z,end.w-start.w};
    // 0x4C1B54..0x4C1B74: VMUL, VADDy, VADDz, SQRT.S
    const auto sum=originalVU0DotXYZ_4748ac(
        out.originalDisplacement,out.originalDisplacement);
    out.originalLengthF12=std::sqrt(sum);
    // Original 0x6D0194=1.0f and 0x4C1B98 DIV.S
    // Preserve source zero-length behavior; do not invent a game policy.
    const float reciprocal=1.0f/out.originalLengthF12;
    // 0x4C1BA4: VMULx.xyzw VF1=VF2*VF1.x
    out.originalNormalizedRay={
        out.originalDisplacement.x*reciprocal,
        out.originalDisplacement.y*reciprocal,
        out.originalDisplacement.z*reciprocal,
        out.originalDisplacement.w*reciprocal};
    return out;
}
} // namespace ae3
