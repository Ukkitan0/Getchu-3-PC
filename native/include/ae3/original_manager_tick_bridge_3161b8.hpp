#pragma once
#include "ae3/original_manager_update_316174.hpp"
#include <cstdint>

namespace ae3 {

// Adapter for source controller 0x57BF5C/0x57C27C/0x57C388/
// 0x57C48C/0x57C554 common 0x3161B8/0x316174 paths.
// OriginalManagerChild70 is the SAME owned typed controller used by
// the native actor dispatcher. This adapter owns neither controller
// nor child. Both must outlive it; the depth belongs to the game.
class OriginalManagerTickBridge_3161b8 final {
public:
    OriginalManagerTickBridge_3161b8(
        OriginalManagerChild70& originalController,
        OriginalUpdateDepth_696c98& originalGlobalDepth,
        OriginalManagerUpdateOwner_316174& genuineOwner) noexcept;

    // Called with the real controller's +0x10 lvalue. Refuses
    // unrelated child slots instead of writing another controller.
    std::uint32_t call_original_3161b8(
        void*& originalChild10,std::uint32_t originalA1);

    // Source 0x316174 on the real CONTROLLER after the method pair
    // at +0x08/+0x0C has already been installed.
    std::uint32_t call_original_316174(
        OriginalManagerChild70& originalController,
        std::uint32_t originalA1);

    // Original child vtable slot 1, reused by source direct
    // child-replacement sites in all five controller methods.
    void call_original_child_vtable1_delete(void* originalChild);

private:
    OriginalManagerChild70& controller_;
    OriginalUpdateDepth_696c98& globalDepth_;
    OriginalManagerUpdateOwner_316174& source_;
};
} // namespace ae3
