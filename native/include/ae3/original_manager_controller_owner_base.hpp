#pragma once
#include "ae3/original_manager_native_method_router.hpp"
#include "ae3/original_manager_tick_bridge_3161b8.hpp"

namespace ae3 {

// One real host-side owner for all recovered original manager methods,
// replacing five disconnected 0x3161B8/0x316174 source callbacks.
// The actual gameplay methods stay pure virtual in the inherited
// original source interfaces. This creates NO fake Player/Monkey,
// movement input or child allocator.
class OriginalManagerControllerOwnerBase :
    public OriginalManagerMovementCalls_57bf5c,
    public OriginalManagerRouteChildCalls_57c27c,
    public OriginalManagerRoutedCalls_57c388,
    public OriginalManagerFollowupCalls_57c48c,
    public OriginalManagerType2Calls_57c554 {
public:
    OriginalManagerControllerOwnerBase(
        OriginalManagerChild70& originalController,
        OriginalUpdateDepth_696c98& sharedSourceGlobalDepth,
        OriginalManagerUpdateOwner_316174& realVtableOwner) noexcept;
    ~OriginalManagerControllerOwnerBase() override = default;

    // Final: all original manager update actions use the same actual
    // 0x3161B8 function with a2=a3=0 and original field+0x10,
    // so source status+0x14 cannot diverge between action types.
    std::uint32_t call_original_3161b8(
        void*& originalChild10,std::uint32_t originalA1) final;
    // Original controller method-pair continuation via the SAME
    // game-wide nesting counter 0x696C98 and vtable[5] owner.
    std::uint32_t call_original_316174(
        OriginalManagerChild70& originalController,
        std::uint32_t originalA1) final;
    // Same actual child deleting destructor across all methods.
    void call_original_child_vtable1_delete(void* originalChild) final;

    // Concrete source controller-method router, with no owner-provided
    // override of the translated 0x3161B8/0x316174 entrypoints.
    OriginalManagerNativeMethodOwner nativeMethodOwner(
        OriginalManagerDispatchCalls_57c620& realDispatchOwner) noexcept;

private:
    OriginalManagerTickBridge_3161b8 tick_;
};

} // namespace ae3
