#include "ae3/original_render_effect.hpp"
#include <stdexcept>
namespace ae3 {
void originalApplyRenderEffect2804f4(
    OriginalRenderResourceOwner& actor,
    OriginalRenderContextSlots& context,
    const OriginalRenderEffectDependencies& deps){
    // Reject early instead of silently skipping an original function.
    if(!deps.buildOriginalDescriptor269f7c||!deps.emitOriginalPacket3ae900)
        throw std::runtime_error("original 0x2804F4 render effect dependencies unavailable");
    auto& resource=originalEnsureRenderActorResource(actor); // 0x26A854
    const auto descriptor=deps.buildOriginalDescriptor269f7c(resource); // 0x26A4AC
    const auto packet=deps.emitOriginalPacket3ae900(descriptor); // 0x3AE900
    originalSetRenderSource830(context,packet); // 0x3B76F8
}
} // namespace ae3
