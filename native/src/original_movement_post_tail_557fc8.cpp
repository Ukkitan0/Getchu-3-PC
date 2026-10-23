#include "ae3/original_movement_post_tail_557fc8.hpp"
#include <stdexcept>

namespace ae3 {
std::uint32_t originalMovementPostTail_557fc8(
    OriginalMovementChildD0_30c57c& child,
    std::uint64_t scratchGuestVA,
    std::uint32_t originalParentWord14,
    const OriginalMovementPostVector_557f54& postVector,
    const OriginalMovementPostStep_558030& movementReturn,
    OriginalMovementPostTailOwner_557fc8& owner) {
    if(postVector.originalSlot5EarlyStatus)
        throw std::invalid_argument(
            "original 557FC0 slot5 early exit already returned before 557FC8");
    const auto parentGuestVA=child.originalWordAt(4);
    // Source 557FC8 LW child+B8; 557FD4 BEQZ bit0 ->
    // 558084 JAL genuine 36F83C. Its v0 contains ALL VF1
    // lanes, and source 558090 MMI3 POR a1,zero,v0
    // copies the complete original 128-bit return register.
    std::array<std::uint32_t,4> originalEEA1{};
    if((child.originalWordAt(0xB8)&1u)==0u) {
        const auto generated=originalMovementRegenerateFlag0Owned_36f83c(
            child,scratchGuestVA,owner);
        originalEEA1=generated.originalReturnV0;
    } else {
        // Source 557FDC LQ child+0x90 -> EE v0;
        // source 557FE0 MMI3 POR a1,zero,v0.
        for(std::size_t i=0;i<4;++i)
            originalEEA1[i]=child.originalWordAt(0x90+4*i);
    }
    // Source 557FE4 JAL original 30C0A0(a0=parent,a1=128-bit)
    // and 557FEC copies original v0 to persistent GPR s5.
    const auto originalS5=owner.call_original_30c0a0(
        parentGuestVA,originalEEA1);
    // Source 557FF4 JAL original 276538(a0=parent+14).
    // 557FF8 original MMI3 POR in the JAL delay slot
    // copies entire 128-bit s3=557F8C DMFC2 VF2 to EE a1.
    owner.call_original_276538(
        originalParentWord14,postVector.originalGprS3VF2);
    // Source BSS debug byte 0x6354E0 is read ONLY if
    // 0x6354E7 is nonzero; if E0 equals zero call
    // genuine 0x36FC28(child+0x10) once before post-step.
    if(owner.read_original_6354e7()!=0u &&
       owner.read_original_6354e0()==0u)
        owner.call_original_36fc28(scratchGuestVA);
    // Genuine original 558030..558080 handles 1.0 progress
    // threshold, parent+0x70 ratio, or ongoing 314C44.
    return originalMovementPostStep_558030(
        child,{movementReturn.originalF21MovementReturnBits,
               movementReturn.originalF20ResourceMetricBits,originalS5},
        owner);
}
} // namespace ae3
