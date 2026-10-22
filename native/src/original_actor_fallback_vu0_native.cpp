#include "ae3/original_actor_fallback_vu0_native.hpp"
#include <bit>
#include <cstdint>
#include <cstring>
#include <stdexcept>
namespace ae3 {
namespace {
// Fixed source op translations only, NOT generic vector instruction decoding.
OriginalVU0Vec4 upper64_57ca68(const std::array<std::byte,16>& b) noexcept {
    // PCPYUD s*,zero,$a1/$a2/$v0: upper original half in VF.zw,
    // lower 64 bits zero. QMTC2 transfers the full 128 bits.
    std::uint32_t z=0,w=0;
    std::memcpy(&z,b.data()+8,4);
    std::memcpy(&w,b.data()+12,4);
    return {0.0f,0.0f,std::bit_cast<float>(z),std::bit_cast<float>(w)};
}
std::uint64_t upper64bits_57ca68(const OriginalVU0Vec4& v) noexcept {
    return std::uint64_t(std::bit_cast<std::uint32_t>(v.z)) |
          (std::uint64_t(std::bit_cast<std::uint32_t>(v.w))<<32);
}
OriginalVU0Vec4 vmulx(const OriginalVU0Vec4& a,float bx) noexcept {
    return {a.x*bx,a.y*bx,a.z*bx,a.w*bx};
}
OriginalVU0Vec4 vmulq(const OriginalVU0Vec4& a,float q) noexcept {
    return {a.x*q,a.y*q,a.z*q,a.w*q};
}
OriginalVU0Vec4 vadd(const OriginalVU0Vec4& a,const OriginalVU0Vec4& b) noexcept {
    return {a.x+b.x,a.y+b.y,a.z+b.z,a.w+b.w};
}
OriginalVU0Vec4 vsub(const OriginalVU0Vec4& a,const OriginalVU0Vec4& b) noexcept {
    return {a.x-b.x,a.y-b.y,a.z-b.z,a.w-b.w};
}
OriginalVU0Vec4 vmul(const OriginalVU0Vec4& a,const OriginalVU0Vec4& b) noexcept {
    return {a.x*b.x,a.y*b.y,a.z*b.z,a.w*b.w};
}
// 0x57CB88,0x57CBA8,0x57CC80,0x57CCA0: VADDy destination mask xyw.
OriginalVU0Vec4 vaddy_xyw(const OriginalVU0Vec4& a) noexcept {
    return {a.x+a.y,a.y+a.y,a.z,a.w+a.y};
}
OriginalVU0Vec4 vaddz_xyzw(const OriginalVU0Vec4& a) noexcept {
    return {a.x+a.z,a.y+a.z,a.z+a.z,a.w+a.z};
}
}
OriginalActorFallbackCandidate_57ca68
OriginalActorFallbackNativeVU0_57ca68::call_original_57ca68_candidate_VU0(
    const std::array<std::array<std::byte,16>,3>& vertices,
    const std::array<std::byte,16>& owner,
    const std::array<std::byte,16>& controller,
    std::uint32_t qBits,
    const OriginalActorTriangleWeights_57c98c& weights,
    std::size_t trial) {
    if(trial>=kOriginalFallbackCandidates_57ca68)
        throw std::out_of_range("Original fallback candidate index must be 0..5");
    const auto v0=upper64_57ca68(vertices[0]); // PCPYUD s5,zero,v0
    const auto v1=upper64_57ca68(vertices[1]); // PCPYUD s4,zero,v0
    const auto v2=upper64_57ca68(vertices[2]); // PCPYUD s3,zero,v0
    const auto controller20=upper64_57ca68(controller); // PCPYUD s6
    const auto owner40=upper64_57ca68(owner);            // PCPYUD s7
    const float q=std::bit_cast<float>(qBits);  // caller-provided original Q
    const float c0=weights.originalWeightedCoefficients[0];
    const float c1=weights.originalWeightedCoefficients[1];
    const float c2=weights.originalWeightedCoefficients[2];
    OriginalVU0Vec4 vf1{},vf2{},vf3{},vf4{},vf5{};
    OriginalVU0Vec4 selected{};
    if(trial==0) {
        // Initial candidate has a unique VMOVE (0x57CB74).
        vf4=v1;                              // 57CB14 QMTC2 s4,vf4
        vf3=v0;                              // 57CB1C QMTC2 s5,vf3
        vf1=vmulx(vf3,c0);                  // 57CB48/4C
        vf2=vmulx(vf4,c1);                  // 57CB54/58
        vf4=v2;                             // 57CB64
        vf1=vadd(vf1,vf2);                  // 57CB68
        vf3=vmulx(vf4,c2);                  // 57CB6C
        vf3=vadd(vf1,vf3);                  // 57CB70
        vf2=vf3;                            // 57CB74 VMOVE vf2,vf3
        vf1=controller20;                   // 57CB78
        selected=vf3;                       // 57CB7C QMFC2 s1,vf3
        vf3=vsub(vf1,vf3);                  // 57CB80
        vf1=vmul(vf3,vf3);                  // 57CB84
        vf1=vaddy_xyw(vf1);                // 57CB88
        vf1=vaddz_xyzw(vf1);               // 57CB8C
    } else {
        // Subsequent candidate updates use different VF registers.
        vf4=v0;                             // 57CC10 QMTC2 s5,vf4
        vf5=v1;                             // 57CC18 QMTC2 s4,vf5
        vf2=vmulx(vf4,c0);                 // 57CC44/48
        vf4=v2;                             // 57CC54 QMTC2 s3,vf4
        vf1=vmulx(vf5,c1);                 // 57CC58
        vf2=vadd(vf2,vf1);                 // 57CC64
        vf3=vmulx(vf4,c2);                 // 57CC68
        vf1=controller20;                  // 57CC6C
        vf2=vadd(vf2,vf3);                 // 57CC70
        vf3=vsub(vf1,vf2);                 // 57CC74
        selected=vf2;                      // 57CC78 QMFC2 s0,vf2
        vf1=vmul(vf3,vf3);                 // 57CC7C
        vf1=vaddy_xyw(vf1);               // 57CC80
        vf1=vaddz_xyzw(vf1);              // 57CC84
    }
    // Existing six-candidate caller executes the TWO VRINIT/VRXOR pairs
    // for each path, separately from original EE MT19937 RNG state.
    vf4=owner40;                           // 57CB98 / 57CC90
    vf3=vmulq(vf3,q);                      // 57CB9C / 57CC94
    vf2=vsub(vf2,vf4);                     // 57CBA0 / 57CC98
    vf1=vmul(vf2,vf2);                     // 57CBA4 / 57CC9C
    vf1=vaddy_xyw(vf1);                   // 57CBA8 / 57CCA0
    vf1=vaddz_xyzw(vf1);                  // 57CBAC / 57CCA4
    vf2=vmulq(vf2,q);                      // 57CBB8 / 57CCB0
    // QMFC2 a0=vf2, a1=vf3 feed genuine 0x4748AC XYZ dot.
    // At 0x57CCE0 PCPYUD v0,zero,s1 returns candidate VF[127:64]
    // in EE GPR[127:64]; GPR[63:0] is exactly zero.
    OriginalActorFallbackCandidate_57ca68 result{};
    result.originalScoreVectorA=vf2;
    result.originalScoreVectorB=vf3;
    result.originalPackedTargetLow64=0;
    result.originalPackedTargetHigh64=upper64bits_57ca68(selected);
    return result;
}
} // namespace ae3
