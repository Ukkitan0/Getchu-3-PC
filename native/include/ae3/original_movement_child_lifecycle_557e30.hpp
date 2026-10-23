#pragma once
#include "ae3/original_manager_update_316174.hpp"
#include "ae3/original_movement_child_30c57c.hpp"
#include <cstdint>
#include <memory>

namespace ae3 {
class OriginalMovementNormalCalls_557ed8;
// Genuine original 0x557E30 child slot5 must read parent+0x14
// in the delay slot of 0x557E64, EVEN when it returns immediately
// for controller update bits 0/1. This source parent is another
// original game object; its 32-bit guest VA is never a host pointer.
class OriginalMovementParentRead_557e30 {
public:
    virtual ~OriginalMovementParentRead_557e30()=default;
    virtual std::uint32_t read_original_parent_word14(
        std::uint32_t originalParentGuestVA)=0;
};

// Native owner for the source VERIFIED FLAG-DRIVEN completion of
// retail child object vtable 0x6B0B20: slot5 0x557E30,
// virtual deleting destructor slot1 0x556A0C.
// This does NOT invent the remainder of slot5 actor gameplay.
// Ordinary a1 without source flags is deliberately refused.
class OriginalMovementChildLifecycle_557e30 final:
    public OriginalManagerUpdateOwner_316174 {
public:
    OriginalMovementChildLifecycle_557e30(
        std::unique_ptr<OriginalMovementChildD0_30c57c> originalChild,
        OriginalMovementParentRead_557e30& genuineParent,
        OriginalMovementNormalCalls_557ed8* optionalGenuineNormalOwner=nullptr);
    void* originalChildPointer() const noexcept;
    bool ownsOriginalChild() const noexcept;
    std::uint32_t call_original_vtable5(
        void* originalObject,std::uint32_t originalA1) override;
    void call_original_child_vtable1_delete(
        void* originalObject) override;
private:
    std::unique_ptr<OriginalMovementChildD0_30c57c> child_;
    OriginalMovementParentRead_557e30& parent_;
    // Nonterminal normal setup is activated ONLY by a genuine owner.
    // The original 0x317B34 / 0x36FB18 gameplay continuation
    // remains unsupported and must not get a synthetic result.
    OriginalMovementNormalCalls_557ed8* normalOwner_;
};
} // namespace ae3
