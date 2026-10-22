#include "ae3/original_render_context.hpp"
#include <stdexcept>

namespace ae3 {
void originalSetRenderSource828(OriginalRenderContextSlots& s,
                                std::uintptr_t p,std::uint32_t value) noexcept{
    s.source828=p; s.source818=value;
}
void originalSetRenderSource82c(OriginalRenderContextSlots& s,std::uintptr_t p) noexcept{s.source82c=p;}
void originalSetRenderSource830(OriginalRenderContextSlots& s,std::uintptr_t p) noexcept{s.source830=p;}
void originalSetRenderSource834(OriginalRenderContextSlots& s,std::uintptr_t p) noexcept{s.source834=p;}
std::uintptr_t originalGetRenderSource824(const OriginalRenderContextSlots& s) noexcept{return s.source824;}
std::uintptr_t originalGetRenderSource828(const OriginalRenderContextSlots& s) noexcept{return s.source828;}
std::uintptr_t originalGetRenderSource82c(const OriginalRenderContextSlots& s) noexcept{return s.source82c;}
std::uintptr_t originalGetRenderSource830(const OriginalRenderContextSlots& s) noexcept{return s.source830;}
std::uintptr_t originalGetRenderSource834(const OriginalRenderContextSlots& s) noexcept{return s.source834;}
void applyOriginalRetailRenderTrace(const OriginalRetailPassTrace& trace,
                                    const OriginalRenderExternalPointers& pointers,
                                    const OriginalRenderSlotResolver& resolve,
                                    const OriginalRenderComplexRoutine& originalCall){
    if(!resolve)throw std::invalid_argument("original render resource resolver missing");
    for(const auto& call:trace.events){
        auto& resource=resolve(call.id,call.selectorIndex);
        const auto arg=[&]()->std::uintptr_t{
            switch(call.argument){
                case OriginalRetailBindArg::none: return 0;
                case OriginalRetailBindArg::savedSlot:return pointers.savedSlot;
                case OriginalRetailBindArg::savedState:return pointers.savedState;
            }
            throw std::invalid_argument("unknown retail render pointer source");
        };
        switch(call.op){
            case OriginalRetailDrawOp::SelectResource:
                if(call.sourceVa!=0x2802D4)
                    throw std::invalid_argument("unexpected retail source selector routine");
                originalSetRenderSource834(resource,pointers.stageSource);
                break;
            case OriginalRetailDrawOp::BindResource:
                if(call.sourceVa!=0x3B76F8)
                    throw std::invalid_argument("unexpected retail source slot830 routine");
                originalSetRenderSource830(resource,arg());
                break;
            case OriginalRetailDrawOp::RestoreResource:
                if(call.sourceVa!=0x3B76E8)
                    throw std::invalid_argument("unexpected retail source slot82c routine");
                originalSetRenderSource82c(resource,arg());
                break;
            case OriginalRetailDrawOp::DrawActor:
            case OriginalRetailDrawOp::DrawActorMode:
            case OriginalRetailDrawOp::ApplyResourceEffect:
                if(!originalCall)
                    throw std::runtime_error("original render draw/effect implementation unavailable");
                originalCall(call,resource);
                break;
            default:throw std::invalid_argument("unknown original retail render operation");
        }
    }
}
} // namespace ae3
