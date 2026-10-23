#include "ae3/original_manager_tick_bridge_3161b8.hpp"
#include <stdexcept>

namespace ae3 {
OriginalManagerTickBridge_3161b8::OriginalManagerTickBridge_3161b8(
    OriginalManagerChild70& controller,
    OriginalUpdateDepth_696c98& depth,
    OriginalManagerUpdateOwner_316174& source) noexcept
    : controller_(controller),globalDepth_(depth),source_(source) {}

std::uint32_t OriginalManagerTickBridge_3161b8::call_original_3161b8(
    void*& child10,std::uint32_t originalA1) {
    if (&child10!=&controller_.child10)
        throw std::invalid_argument(
            "0x3161B8 child+0x10 does not belong to the current original controller");
    // The retail movement/controller paths set a2=a3=0.
    // Reuse the actual translated source wrapper: flags, status
    // publication, destructor and nested original update depth.
    return originalManagerChildUpdate_3161b8(
        controller_,originalA1,0,0,globalDepth_,source_);
}

std::uint32_t OriginalManagerTickBridge_3161b8::call_original_316174(
    OriginalManagerChild70& controller,std::uint32_t originalA1) {
    if (&controller!=&controller_)
        throw std::invalid_argument(
            "0x316174 continuation has a foreign original controller");
    return originalManagerGuardedUpdate_316174(
        &controller_,originalA1,globalDepth_,source_);
}

void OriginalManagerTickBridge_3161b8::
call_original_child_vtable1_delete(void* child) {
    if (!child)
        throw std::invalid_argument(
            "0x3161B8 original deleting destructor called on null");
    source_.call_original_child_vtable1_delete(child);
}
} // namespace ae3
