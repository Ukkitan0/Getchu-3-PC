#include "ae3/original_manager_movement_57bf5c.hpp"
#include <bit>

namespace ae3 {
namespace {
// Exact IEEE-754 bits of original constant at USA VA 0x708A40.
constexpr float originalDirectionCutoff=
    std::bit_cast<float>(std::uint32_t{0xBF34FDF4});
// Original +0x708A44 is 2.0f. Use strictly less-than as in
// retail C.OLT.S; NaNs never satisfy either original comparison.
constexpr float originalMetricCutoff=2.0f;
void installOriginalNextMethod(
    OriginalManagerChild70& controller,std::uint32_t originalVA) {
    controller.word08=originalVA;
    controller.word0c=0;
}
}
std::uint32_t originalManagerMovementUpdate_57bf5c(
    OriginalManagerChild70& controller,std::uint32_t originalA1,
    OriginalManagerMovementCalls_57bf5c& source) {
    // 0x57BF98 skips the entire VU0/source construction region if
    // controller child+0x10 already exists. Preserve the old child.
    if (!controller.child10) {
        // Original 0x57BFAC..0x57C008: 128-bit VU0 normalisation,
        // DMFC2/MTC1 then C.OLT.S against 0x708A40 (-0.707f).
        // VU0 calculation must be genuine source owner functionality.
        const float direction=source.call_original_vu0_direction_57bfac(
            controller);
        if (direction<originalDirectionCutoff) {
            // Source 0x57C010..0x57C034 -> 0x708A30.
            installOriginalNextMethod(controller,0x57C27C);
            return source.call_original_316174(controller,originalA1);
        }
        if (controller.flag1c)
            source.call_original_21b854(controller);
        // Source 0x57C090..0x57C0DC: VU0 distance, SQRT.S,
        // C.OLT.S f0,2.0f and branch-likely result.
        const float metric=source.call_original_vu0_metric_57c090(
            controller);
        if (metric<originalMetricCutoff) {
            // Source 0x57C0F0..0x57C118 -> 0x708A30.
            installOriginalNextMethod(controller,0x57C27C);
            return source.call_original_316174(controller,originalA1);
        }
        // 0x57C120..0x57C15C; preserve the exact two separate
        // 0x441F40 initializer calls before original 0x36F5F4 and
        // original 0x30C57C child constructor.
        OriginalMovementScratch_57bf5c scratch{};
        source.call_original_441f40(scratch.sourceStack20);
        source.call_original_441f40(scratch.sourceStack60);
        source.call_original_36f5f4(scratch);
        void* created=source.call_original_30c57c(controller,scratch);
        // Retail 0x57C164 rereads child+0x10 after construction;
        // the original virtual slot-1 destructor runs before store.
        if (controller.child10)
            source.call_original_child_vtable1_delete(controller.child10);
        controller.child10=created;
    }

    // 0x57C184..0x57C198: &child+0x10, original a1, zero, zero.
    if (source.call_original_3161b8(controller.child10,originalA1)==0) {
        // 0x57C220..0x57C270: game-owned 0x21DB64 is gated by
        // TWO real original globals. Preserve short-circuit order.
        if (source.read_original_6354e7()!=0 &&
            source.read_original_6354e0()==0)
            source.call_original_21db64(controller);
        return 0;
    }

    // 0x57C1A0..0x57C214 original controller state split.
    if (controller.word14!=2) {
        installOriginalNextMethod(controller,0x57C27C);
        return source.call_original_316174(controller,originalA1);
    }
    if (controller.flag1c) {
        // Original 0x57C1B8..0x57C1CC -> 0x708A38.
        installOriginalNextMethod(controller,0x57C48C);
        return source.call_original_316174(controller,originalA1);
    }
    // Source 0x57C1D4..0x57C210 variant-zero returns 2.
    // Variant-one PCEQW all-four-lanes equality also returns 2,
    // inequality returns 1 via MOVZ after the original branch-likely.
    if (!controller.alternative30)
        return 2;
    return source.call_original_vectors_equal_57c1e0(controller)?2u:1u;
}
} // namespace ae3
