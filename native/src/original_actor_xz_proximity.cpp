#include "ae3/original_actor_xz_proximity.hpp"
#include <bit>
#include <cmath>
#include <cstring>
namespace ae3 {
std::array<std::byte,16> originalActorPack21B854Return_57c07c(
    const std::array<std::byte,16>& originalResultV0) noexcept {
    std::array<std::byte,16> saved{};
    // PCPYUD s3,zero,v0: low 64 bits cleared, high 64 preserved.
    std::memcpy(saved.data()+8,originalResultV0.data()+8,8);
    return saved;
}
OriginalActorXZProximity_57c080 originalActorXZProximity_57c080(
    const std::array<std::byte,16>& s3,
    const std::array<std::byte,16>& s4) noexcept {
    OriginalVU0Vec4 a{},b{};
    // 0x57C080 DMTC2 s3,VF1, 0x57C088 DMTC2 s4,VF2.
    std::memcpy(&a,s3.data(),16);
    std::memcpy(&b,s4.data(),16);
    // 0x57C090 VSUB.xyzw VF1,VF1,VF2.
    OriginalActorXZProximity_57c080 result{};
    result.originalDifferenceVF1={a.x-b.x,a.y-b.y,a.z-b.z,a.w-b.w};
    const auto& d=result.originalDifferenceVF1;
    // 0x57C094 QMFC2 t7,VF1; 0x57C098 PEXTLW t6,zero,t7;
    // 0x57C09C PCPYUD t7,t7,t7; 0x57C0A0 PCPYLD t7,t7,t6;
    // 0x57C0A4 QMTC2 t7,VF3. Register result is exactly
    // VF3 = {original difference X, +0, difference Z, difference W}.
    // Preserve the original raw 32-bit X/Z/W lane bits.
    result.originalPackedVF3={d.x,0.0f,d.z,d.w};
    const auto& v=result.originalPackedVF3;
    // 0x57C0A8 VMUL.xyzw VF1,VF3,VF3.
    const float x2=v.x*v.x;
    const float y2=v.y*v.y;
    const float z2=v.z*v.z;
    // 0x57C0B8 VADDy.xyw VF1, VF1, VF1;
    // 0x57C0BC VADDz.xyzw VF1, VF1, VF1.
    result.originalSquaredDistanceF0=(x2+y2)+z2;
    // 0x57C0D4 SQRT.S, 0x57C0D8 C.OLT.S f0,f1;
    // original 0x708A44 = exactly float32 2.0f.
    result.originalDistanceF0=std::sqrt(result.originalSquaredDistanceF0);
    constexpr float originalThreshold=std::bit_cast<float>(std::uint32_t{0x40000000u});
    result.originalWithin2Units=result.originalDistanceF0<originalThreshold;
    return result;
}
} // namespace ae3
