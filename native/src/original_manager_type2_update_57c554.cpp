#include "ae3/original_manager_type2_update_57c554.hpp"

namespace ae3 {
std::uint32_t originalManagerType2Update_57c554(
    OriginalManagerChild70& controller,
    std::uint32_t originalA1,
    OriginalManagerType2Calls_57c554& source) {
    // 0x57C570..0x57C5A8: only when original child+0x10 is
    // null, call actual 0x30C360 with the game's real parent,
    // controller vector, candidate polygon+0x50 and mode+0x1D.
    if (!controller.child10) {
        // This callback may synchronously populate child+0x10;
        // the original reads that field AGAIN after 0x30C360.
        void* created=source.call_original_30c360(controller);
        if (controller.child10)
            source.call_original_child_vtable1_delete(
                controller.child10);
        controller.child10=created;
    }

    // 0x57C5AC..0x57C5C4: actual source 0x3161B8
    // manages a pointer to child+0x10, using original a1,0,0.
    // Source zero result returns immediately with zero.
    if (source.call_original_3161b8(controller.child10,originalA1)==0)
        return 0;

    // 0x57C5C8..0x57C608: source variant+0x30 zero =>1.
    if (!controller.alternative30)
        return 1;

    // 0x57C5D4..0x57C5FC: EE/VU packed vector equality from
    // original controller+0x40 and +0x20, not an approximation
    // based on host position or Unreal transforms.
    if (source.call_original_variant_vector_equal_57c5d4(controller))
        return 1;

    // Original 0x57C604 returns EXACT controller+0x14 field.
    return controller.word14;
}
} // namespace ae3
