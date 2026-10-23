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
        // Original 0x36FB5C branches to genuine 36F8C8 when
        // source flag bit1 is clear. It must not request the
        // previous VF1 from a game owner on that path.
        const bool flagged=(child.originalWordAt(0xB8)&2u)!=0u;
        const auto genuine=flagged
            ? owner.read_original_flagged_vf1_36fb5c(child,scaledRaw)
            : std::optional<OriginalMovementFlaggedInputs_36fb5c>{};
        if(genuine) {
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
                // Source 36FBE0 -> 36FC18 regenerates VF1 and
                // jumps back to 36FB70 with the remaining f20.
                // This is an opt-in native recovery only when an
                // original owner can publish the live VU0 VF1 state,
                // supply the exact 36F8C8 binding and read the new
                // live VF1 after regeneration. The old owner fallback
                // remains for an unavailable original register owner.
                const auto originalBinding=
                    owner.read_original_regeneration_binding_36f8c8(child);
                if(!originalBinding) {
                    step=owner.call_original_36fb18(child,scaledRaw);
                } else {
                    if(!originalBinding->originalOwner ||
                       originalBinding->genuineChildScratchGuestVA!=
                           genuine->originalChildScratchGuestVA)
                        throw std::invalid_argument(
                            "original 36FC18 source regeneration owner/guest child mismatch");
                    OriginalMovementFlaggedInputs_36fb5c current=*genuine;
                    bool completed=false;
                    // Every large pass increases child+A4 by 1/8,
                    // starting below 1. Source is guaranteed to
                    // terminate within at most eight large steps
                    // if its exact finite COP1 arithmetic completes.
                    for(unsigned iteration=0;iteration<9u;++iteration) {
                        if(current.originalChildScratchGuestVA!=
                               originalBinding->genuineChildScratchGuestVA ||
                           current.originalChildStatusA4Bits!=child.originalWordAt(0xB4) ||
                           current.originalChildFlagsA8!=child.originalWordAt(0xB8) ||
                           (current.originalChildFlagsA8&2u)==0u)
                            throw std::invalid_argument(
                                "original 36FB70 VF1 owner disagrees with regenerated child");
                        const auto actual=originalMovementFlaggedPass_36fb5c(current);
                        owner.publish_original_vf1_after_36fb5c(
                            actual.originalVF1After);
                        if(actual.originalReturnF0Bits) {
                            // Source complete: commits +A4 and clears
                            // +A8 before returning the real f0.
                            originalMovementCommitFlaggedReturn_36fb5c(
                                child,current,actual);
                            step=*actual.originalReturnF0Bits;
                            completed=true;
                            break;
                        }
                        if(!actual.requiresOriginal36F8C8)
                            throw std::logic_error(
                                "original 36FB70 unexpected incomplete non-regenerating step");
                        // Genuine source 36FC14 status increment,
                        // 36FBE4 flag clearing, 36FC18 JAL 36F8C8.
                        originalMovementCommitFlaggedIteration_36fb5c(
                            child,current,actual);
                        (void)originalMovementRegenerateOwned_36f8c8(
                            child,originalBinding->genuineChildScratchGuestVA,
                            *originalBinding->originalOwner);
                        // Source 36FC20 J 36FB70 and 36FC24
                        // DMFC2 VF1. The owner must supply the
                        // newly published original live VF1, not
                        // the previous iteration's register state.
                        const auto next=owner.read_original_flagged_vf1_36fb5c(
                            child,actual.originalRemainingF20Bits);
                        if(!next)
                            throw std::logic_error(
                                "original 36FC24 regenerated VF1 owner unavailable");
                        if(next->originalMovementF12Bits!=
                               actual.originalRemainingF20Bits)
                            throw std::invalid_argument(
                                "original 36FC24 regenerated movement f20 mismatch");
                        current=*next;
                    }
                    if(!completed)
                        throw std::logic_error(
                            "original 36FB70 exceeded source 1/8-step iteration bound");
                }
            }
        } else {
            if(!flagged) {
                // Genuine retail 36FB64 branch to 36FC18 runs
                // actual 36F8C8 before returning to 36FB70.
                // If an authoritative owner provides the raw VU0
                // state and source guest child VA, recover that
                // original regeneration exactly ONCE here.
                const auto binding=
                    owner.read_original_regeneration_binding_36f8c8(child);
                if(binding) {
                    if(!binding->originalOwner)
                        throw std::invalid_argument(
                            "original 36F8C8 binding missing genuine register owner");
                    (void)originalMovementRegenerateOwned_36f8c8(
                        child,binding->genuineChildScratchGuestVA,
                        *binding->originalOwner);
                }
            }
            // Even if VF1 regeneration completed, the following
            // 36FB70/36F8C8 iterative collision handling remains
            // the real original owner, NOT a synthetic result.
            step=owner.call_original_36fb18(child,scaledRaw);
        }
    }
    return {metric,scaledRaw,step};
}
} // namespace ae3
