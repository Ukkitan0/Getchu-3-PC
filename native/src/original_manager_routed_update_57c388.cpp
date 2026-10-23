#include "ae3/original_manager_routed_update_57c388.hpp"
namespace ae3 {
std::uint32_t originalManagerRoutedUpdate_57c388(
    OriginalManagerChild70& controller,std::uint32_t a1,
    OriginalManagerRoutedCalls_57c388& source) {
    if(!controller.child10) {
        // Original 0x57C3BC/C4 captures parent+0x14 vector at
        // +0x40 before calling source actor route control.
        const auto originalOwnerVector=
            source.read_original_parent14_vector40(controller);
        if(controller.flag1c)
            source.call_original_57c7cc(controller,a1);
        // Original 0x57C3D8..0x57C3E8 VU0 operands include genuine
        // controller+0x40. Owner-specific VU transform stays with
        // the real 0x30C21C creation callback; do not invent it.
        void* created=source.call_original_30c21c(
            controller,originalOwnerVector);
        // Source rereads child+0x10 after the creation callback.
        if(controller.child10)
            source.call_original_child_vtable1_delete(controller.child10);
        controller.child10=created;
    }
    // Source 0x57C424: original 3161B8(&child+0x10,a1,0,0).
    if(source.call_original_3161b8(controller.child10,a1)==0)
        return 0;
    // Retail 0x708A38 contains exact pair {0x57C48C,0},
    // installed before forwarding original 0x316174 result.
    controller.word08=0x57c48c;
    controller.word0c=0;
    return source.call_original_316174(controller,a1);
}
} // namespace ae3
