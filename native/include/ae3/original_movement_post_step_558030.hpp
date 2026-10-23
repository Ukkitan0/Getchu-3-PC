#pragma once
#include "ae3/original_movement_child_30c57c.hpp"
#include <cstdint>
#include <optional>

namespace ae3 { class OriginalMovementFlag0Owner_36f83c; }


namespace ae3 {
// Source 0x558030..0x558080: retail child slot5 post-movement
// terminal or ordinary-actor transition. Previous original calls
// 0x30C0A0/0x276538 must have supplied originalS5 for the
// under-threshold path. Never infer this from host physics.
struct OriginalMovementPostStep_558030 {
    std::uint32_t originalF21MovementReturnBits;
    std::uint32_t originalF20ResourceMetricBits;
    // 557FEC DADDU s5,v0,zero: preserve the full 64-bit EE value.
    std::optional<std::uint64_t> originalS5From30C0A0;
};
class OriginalMovementPostStepOwner_558030 {
public:
    virtual ~OriginalMovementPostStepOwner_558030()=default;
    // Source 0x558054 BEQZL delay-slot reads parent+0x1C
    // ONLY when child+B4 is strictly less than 1.0f.
    virtual std::uint32_t read_original_parent_word1c(
        std::uint32_t originalParentGuestVA)=0;
    // Original 558054 LW a0,parent+0x1C sign-extends to
    // 64-bit EE a0 even though the source memory word is 32-bit.
    virtual void call_original_314c44(
        std::uint64_t originalParentWord1CSignExtended,
        std::uint64_t originalS5From30C0A0,
        std::uint32_t originalChildModeByte09)=0;
    // 0x55806C SWC1 parent+0x70 is the return branch delay slot.
    virtual void store_original_parent_float70(
        std::uint32_t originalParentGuestVA,
        std::uint32_t exactOriginalFloatBits)=0;
};
// Source 0x557FC8 checks child+B8 bit 0. When clear, its
// 0x558084 JAL must regenerate the original VU0 state through
// 0x36F83C before the remaining actor processing. Do not
// fabricate the source guest stack address or VU registers.
struct OriginalMovementFlag0Binding_36f83c {
    std::uint64_t genuineChildScratchGuestVA;
    OriginalMovementFlag0Owner_36f83c* originalOwner;
};

// A caller may continue to 0x558030 ONLY after authentic upstream
// 0x557FC8..0x55802C actor/collision processing has completed.
// This is an explicit original-game state boundary, NOT permission
// to skip unresolved movement or fabricate a source S5 value.
class OriginalMovementPostStepContinuation_558030:
    public OriginalMovementPostStepOwner_558030 {
public:
    // Optional source-backed 558084 -> 36F83C regeneration owner.
    // If bit0 is clear at 557FC8 and there is no valid binding,
    // the typed lifecycle must refuse rather than jump to 558030.
    virtual std::optional<OriginalMovementFlag0Binding_36f83c>
    read_original_flag0_binding_36f83c(
        const OriginalMovementChildD0_30c57c&) {return std::nullopt;}

    // Return nullopt if the actual upstream original game owner
    // cannot establish the source registers and branch provenance.
    // All returned data must reflect the real 0x558030 entry state.
    virtual std::optional<OriginalMovementPostStep_558030>
    read_original_post_step_after_557fc8(
        const OriginalMovementChildD0_30c57c& genuineChild)=0;
};

// Genuine source status 2 after successful parent+0x70 store,
// otherwise source status 0 after original 0x314C44 dispatch.
// Requires exactly representable ordinary COP1 operands for division.
std::uint32_t originalMovementPostStep_558030(
    const OriginalMovementChildD0_30c57c& genuineChild,
    const OriginalMovementPostStep_558030& actualSource,
    OriginalMovementPostStepOwner_558030& originalOwner);
} // namespace ae3
