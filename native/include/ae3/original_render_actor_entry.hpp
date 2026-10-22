#pragma once
#include <cstdint>
#include <functional>

namespace ae3 {
// Verified retail original renderer 0x26831C-0x268424 entry state.
// These decoded native fields correspond to original actor offsets; this is
// neither a replacement animation engine nor a PS2 machine state.
struct OriginalRenderActorEntryState {
    std::uint8_t flag30=0;
    std::uint32_t modeF8=0;
    float accumulatorF0=0;
    float scaleF4=0;
};
// 0x23C9CC, 0x23CAA4, 0x23CAE8 operate on owner+0xFC; the real engine must
// provide their implementations. The source passes literal zero as second
// argument to the last two routines.
struct OriginalRenderActorEntryCallbacks {
    std::function<void()> initial23c9cc;
    std::function<std::int32_t()> query23caa4;
    std::function<void()> followup23cae8;
};
enum class OriginalRenderActorEntryResult {returnZero,continueRender};
// Runtime delta from original 0x635488 (uninitialized .data in private ELF)
// and gate byte from global 0x6496F0 MUST come from actual game runtime.
OriginalRenderActorEntryResult originalAdvanceRetailRenderActorEntry(
    OriginalRenderActorEntryState& actor,
    float originalRuntimeDelta635488,
    std::uint8_t originalGlobalFlag6496f0,
    const OriginalRenderActorEntryCallbacks& callbacks);
} // namespace ae3
