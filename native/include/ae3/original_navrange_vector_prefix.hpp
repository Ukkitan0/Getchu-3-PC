#pragma once
#include "ae3/original_vu0_geometry.hpp"
namespace ae3 {
inline void originalNavrangeBarrier_441f40() noexcept {}
struct OriginalNavrangeVectorPrefix {
    OriginalVU0Vec4 vf1, vf2, vf3, vf4, vf5, vf6;
};
inline OriginalNavrangeVectorPrefix originalNavrangePrefix_359ea4(
    OriginalVU0Vec4 vf1, OriginalVU0Vec4 vf2,
    OriginalVU0Vec4 vf3, OriginalVU0Vec4 vf4) noexcept {
    // Authentic USA 0x359EA4: VADD.xyzw VF1,VF1,VF1
    vf1={vf1.x+vf1.x,vf1.y+vf1.y,vf1.z+vf1.z,vf1.w+vf1.w};
    // 0x359EA8: VSUB.xz VF2,VF0,VF2
    vf2.x=0.0f-vf2.x; vf2.z=0.0f-vf2.z;
    // 0x359EB0: VADDw.xyzw VF6,VF0,VF0.w (VF0=(0,0,0,1))
    const OriginalVU0Vec4 vf6{1.f,1.f,1.f,2.f};
    // 0x359EB4: VSUB.yz VF3,VF0,VF3
    vf3.y=0.0f-vf3.y; vf3.z=0.0f-vf3.z;
    // 0x359EB8: VMULAy.xyzw ACC,VF2,VF1.y
    OriginalVU0Vec4 acc{vf2.x*vf1.y,vf2.y*vf1.y,vf2.z*vf1.y,vf2.w*vf1.y};
    // 0x359EC0: VMADDAw.x ACC,VF6,VF0.w
    acc.x=acc.x+vf6.x*1.0f;
    // 0x359EC4: VMSUBz.xyzw VF5,ACC,VF3*VF1.z
    const OriginalVU0Vec4 vf5{
        acc.x-vf3.x*vf1.z,acc.y-vf3.y*vf1.z,
        acc.z-vf3.z*vf1.z,acc.w-vf3.w*vf1.z};
    // 0x359EC8: VSUB.xy VF4,VF0,VF4
    vf4.x=0.0f-vf4.x; vf4.y=0.0f-vf4.y;
    return {vf1,vf2,vf3,vf4,vf5,vf6};
}
} // namespace ae3
