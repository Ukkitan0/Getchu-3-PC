#include "ae3/original_render_actor_entry.hpp"
#include <cmath>
#include <stdexcept>

namespace ae3 {
OriginalRenderActorEntryResult originalAdvanceRetailRenderActorEntry(
    OriginalRenderActorEntryState& actor,float runtimeDelta,
    std::uint8_t globalRenderFlag,const OriginalRenderActorEntryCallbacks& cb){
    if(!cb.initial23c9cc||!cb.query23caa4||!cb.followup23cae8)
        throw std::runtime_error("original render actor +0xFC state methods missing");
    // Retail 0x268368 and 0x268374: always call both methods before
    // testing +0xF8, even if actor flag+0x30 will skip later drawing.
    cb.initial23c9cc();
    const std::int32_t state=cb.query23caa4();
    // 0x26837C bltzl skips original 0x23CAE8 on negative result.
    if(state>=0)cb.followup23cae8();
    // 0x268394-0x2683BC: only +0xF8 == 1 integrates the scalar.
    if(actor.modeF8==1){
        if(!std::isfinite(actor.accumulatorF0)||!std::isfinite(actor.scaleF4)||
           !std::isfinite(runtimeDelta))
            throw std::invalid_argument("nonfinite original renderer timing input");
        // Original MIPS mul.s then add.s, not a host fused-multiply-add.
        volatile float product=runtimeDelta*actor.scaleF4;
        const float advanced=actor.accumulatorF0+product;
        if(!std::isfinite(advanced))
            throw std::overflow_error("original renderer accumulated invalid float");
        actor.accumulatorF0=advanced;
    }
    // 0x2683C0/0x2683D0: disabled actor (+0x30=0) or nonzero global
    // runtime flag (+0x6496F0) exits with v0=0; else continue at 0x26841C.
    if(!actor.flag30||globalRenderFlag)
        return OriginalRenderActorEntryResult::returnZero;
    return OriginalRenderActorEntryResult::continueRender;
}
} // namespace ae3
