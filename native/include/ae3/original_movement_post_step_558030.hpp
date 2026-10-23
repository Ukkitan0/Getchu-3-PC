#pragma once
#include "ae3/original_movement_child_30c57c.hpp"
#include <cstdint>
#include <optional>

namespace ae3 {
// Source 0x558030..0x558080: retail child slot5 post-movement
// terminal or ordinary-actor transition. Previous original calls
// 0x30C0A0/0x276538 must have supplied originalS5 for the
// under-threshold path. Never infer this from host physics.
struct OriginalMovementPostStep_558030 {
    std::uint32_t originalF21MovementReturnBits;
    std::uint32_t originalF20ResourceMetricBits;
    std::optional<std::uint32_t> originalS5From30C0A0;
};
class OriginalMovementPostStepOwner_558030 {
public:
    virtual ~OriginalMovementPostStepOwner_558030()=default;
    // Source 0x558054 BEQZL delay-slot reads parent+0x1C
    // ONLY when child+B4 is strictly less than 1.0f.
    virtual std::uint32_t read_original_parent_word1c(
        std::uint32_t originalParentGuestVA)=0;
    virtual void call_original_314c44(
        std::uint32_t originalParentWord1C,
        std::uint32_t originalS5From30C0A0,
        std::uint32_t originalChildModeByte09)=0;
    // 0x55806C SWC1 parent+0x70 is the return branch delay slot.
    virtual void store_original_parent_float70(
        std::uint32_t originalParentGuestVA,
        std::uint32_t exactOriginalFloatBits)=0;
};
// Genuine source status 2 after successful parent+0x70 store,
// otherwise source status 0 after original 0x314C44 dispatch.
// Requires exactly representable ordinary COP1 operands for division.
std::uint32_t originalMovementPostStep_558030(
    const OriginalMovementChildD0_30c57c& genuineChild,
    const OriginalMovementPostStep_558030& actualSource,
    OriginalMovementPostStepOwner_558030& originalOwner);
} // namespace ae3
