#pragma once
#include "ae3/original_navrange_vector_prefix.hpp"
#include <array>
#include <bit>
#include <cstdint>

namespace ae3 {
// Original register words are in little-endian, low-lane-first order.
using OriginalNavrangeGpr128 = std::array<std::uint32_t,4>;
using OriginalNavrangeMatrix4 = std::array<OriginalVU0Vec4,4>;
struct OriginalNavrangePackedVUInput {
    OriginalVU0Vec4 vf1,vf2,vf3,vf4;
};
namespace detail {
inline OriginalVU0Vec4 navrangeVF(OriginalNavrangeGpr128 words) noexcept {
    return {std::bit_cast<float>(words[0]),std::bit_cast<float>(words[1]),
            std::bit_cast<float>(words[2]),std::bit_cast<float>(words[3])};
}
}
// Retail 0x359E80 MTSAH zero,4: SA=64 bits.
// 0x359E84 QFSRV t5,a1,a1 (rotate halves of a1 into t5).
// 0x359E88 PEXCW t6,a1; 0x359E8C QFSRV t6,t6,t6;
// 0x359E90 PEXCW t6,t6; 0x359E94 QFSRV t7,t6,t6.
// 0x359E98/EA0/EAC/EBC: QMTC2 a1/t6/t5/t7 to VF1/2/3/4.
// Unlike a runtime instruction interpreter, this specializes the
// fixed original instructions into source-level native lane mappings.
inline OriginalNavrangePackedVUInput originalNavrangePack_359e80(
        OriginalNavrangeGpr128 gpr5) noexcept {
    const OriginalNavrangeGpr128 vf2{gpr5[1],gpr5[0],gpr5[3],gpr5[2]};
    const OriginalNavrangeGpr128 vf3{gpr5[2],gpr5[3],gpr5[0],gpr5[1]};
    const OriginalNavrangeGpr128 vf4{gpr5[3],gpr5[2],gpr5[1],gpr5[0]};
    return {detail::navrangeVF(gpr5),detail::navrangeVF(vf2),
            detail::navrangeVF(vf3),detail::navrangeVF(vf4)};
}
// Exact register preparation feeds the previously recovered source
// sequence at 0x359EA4..0x359EC8.
inline OriginalNavrangeVectorPrefix originalNavrangePrefixFromGpr_359e80(
        OriginalNavrangeGpr128 gpr5) noexcept {
    const auto prep=originalNavrangePack_359e80(gpr5);
    return originalNavrangePrefix_359ea4(prep.vf1,prep.vf2,prep.vf3,prep.vf4);
}
// Complete original quaternion -> matrix function at 0x359E74..0x359EF4.
// Source VF0=(0,0,0,1). SQC2 offsets at A0+0,+16,+32,+48
// store VF5,VF2,VF1,VF0, respectively. This is NOT the navrange
// polygon eligibility predicate; its caller/source are still incomplete.
inline OriginalNavrangeMatrix4 originalNavrangeQuaternionMatrix_359e74(
        OriginalNavrangeGpr128 gpr5) noexcept {
    auto r=originalNavrangePrefixFromGpr_359e80(gpr5);
    // 0x359ED4 VMULAz.xyzw ACC,VF4,VF1.z
    OriginalVU0Vec4 acc{r.vf4.x*r.vf1.z,r.vf4.y*r.vf1.z,
                        r.vf4.z*r.vf1.z,r.vf4.w*r.vf1.z};
    // 0x359ED8 VMADDAw.y ACC,VF6,VF0.w
    acc.y=acc.y+(r.vf6.y*1.f);
    // 0x359EDC VMSUBx.xyzw VF2,ACC,VF2,VF1.x
    r.vf2={acc.x-r.vf2.x*r.vf1.x,acc.y-r.vf2.y*r.vf1.x,
           acc.z-r.vf2.z*r.vf1.x,acc.w-r.vf2.w*r.vf1.x};
    // 0x359EE0 VMULAx.xyzw ACC,VF3,VF1.x
    acc={r.vf3.x*r.vf1.x,r.vf3.y*r.vf1.x,
         r.vf3.z*r.vf1.x,r.vf3.w*r.vf1.x};
    // 0x359EE4 VMADDAw.z ACC,VF6,VF0.w
    acc.z=acc.z+(r.vf6.z*1.f);
    // 0x359EEC VMSUBy.xyzw VF1,ACC,VF4,VF1.y
    r.vf1={acc.x-r.vf4.x*r.vf1.y,acc.y-r.vf4.y*r.vf1.y,
           acc.z-r.vf4.z*r.vf1.y,acc.w-r.vf4.w*r.vf1.y};
    // Original return at 0x359EF0; final VF1 store in delay slot.
    return {r.vf5,r.vf2,r.vf1,OriginalVU0Vec4{0.f,0.f,0.f,1.f}};
}
} // namespace ae3
