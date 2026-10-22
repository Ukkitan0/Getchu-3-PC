#include "ae3/original_render_resource.hpp"
#include <cmath>
#include <stdexcept>

namespace ae3 {
OriginalRenderResourceState& originalEnsureRenderActorResource(OriginalRenderResourceOwner& owner){
    // Host optional is the native equivalent of original +0x110 handle
    // allocation; unlike malloc, unknown contents must not leak into state.
    if(!owner.state){
        owner.state.emplace(); // field layout initialized by 0x26A510
        owner.state->flag00=0;
        owner.state->flag08=0;
        owner.state->selector.active10=0;
        owner.state->references40to50.fill(0);
        owner.state->value04.reset();
        owner.state->value0c.reset();
        owner.state->selected14Initialized=false;
    }
    return *owner.state;
}
std::uint8_t originalReadActorFlag00(const OriginalRenderResourceOwner& o) noexcept {
    return o.state ? o.state->flag00 : 0;
}
std::uint8_t originalReadActorFlag08(const OriginalRenderResourceOwner& o) noexcept {
    return o.state ? o.state->flag08 : 0;
}
std::uint8_t originalReadActorFlag10(const OriginalRenderResourceOwner& o) noexcept {
    return o.state ? o.state->selector.active10 : 0;
}
void originalSetResourceFlag00(OriginalRenderResourceState& s,std::uint8_t value) noexcept {
    s.flag00=value;
}
void originalSetResourceFlag08(OriginalRenderResourceState& s,std::uint8_t value) noexcept {
    s.flag08=value;
}
void originalActivateResourceValue04(OriginalRenderResourceState& s,float value){
    if(!std::isfinite(value))throw std::invalid_argument("nonfinite original render scalar+4");
    s.flag00=1;s.value04=value;
}
void originalActivateResourceValue0c(OriginalRenderResourceState& s,float value){
    if(!std::isfinite(value))throw std::invalid_argument("nonfinite original render scalar+c");
    s.flag08=1;s.value0c=value;
}
void originalSelectResourceIndex(OriginalRenderResourceState& s,std::uint32_t index) noexcept{
    originalSetRenderSelector(s.selector,index);
    s.selected14Initialized=true;
}
void originalClearResourceIndex(OriginalRenderResourceState& s) noexcept{
    originalClearRenderSelector(s.selector);
}
std::uint32_t originalReadResourceIndex(const OriginalRenderResourceState& s){
    if(!s.selected14Initialized)
        throw std::runtime_error("original retail selector +0x14 not initialized");
    return originalReadRenderSelectorValue(s.selector);
}
} // namespace ae3
