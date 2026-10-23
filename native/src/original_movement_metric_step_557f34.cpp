#include "ae3/original_movement_metric_step_557f34.hpp"
#include <bit>
#include <cmath>
#include <stdexcept>

namespace ae3 {
OriginalMovementMetricStepResult_557f34 originalMovementMetricStep_557f34(
    OriginalMovementChildD0_30c57c& child,
    OriginalMovementMetricOwner_557f34& owner) {
    // Reached only after source 0x557ED8 setup writes child+C0=1.
    const auto initialized=child.originalByteAt(0xC0);
    const auto modeByte=child.originalByteAt(9);
    if(!initialized || *initialized==std::byte{0} || !modeByte)
        throw std::logic_error("original 557F34 must follow real normal child setup");
    const std::uint32_t parentVA=child.originalWordAt(4);

    // 0x557F30 LBU mode, 0x557F38 JAL real source 317B34.
    const auto resource=owner.read_original_resource_float_fields(parentVA);
    const auto mode=std::to_integer<std::uint32_t>(*modeByte);
    std::uint32_t metric{};
    if(resource.originalParentWord40==1u || mode==0u)
        metric=originalManagerResourceFloatBits_317b34(resource,mode);
    else {
        const auto input=owner.read_original_resource_vu0_inputs(parentVA);
        if(input.originalParentGuestVA!=parentVA)
            throw std::invalid_argument(
                "original 317B34 VU0 VF1 input belongs to a different retail parent");
        const auto original=originalManagerResourceVU0_317b34(
            resource,mode,input);
        // Real original 317B34 mutates VF1 in VU0 BEFORE
        // returning COP1 f0. A source-owner without VF1
        // publication cannot claim correct frame continuity.
        owner.publish_original_vf1_after_317b34(
            parentVA,original.originalVF1After);
        metric=original.originalF0Raw;
    }

    // Original 0x557F40 LWC1 f12,parent+0x70, 0x557F48
    // MOV.S f20,f0. Original 0x557F4C JAL 0x36FB18 has
    // 0x557F50 MUL.S f12=f0*f12 in its delay slot.
    const auto scaleRaw=owner.read_original_parent_float70(parentVA);
    const float f0=std::bit_cast<float>(metric);
    const float scale=std::bit_cast<float>(scaleRaw);
    if(!std::isfinite(f0)||!std::isfinite(scale) ||
       std::fpclassify(f0)==FP_SUBNORMAL ||
       std::fpclassify(scale)==FP_SUBNORMAL)
        throw std::invalid_argument("original 557F50 requires PS2 COP1 special mode");
    const volatile float scaled=f0*scale;
    if(!std::isfinite(scaled) || std::fpclassify(scaled)==FP_SUBNORMAL ||
       static_cast<double>(scaled)!=
           static_cast<double>(f0)*static_cast<double>(scale))
        throw std::domain_error("original 557F50 MUL.S requires PS2 rounding");
    const auto scaledRaw=std::bit_cast<std::uint32_t>(float(scaled));
    // Original 36FB18 fast return may be computed without its
    // movement/collision engine. Otherwise mandatory source owner.
    const auto fast=originalMovementStepFast_36fb18(child,scaledRaw);
    std::uint32_t step{};
    if(fast) {
        step=*fast;
    } else {
        const auto genuine=owner.read_original_flagged_vf1_36fb5c(
            child,scaledRaw);
        if(genuine && child.originalWordAt(0xB8)&2u) {
            // Verify the source original 36FB5C arguments are
            // THIS owned child's actual status/flags/scale.
            if(genuine->originalChildStatusA4Bits!=child.originalWordAt(0xB4) ||
               genuine->originalChildFlagsA8!=child.originalWordAt(0xB8) ||
               genuine->originalMovementF12Bits!=scaledRaw)
                throw std::invalid_argument(
                    "source 36FB5C owner supplied mismatched original child/scale");
            const auto plan=originalMovementFlaggedPass_36fb5c(*genuine);
            if(plan.originalReturnF0Bits) {
                // 36FB70 VU0 VF1 mutation must be published BEFORE
                // retail 36FBBC/36FBEC child status writes.
                owner.publish_original_vf1_after_36fb5c(
                    plan.originalVF1After);
                originalMovementCommitFlaggedReturn_36fb5c(
                    child,*genuine,plan);
                step=*plan.originalReturnF0Bits;
            } else {
                // This first pass must call genuine 36F8C8 to
                // regenerate VF1. We have NOT mutated child or VF1
                // through our partial plan: delegate full retail
                // 36FB18 instead of duplicating first-pass effects.
                step=owner.call_original_36fb18(child,scaledRaw);
            }
        } else {
            step=owner.call_original_36fb18(child,scaledRaw);
        }
    }
    return {metric,scaledRaw,step};
}
} // namespace ae3
