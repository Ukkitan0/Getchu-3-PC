#include "ae3/original_manager_route_child_57c27c.hpp"

namespace ae3 {
std::uint32_t originalManagerRouteChildUpdate_57c27c(
    OriginalManagerChild70& controller,
    std::uint32_t originalA1,
    OriginalManagerRouteChildCalls_57c27c& source) {
    // 0x57C29C: when a prior source child exists, skip 0x3176AC
    // and 0x30C1D4 altogether. Do not synthesize a replacement.
    if (!controller.child10) {
        if (source.call_original_3176ac(controller.parent04) == 0) {
            // Retail original pair at 0x708A48 is {0x57C388,0}.
            // The false readiness branch jumps directly to 0x57C338;
            // it must NOT call 0x3161B8 before publishing the method.
            controller.word08=0x57c388;
            controller.word0c=0;
            return source.call_original_316174(controller,originalA1);
        }
        void* created=source.call_original_30c1d4(
            controller.parent04,originalA1);
        // 0x57C2CC reloads +0x10 after source construction. Creation
        // can publish an existing child synchronously; original deletes
        // that old child (vtable slot 1) before writing created pointer.
        if (controller.child10)
            source.call_original_child_vtable1_delete(controller.child10);
        controller.child10=created;
    }
    // 0x57C2F0..0x57C314: original updater controls the actual pointer
    // at +0x10. A zero response returns immediately without publishing
    // the next method pair or calling 0x316174.
    if (source.call_original_3161b8(controller.child10,originalA1)==0)
        return 0;
    controller.word08=0x57c388;
    controller.word0c=0;
    // Actual original continuation, never a synthetic success value.
    return source.call_original_316174(controller,originalA1);
}
} // namespace ae3
