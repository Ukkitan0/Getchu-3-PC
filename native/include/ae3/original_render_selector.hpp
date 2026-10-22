#pragma once
#include <cstdint>

namespace ae3 {
// Recovered USA ELF resource fields: inner pointer from a handle's +0x00,
// active byte at inner+0x10, selection u32 at inner+0x14.
// This is HOST data, not a PS2 memory emulation structure.
struct OriginalRenderSelector {
    std::uint8_t active10=0;
    std::uint32_t selected14=0;
};
// Original 0x26AEB4/0x26AEC8/0x26AED4/0x26AEE0.
void originalSetRenderSelector(OriginalRenderSelector& state, std::uint32_t selector) noexcept;
std::uint8_t originalReadRenderSelectorActive(const OriginalRenderSelector& state) noexcept;
std::uint32_t originalReadRenderSelectorValue(const OriginalRenderSelector& state) noexcept;
void originalClearRenderSelector(OriginalRenderSelector& state) noexcept;
// Original 0x26B550 first checks actor+0x110 for null; only that
// wrapper safely returns zero for a missing resource handle.
std::uint8_t originalActorRenderSelectorActive(const OriginalRenderSelector* state) noexcept;
// Source 0x280888-0x2808D4 chooses the resource-specific draw path only
// when the selector is enabled; actor flag +0x33 independently controls
// the next render path. These are route decisions, not rendering calls.
struct OriginalRenderDispatch {
    bool selectedResourcePath=false;
    std::uint32_t resourceIndex=0;
    bool actorFlag33Path=false;
};
OriginalRenderDispatch originalRenderDispatch(const OriginalRenderSelector* state,
                                             std::uint8_t actorFlag33) noexcept;
} // namespace ae3
