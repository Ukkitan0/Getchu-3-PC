#include "ae3/original_manager_update_316174.hpp"
#include <stdexcept>

namespace ae3 {
std::uint32_t originalManagerGuardedUpdate_316174(
    void* child,std::uint32_t originalA1,
    OriginalUpdateDepth_696c98& state,
    OriginalManagerUpdateOwner_316174& source) {
    if (!child)
        throw std::invalid_argument(
            "original 0x316174 requires a genuine source object/vtable");
    // Retail 0x316188..0x316198: source global 0x696C98 ++
    // in the JALR delay slot before the virtual slot5 method.
    ++state.depth;
    std::uint32_t result=0;
    try {
        result=source.call_original_vtable5(child,originalA1);
    } catch (...) {
        // The retail game does not use C++ exceptions. A native
        // missing owner method must refuse without poisoning the
        // shared original nesting state.
        --state.depth;
        throw;
    }
    // Retail 0x31619C..0x3161A8 reloads and decrements the
    // current original global. Preserve the original v0 result.
    --state.depth;
    return result;
}
std::uint32_t originalManagerChildUpdate_3161b8(
    OriginalManagerChild70& controller,
    std::uint32_t originalA1,std::uint32_t a2,std::uint32_t a3,
    OriginalUpdateDepth_696c98& state,
    OriginalManagerUpdateOwner_316174& source) {
    // Original ANDI and MOVN only inspect low 8 bits of a2/a3.
    if ((a2&0xFFu)!=0u) originalA1|=1u;
    if ((a3&0xFFu)!=0u) originalA1|=2u;
    // Source 0x3161DC reads child+0x10 before JAL 0x316174.
    const auto originalChild=controller.child10;
    const auto result=originalManagerGuardedUpdate_316174(
        originalChild,originalA1,state,source);
    // Source 0x3161E8 stores EXACT v0 to field +0x14 even when
    // that value is not boolean (e.g. status 2).
    controller.word14=result;
    if (result==0) return 0;
    // Source 0x3161F4 rereads child+0x10 AFTER callback;
    // nested source code may have replaced it.
    if (controller.child10)
        source.call_original_child_vtable1_delete(controller.child10);
    controller.child10=nullptr;
    return 1;
}
} // namespace ae3
