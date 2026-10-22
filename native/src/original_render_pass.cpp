#include "ae3/original_render_pass.hpp"
#include "ae3/original_render_visibility.hpp"
#include <stdexcept>

namespace ae3 {
namespace {
constexpr std::size_t maxSourceObjects=100000, maxEvents=1000000;
void event(OriginalRetailPassTrace& out,OriginalRenderId id,
           OriginalRetailDrawOp op,std::uint32_t va,
           std::uint32_t index=0,OriginalRetailBindArg arg=OriginalRetailBindArg::none,
           std::uint8_t mode=0){
    if(out.events.size()==maxEvents)
        throw std::runtime_error("original render call event budget exceeded");
    out.events.push_back({id,op,va,index,arg,mode});
}
bool cull(OriginalRenderId id,const OriginalRetailDrawProviders& p){
    if(!p.rejectByOriginalBounds)
        throw std::runtime_error("original bounds cull 0x26C9B8 has no implementation");
    return p.rejectByOriginalBounds(id);
}
bool effect(const OriginalRetailDrawObject& actor,const OriginalRetailDrawProviders& p){
    // Retail 0x26B3D0 checks actor +0x110 and if present calls 0x26A974:
    // inner resource byte +0x08. Evaluated at both sites, NOT cached.
    if(!actor.hasResourceHandle)return false;
    if(!p.originalResourceEffectEnabled)
        throw std::runtime_error("retail resource flag +0x08 accessor not bound");
    return p.originalResourceEffectEnabled(actor.id);
}
void validate(const std::vector<OriginalRetailDrawObject>& objects){
    if(objects.size()>maxSourceObjects)
        throw std::runtime_error("original render owner list exceeds host budget");
    for(const auto& obj:objects){
        if(!obj.id)throw std::invalid_argument("original render actor lacks stable identity");
        if(obj.selector&&!obj.hasResourceHandle)
            throw std::invalid_argument("selector set without retail resource handle");
    }
}
} // namespace
OriginalRetailPassTrace traceOriginalPrimaryRenderPass(
    const std::vector<OriginalRetailDrawObject>& objects,
    std::uint64_t sceneMask58,const OriginalRetailDrawProviders& p){
    validate(objects);
    OriginalRetailPassTrace out;
    for(const auto& actor:objects){
        ++out.considered;
        const OriginalRenderEarlyGate gate{actor.groupMask20,sceneMask58,
                                          actor.flag30,actor.flag35,actor.flag32};
        // 0x280838..0x280884; callback only demanded when flag32 != 0.
        if(!passesOriginalRetailRenderEarlyGate(gate,[&]{return cull(actor.id,p);} ))continue;
        ++out.accepted;
        // 0x280888, 0x26B550 and 0x26B57C: resource-specific draw
        // optionally switches current s1; otherwise uses inherited s2.
        std::uint32_t index=0;
        if(actor.selector && originalActorRenderSelectorActive(actor.selector)){
            index=originalReadRenderSelectorValue(*actor.selector);
            event(out,actor.id,OriginalRetailDrawOp::SelectResource,0x2802D4,index);
            event(out,actor.id,OriginalRetailDrawOp::BindResource,0x3B76F8,index,
                  OriginalRetailBindArg::savedSlot);
        }
        if(actor.flag33)
            event(out,actor.id,OriginalRetailDrawOp::DrawActor,0x2803B4,index);
        else // 0x280A90 (source branch from 0x2808D0)
            event(out,actor.id,OriginalRetailDrawOp::RestoreResource,0x3B76E8,index,
                  OriginalRetailBindArg::savedState);
        if(effect(actor,p)) // 0x2808E4 original 0x26B3D0
            event(out,actor.id,OriginalRetailDrawOp::ApplyResourceEffect,0x2804F4,index);
        if(!actor.flag34) // 0x280900
            event(out,actor.id,OriginalRetailDrawOp::BindResource,0x3B76F8,index,
                  OriginalRetailBindArg::none);
        event(out,actor.id,OriginalRetailDrawOp::DrawActorMode,0x26831C,index,
              OriginalRetailBindArg::none,1);
        const bool postEffect=effect(actor,p); // second original 0x26B3D0
        if(postEffect || !actor.flag34) // 0x28092C..0x280950
            event(out,actor.id,OriginalRetailDrawOp::BindResource,0x3B76F8,index,
                  OriginalRetailBindArg::savedSlot);
    }
    return out;
}
OriginalRetailPassTrace traceOriginalSecondaryRenderPass(
    const std::vector<OriginalRetailDrawObject>& objects,
    const OriginalRetailDrawProviders& p){
    validate(objects);
    OriginalRetailPassTrace out;
    for(const auto& actor:objects){
        ++out.considered;
        // Retail 0x280E48-0x280E74: unlike primary, neither group mask
        // nor object byte 0x32 is tested. Both enable bytes gate culling.
        if(!actor.flag30||!actor.flag35)continue;
        if(cull(actor.id,p))continue;
        ++out.accepted;
        if(actor.flag33)
            event(out,actor.id,OriginalRetailDrawOp::DrawActor,0x2803B4);
        else
            event(out,actor.id,OriginalRetailDrawOp::RestoreResource,0x3B76E8,0,
                  OriginalRetailBindArg::savedSlot);
        event(out,actor.id,OriginalRetailDrawOp::DrawActorMode,0x26831C,0,
              OriginalRetailBindArg::none,0);
    }
    return out;
}
} // namespace ae3
