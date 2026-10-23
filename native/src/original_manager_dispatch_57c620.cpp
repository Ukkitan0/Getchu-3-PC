#include "ae3/original_manager_dispatch_57c620.hpp"
#include <bit>
#include <cstdint>

namespace ae3 {
namespace {
std::int32_t originalSraOne(std::uint32_t word) noexcept {
    // R5900 SRA by one, including sign-extension on original bit31.
    return std::bit_cast<std::int32_t>(
        (word >> 1) | (word & 0x80000000u));
}
}

std::uint32_t originalManagerDispatch_57c620(
    OriginalManagerChild70& controller,
    std::uint32_t originalA1,
    OriginalManagerDispatchCalls_57c620& calls) {
    // 0x57C648..0x57C67C: the source compares +8/+C against
    // retail 0x708A50..54 (both source-pinned zero words).
    // On any +8==0 path the original returns exactly 2.
    if (controller.word08==0)
        return 2;

    std::uint32_t dispatchResult=1;
    // 0x57C680: a non-null child+0x10 forces method dispatch.
    // Otherwise 0x57C68C..A8 skip dispatch when a1 has either
    // bit 0 or bit 1 set, leaving $s3=1.
    if (controller.child10 ||
        (originalA1 & 3u)==0u) {
        const auto originalThisOffset=
            originalSraOne(controller.word0c);
        if ((controller.word0c & 1u)!=0u) {
            // 0x57C77C..0x57C7B8: get source vtable owner from
            // (original controller + signed (word0c>>1)); source
            // slot offset = (word08>>2)<<2, then JALR resolved
            // function. The host original actor owner resolves it.
            dispatchResult=calls.call_original_virtual_57c79c(
                controller.word08 & ~3u,originalThisOffset);
        } else {
            // 0x57C7C4..0x57C7C8/0x57C7AC..B8: direct JALR
            // original controller+8 method word with the same
            // source subobject offset. Never call that VA as a
            // Windows native pointer.
            dispatchResult=calls.call_original_direct_57c7ac(
                controller.word08,originalThisOffset);
        }
    }

    // 0x57C6AC..0x57C6CC: update original actor polygon through 0x30FFD0 only when both the
    // source candidate exists AND $s3 (source dispatch result)
    // is nonzero. Original parent+0x04 is the first argument.
    if (controller.originalPendingPolygon50 && dispatchResult!=0) {
        calls.call_original_30ffd0(
            controller.parent04,controller.originalPendingPolygon50);
        controller.originalPendingPolygon50=nullptr;
    }
    if (!controller.originalPendingPolygon50)
        return dispatchResult;

    // 0x57C6D0..0x57C6F8: the source reads these two globals
    // only with a still-pending +0x50. The second read is gated
    // by the first being nonzero; these are owner runtime data.
    if (calls.read_original_6354e7()!=0u &&
        calls.read_original_6354e0()==0u) {
        // 0x57C6FC..0x57C754 uses original graphics/debug state
        // at 0x72C6C8, the original 1.0f at 0x708A58 and calls
        // 0x2478EC/0x247D84. Do not replace with a synthetic log.
        calls.call_original_2478ec_247d84_warning();
    }

    // 0x57C758..0x57C778 restores $s3 (not the last callback's v0).
    return dispatchResult;
}
} // namespace ae3
