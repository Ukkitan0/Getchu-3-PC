#include "ae3/original_actor_target_dispatch.hpp"
#include <bit>

namespace ae3 {
OriginalVU0Vec4 originalActorTargetProjectionOperand_57c894(
    const OriginalActorTargetResult_57c8ec& result) noexcept {
    // 0x57C894: PCPYUD a2,zero,v0.
    // EE a2[63:0]=0, a2[127:64]=v0[127:64].
    const auto hi=result.originalVectorHigh64;
    return {0.0f,0.0f,
            std::bit_cast<float>(std::uint32_t(hi)),
            std::bit_cast<float>(std::uint32_t(hi>>32))};
}
OriginalActorTargetResult_57c8ec originalActorTargetVector_57c8ec(
    const OriginalSpatialRecord70& target,
    bool variant30,
    const std::array<std::byte,16>& originalControllerVector20,
    OriginalActorTargetGeometryCalls_57c8ec& original) {
    // 0x57C918..0x57C920: LBU original controller +0x30.
    if(variant30) {
        // 0x57C924..0x57C93C: F12 is original global float 0x708A60,
        // whose raw bits are 0x3DCCCCCD (near 0.1).
        constexpr float originalF12=std::bit_cast<float>(std::uint32_t(0x3dcccccd));
        if(original.call_original_21b788_eligibility(target,originalF12)) {
            // 0x57C944 LQC2 VF1, +0x20; 0x57C948 QMFC2 v0,VF1
            // transfers BOTH doublewords. Later 0x57C894 PCPYUD
            // selects the upper doubleword for the projection ray.
            std::uint64_t lo=0,hi=0;
            for(unsigned i=0;i<8;++i) {
                lo|=std::uint64_t(std::to_integer<unsigned>(originalControllerVector20[i]))<<(8*i);
                hi|=std::uint64_t(std::to_integer<unsigned>(originalControllerVector20[i+8]))<<(8*i);
            }
            return {OriginalActorTargetBranch_57c8ec::OriginalVariantVectorPassThrough_57c944,lo,hi};
        }
        // 0x57C970..0x57C988: original 0x57CA68 computes VF1.
        const auto lower=original.call_original_57ca68_geometry(target);
        return {OriginalActorTargetBranch_57c8ec::OriginalFallbackGeometry_57ca68,
                lower,original.call_original_57ca68_geometry_high64()};
    }
    // 0x57C98C..0x57C9B8: exactly three original 0x21B458 reads.
    // Source PCPYUD/VMULx/VADD target is original VF1[127:0].
    std::array<std::array<std::byte,16>,3> vertices{};
    for(std::size_t i=0;i<3;++i)vertices[i]=originalSpatialVector_21b458(target,i);
    const auto lo=original.call_original_57c98c_three_vertex_transform(target,vertices);
    return {OriginalActorTargetBranch_57c8ec::OriginalThreeVertexTransform_57c98c,
            lo, original.call_original_57c98c_geometry_high64()};
}
} // namespace ae3
