#pragma once
#include "ae3/original_movement_flag0_regenerate_36f83c.hpp"
#include "ae3/original_movement_post_vector_557f54.hpp"
#include "ae3/original_movement_post_step_558030.hpp"
#include <array>
#include <cstdint>

namespace ae3 {
// Original USA 0x557FC8..0x558030 source control flow.
// External 30C0A0, 276538, optional 36FC28 and 314C44
// are the genuine original game-owner methods, not substitutes.
class OriginalMovementPostTailOwner_557fc8:
    public OriginalMovementFlag0Owner_36f83c,
    public OriginalMovementPostStepOwner_558030 {
public:
    virtual ~OriginalMovementPostTailOwner_557fc8()=default;
    // The real EE a0 for 36F83C/36FC28 is child+0x10 as
    // a GUEST VA. Never derive this from the native host
    // object pointer; the live original game owner supplies it.
    virtual std::uint64_t read_original_child_scratch_guest_va_557fc8(
        const OriginalMovementChildD0_30c57c& originalChild)=0;
    // Source 557FE4 JAL: original EE a0=parent and complete
    // 128-bit EE a1 from MMI3 POR original child+0x90 or VF1.
    // 557FEC DADDU s5,v0,zero consumes v0's full low 64 bits.
    // Do not narrow original return state to a 32-bit guest VA.
    virtual std::uint64_t call_original_30c0a0(
        std::uint32_t originalParentGuestVA,
        const std::array<std::uint32_t,4>& originalEEA1)=0;
    // Source 557FF4 JAL: original a0=parent+0x14, a1=EE s3.
    // 557FF8 MMI3 POR a1,zero,s3 copies ALL four VF2 lanes.
    virtual void call_original_276538(
        std::uint32_t originalParentWord14,
        const std::array<std::uint32_t,4>& originalEES3)=0;
    virtual std::uint8_t read_original_6354e7()=0;
    virtual std::uint8_t read_original_6354e0()=0;
    virtual void call_original_36fc28(
        std::uint64_t originalChildScratchGuestVA)=0;
};
// Source flag0 selection, exact packed-register ABI, actual
// original manager callback ordering, optional debug gate,
// followed by native original 558030 terminal/ongoing branch.
// Original 557F54 VF1/VF2 must already be game-owner published.
std::uint32_t originalMovementPostTail_557fc8(
    OriginalMovementChildD0_30c57c& actualChild,
    std::uint64_t originalChildScratchGuestVA,
    std::uint32_t originalParentWord14,
    const OriginalMovementPostVector_557f54& originalPostVector,
    const OriginalMovementPostStep_558030& originalMovementReturn,
    OriginalMovementPostTailOwner_557fc8& genuineOwner);
} // namespace ae3
