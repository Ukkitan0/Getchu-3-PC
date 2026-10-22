#pragma once
#include "ae3/original_render_context.hpp"
#include "ae3/original_render_resource.hpp"
#include <cstdint>
#include <functional>

namespace ae3 {
// Verified original USA 0x2804F4..0x280540 effect/resource pipeline.
// 0x269F7C and 0x3AE900 are not recovered yet: callbacks must implement
// those genuine operations on native host data, never replacement effects.
struct OriginalRenderEffectDependencies {
    // Original 0x26A4AC -> 0x269F7C populates temporary effect descriptor.
    // Return a host-owned opaque handle for that converted descriptor.
    std::function<std::uintptr_t(const OriginalRenderResourceState&)> buildOriginalDescriptor269f7c;
    // Original 0x3AE900 reads runtime renderer +0x64942C and the descriptor
    // and returns a native host-owned packet handle.
    std::function<std::uintptr_t(std::uintptr_t)> emitOriginalPacket3ae900;
};
// Reproduces retail 0x2804F4 original call order: ensure +0x110 resource,
// 0x26A4AC -> 0x269F7C -> 0x3AE900 -> 0x3B76F8 write context+0x830.
void originalApplyRenderEffect2804f4(
    OriginalRenderResourceOwner& originalActor,
    OriginalRenderContextSlots& sourceContext,
    const OriginalRenderEffectDependencies& dependencies);
} // namespace ae3
