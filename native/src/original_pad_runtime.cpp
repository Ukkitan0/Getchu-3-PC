#include "ae3/original_pad_runtime.hpp"
#include <stdexcept>

namespace ae3 {
OriginalPadFrame OriginalPadFrameRuntime::advance(
    std::uint64_t frame,OriginalPadObservation observation) {
    if(initialized_ && frame<=previousFrame_)
        throw std::invalid_argument("original pad native frame must increase");
    if(!observation.connected)observation=OriginalPadObservation{};
    const std::uint16_t now=observation.pressedMask;
    const std::uint16_t pressed=static_cast<std::uint16_t>(now & ~previousButtons_);
    const std::uint16_t released=static_cast<std::uint16_t>(previousButtons_ & ~now);
    previousButtons_=now;
    previousFrame_=frame;
    initialized_=true;
    return {frame,observation,pressed,released};
}
void OriginalPadFrameRuntime::reset() noexcept {
    previousFrame_=0;
    previousButtons_=0;
    initialized_=false;
}
std::uint32_t originalSetPlayerWeaponSelection_512000(std::string_view name) noexcept {
    if(name=="empty")return 0;
    if(name=="net")return 1;
    if(name=="club")return 2;
    return 3;
}
void forwardOriginalGadgetSelection(std::optional<std::uint32_t> selection,
    std::uint64_t gameFrame,OriginalGadgetSelectionSink& sink) {
    if(selection)sink.onOriginalGadgetSelection({gameFrame,*selection});
}
} // namespace ae3
