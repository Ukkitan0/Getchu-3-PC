#include "ae3/original_actor_target_dispatch.hpp"
#include <bit>

namespace ae3 {
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
            // 0x57C944/0x57C948: LQC2 VF1, controller+0x20,
            // then DMFC2 $v0,VF1. On little-endian retail MIPS EE,
            // $v0 receives exactly the LOW 64 original vector bits.
            std::uint64_t bits=0;
            for(unsigned i=0;i<8;++i)
                bits|=std::uint64_t(std::to_integer<unsigned>(originalControllerVector20[i]))<<(8*i);
            return {OriginalActorTargetBranch_57c8ec::OriginalVariantVectorPassThrough_57c944,bits};
        }
        // 0x57C970..0x57C988: original 0x57CA68 computes VF1.
        const auto lower=original.call_original_57ca68_geometry(target);
        return {OriginalActorTargetBranch_57c8ec::OriginalFallbackGeometry_57ca68,
                lower,original.call_original_57ca68_geometry_high64()};
    }
    // 0x57C98C..0x57C9B8: exactly three original 0x21B458 reads.
    // Source FPU and VU transformations remain external dependencies.
    std::array<std::array<std::byte,16>,3> vertices{};
    for(std::size_t i=0;i<3;++i)vertices[i]=originalSpatialVector_21b458(target,i);
    return {OriginalActorTargetBranch_57c8ec::OriginalThreeVertexTransform_57c98c,
            original.call_original_57c98c_three_vertex_transform(target,vertices)};
}
} // namespace ae3
