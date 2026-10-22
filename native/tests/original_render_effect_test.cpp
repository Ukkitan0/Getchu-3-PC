#include "ae3/original_render_effect.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
namespace {
void check(bool c,const char* m){if(!c)throw std::runtime_error(m);}
void run(){
    ae3::OriginalRenderResourceOwner actor{};
    ae3::OriginalRenderContextSlots ctx{};
    ctx.source830=0x123;
    std::string calls;
    ae3::OriginalRenderEffectDependencies deps{
        [&](const ae3::OriginalRenderResourceState& r)->std::uintptr_t{
            check(actor.state.has_value(),"original 0x26A854 must allocate before transform");
            check(!r.flag00&&!r.flag08&&!r.selector.active10,
                  "original resource must be initialized before 0x269F7C");
            calls+='A';return 0xC010;
        },
        [&](std::uintptr_t source)->std::uintptr_t{
            check(source==0xC010,"original converted descriptor not forwarded");
            calls+='B';return 0xAE3E;
        }
    };
    ae3::originalApplyRenderEffect2804f4(actor,ctx,deps);
    check(calls=="AB"&&ae3::originalGetRenderSource830(ctx)==0xAE3E,
          "retail 0x2804F4 packet context write/order not preserved");
    calls.clear();
    actor.state->flag08=1;
    ae3::originalApplyRenderEffect2804f4(actor,ctx,{
        [&](const ae3::OriginalRenderResourceState& r){
            calls+='C';check(r.flag08==1,"retail effect must reuse actor resource");
            return std::uintptr_t(0);
        },
        [&](std::uintptr_t handle){calls+='D';return handle;}
    });
    check(calls=="CD"&&ctx.source830==0,
          "original packet null result must replace previously bound pointer");
    bool missing=false;
    try{ae3::originalApplyRenderEffect2804f4(actor,ctx,{});}
    catch(const std::runtime_error&){missing=true;}
    check(missing,"unrecovered effect functions were guessed");
}
}
int main(){
    try{run();std::cout<<"ORIGINAL_RENDER_EFFECT_PASS: retail_2804F4 source_allocation descriptor_conversion context_bind\n";return 0;}
    catch(const std::exception& e){std::cerr<<"ORIGINAL_RENDER_EFFECT_FAIL: "<<e.what()<<"\n";return 1;}
}
