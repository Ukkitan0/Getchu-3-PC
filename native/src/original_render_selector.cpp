#include "ae3/original_render_selector.hpp"

namespace ae3 {
void originalSetRenderSelector(OriginalRenderSelector& state,std::uint32_t selector) noexcept {
    // Retail 0x26AEB4: sw a1,0x14(inner); sb 1,0x10(inner).
    state.selected14=selector;
    state.active10=1;
}
std::uint8_t originalReadRenderSelectorActive(const OriginalRenderSelector& state) noexcept {
    return state.active10; // Retail 0x26AEC8: lbu v0,0x10(inner)
}
std::uint32_t originalReadRenderSelectorValue(const OriginalRenderSelector& state) noexcept {
    return state.selected14; // Retail 0x26AED4: lw v0,0x14(inner)
}
void originalClearRenderSelector(OriginalRenderSelector& state) noexcept {
    state.active10=0; // Retail 0x26AEE0: sb zero,0x10(inner), retains selector
}
std::uint8_t originalActorRenderSelectorActive(const OriginalRenderSelector* state) noexcept {
    return state ? originalReadRenderSelectorActive(*state) : 0;
}
OriginalRenderDispatch originalRenderDispatch(const OriginalRenderSelector* state,
                                              std::uint8_t flag33) noexcept {
    OriginalRenderDispatch out;
    out.selectedResourcePath=originalActorRenderSelectorActive(state)!=0;
    if(out.selectedResourcePath)out.resourceIndex=originalReadRenderSelectorValue(*state);
    out.actorFlag33Path=flag33!=0;
    return out;
}
} // namespace ae3
