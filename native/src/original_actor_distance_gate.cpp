#include "ae3/original_actor_distance_gate.hpp"
#include <bit>
#include <cmath>
#include <cstring>

namespace ae3 {
OriginalActorDistanceGate_57bcdc originalActorDistanceGate_57bcdc(
    bool variant30,
    const std::array<std::byte,16>& controller20,
    const std::array<std::byte,16>& owner14vector40) noexcept {
    // 0x57BCDC LBU controller+0x30, 0x57BCE0 BEQZ skips ALL math.
    if(!variant30)return {std::nullopt,false};
    OriginalVU0Vec4 controller{},owner{};
    static_assert(sizeof(OriginalVU0Vec4)==16);
    // 0x57BCEC LQC2 VF1,0x20(controller); 0x57BCF4 LQC2
    // VF2,0x40(owner->+0x14). Both loads transfer ALL 4 lanes.
    std::memcpy(&controller,controller20.data(),16);
    std::memcpy(&owner,owner14vector40.data(),16);
    // 0x57BCF8 VSUB.xyzw VF1,VF1,VF2 in original order.
    const OriginalVU0Vec4 difference{
        controller.x-owner.x,controller.y-owner.y,
        controller.z-owner.z,controller.w-owner.w};
    // 0x57BD00 original 0x4748AC dot; W deliberately omitted.
    const float squared=originalVU0DotXYZ_4748ac(difference,difference);
    // 0x57BD10 SQRT.S; 0x57BD1C loads original 0x708A20=0x40000000.
    const float length=std::sqrt(squared);
    constexpr float retailThreshold=std::bit_cast<float>(std::uint32_t{0x40000000});
    // 0x57BD20 C.OLT.S and 0x57BD24 BC1T: strict, NaN is false.
    return {length,length<retailThreshold};
}
void originalActorApplyDistanceGate_57bd3c(
    const OriginalActorDistanceGate_57bcdc& gate,
    std::uint32_t& controller08,std::uint32_t& controller0c) noexcept {
    // 0x57BD3C SD zero,0(sp), 0x57BD40/44 LW zero fields;
    // 0x57BD4C SW 0,+0x0C, 0x57BD54 SW 0,+0x08.
    if(gate.originalClearsControllerRoute08And0c) {
        controller08=0;
        controller0c=0;
    }
}
} // namespace ae3
