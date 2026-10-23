#include "ae3/original_manager_followup_57c48c.hpp"
namespace ae3 {
std::uint32_t originalManagerFollowupUpdate_57c48c(
    OriginalManagerChild70& controller,std::uint32_t originalA1,
    OriginalManagerFollowupCalls_57c48c& source) {
    if (!controller.child10) {
        // 0x57C4B8: original source child creation using parent+0x40
        // packed operand and controller mode+0x1D, not host movement.
        void* created=source.call_original_30c520(controller);
        // 0x57C4C0 rereads +0x10 after synchronous construction.
        if (controller.child10)
            source.call_original_child_vtable1_delete(controller.child10);
        controller.child10=created;
    }
    // 0x57C4EC: original pointer-to-child +0x10, a1, 0, 0.
    if(source.call_original_3161b8(controller.child10,originalA1)==0)
        return 0;
    // 0x57C500 branch to 0x57C528, then 0x57C538 LW +0x14.
    if (!controller.alternative30)
        return controller.word14;
    // Four-lane PCEQW+PPACB: equality returns +0x14;
    // non-equality branches to source 0x57C534 ADDIU v0,1 delay.
    if(source.call_original_variant_vector_equal_57c508(controller))
        return controller.word14;
    return 1;
}
} // namespace ae3
