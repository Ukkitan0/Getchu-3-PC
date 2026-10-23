#pragma once
#include "ae3/original_movement_child_30c57c.hpp"
#include "ae3/original_manager_resource_selector_317b9c.hpp"
#include <cstdint>

namespace ae3 {
// Original SCUS_975.01 0x557ED8..0x557F30, NORMAL zero-flag
// movement child setup following a 0x36F954 terminal result ZERO.
// The external calls are genuine retail managers and MUST be
// implemented by the game owner. No Unreal/host replacements.
class OriginalMovementNormalCalls_557ed8 {
public:
    virtual ~OriginalMovementNormalCalls_557ed8()=default;
    // Actual retail 0x317B9C reads (parent+0x10), (+0x40)
    // and conditionally (+0x34). The owner must obtain these
    // from the same live parent whose source +0x14 was read
    // earlier in 0x557E68.
    virtual OriginalManagerResourceFields_317b9c
    read_original_parent_resource_fields(std::uint32_t guestParentVA)=0;
    // Source 0x557F00: a0=parent+0x14, a1=317B9C result;
    // a2=0, a3=0, f12=0. Preserve return v0 status.
    virtual std::uint32_t call_original_276b68(
        std::uint32_t originalParentWord14,
        std::uint32_t originalResourceVA)=0;
    // Called only if genuine 0x276B68 returned nonzero.
    virtual void call_original_2767f4(std::uint32_t originalParentWord14)=0;
    // Always called: original (a0=parent+0x14, a1=0,a2=0)
    virtual void call_original_2773d4(std::uint32_t originalParentWord14)=0;
};

// Source-authentic preparation up to original 0x557F30 inclusive.
// No fake update return value: subsequent original 0x317B34 and
// 0x36FB18 actor/collision/animation still must be executed.
void originalMovementNormalBegin_557ed8(
    OriginalMovementChildD0_30c57c& originalChild,
    std::uint32_t originalParentWord14,
    const OriginalManagerResourceFields_317b9c& genuineResourceFields,
    OriginalMovementNormalCalls_557ed8& originalOwner);
} // namespace ae3
