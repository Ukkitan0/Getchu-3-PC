#include "ae3/original_render_pass.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace ae3;
namespace {
void require(bool c,const char* m){if(!c)throw std::runtime_error(m);}
std::vector<std::uint32_t> vas(const OriginalRetailPassTrace& t){
    std::vector<std::uint32_t> out;
    for(const auto& e:t.events)out.push_back(e.sourceVa);
    return out;
}
void test(){
    OriginalRenderSelector selector{};
    originalSetRenderSelector(selector,17);
    OriginalRetailDrawObject obj{};
    obj.id=9;obj.groupMask20=1;obj.flag30=obj.flag35=obj.flag33=1;
    obj.hasResourceHandle=true;obj.selector=&selector;
    unsigned culls=0,effects=0;
    OriginalRetailDrawProviders providers{
        [&](OriginalRenderId id){require(id==9,"wrong original object cull ID");++culls;return true;},
        [&](OriginalRenderId id){require(id==9,"wrong original effect object ID");++effects;return effects==1;}
    };
    auto t=traceOriginalPrimaryRenderPass({obj},1,providers);
    require(t.considered==1&&t.accepted==1&&t.acceptedIds==std::vector<OriginalRenderId>{9}&&culls==0,"source gate should not cull when +0x32 unset");
    require(effects==2,"original effect flag must be re-evaluated after actor draw");
    require(vas(t)==std::vector<std::uint32_t>({0x2802D4,0x3B76F8,0x2803B4,0x2804F4,
                                                0x3B76F8,0x26831C,0x3B76F8}),
            "retail primary graphics call ordering incorrect");
    require(t.events[0].selectorIndex==17 &&
            t.events[1].argument==OriginalRetailBindArg::savedSlot &&
            t.events[4].argument==OriginalRetailBindArg::none &&
            t.events[5].mode==1,"original graphics argument routes lost");
    obj.flag34=1;
    t=traceOriginalPrimaryRenderPass({obj},1,providers);
    require(vas(t)==std::vector<std::uint32_t>({0x2802D4,0x3B76F8,0x2803B4,0x26831C}),
            "flag34 must suppress pre/post restore when effect off");
    originalClearRenderSelector(selector);
    obj.flag33=0;obj.flag34=0;obj.hasResourceHandle=false;obj.selector=nullptr;
    t=traceOriginalPrimaryRenderPass({obj},1,providers);
    require(vas(t)==std::vector<std::uint32_t>({0x3B76E8,0x3B76F8,0x26831C,0x3B76F8}),
            "source fallback-path call sequence incorrect");
    require(t.events[0].argument==OriginalRetailBindArg::savedState,"first pass fallback needs s7 state");
    obj.flag32=1;
    providers.originalBoundsVisible=[&](OriginalRenderId){++culls;return false;};
    t=traceOriginalPrimaryRenderPass({obj},1,providers);
    require(t.events.empty()&&t.acceptedIds.empty()&&t.accepted==0&&culls==1,"enabled source cull should reject early");
    providers.originalBoundsVisible=[&](OriginalRenderId){++culls;return true;};
    t=traceOriginalSecondaryRenderPass({obj},providers);
    require(vas(t)==std::vector<std::uint32_t>({0x3B76E8,0x26831C}),
            "secondary source pass should restore and draw actor mode zero");
    require(t.events.front().argument==OriginalRetailBindArg::savedSlot &&
            t.events.back().mode==0,"secondary pass argument semantics wrong");
    obj.flag33=1;obj.groupMask20=0;
    t=traceOriginalSecondaryRenderPass({obj},providers);
    require(vas(t)==std::vector<std::uint32_t>({0x2803B4,0x26831C}),
            "secondary must not use primary group mask or flag32");
    obj.flag35=0;
    t=traceOriginalSecondaryRenderPass({obj},providers);
    require(t.events.empty(),"secondary enable guard missing");
    obj.flag35=1;providers.originalBoundsVisible={};
    bool threw=false;
    try{traceOriginalSecondaryRenderPass({obj},providers);}catch(const std::runtime_error&){threw=true;}
    require(threw,"unrecovered source bounds must never be guessed");
    obj.flag32=0;obj.groupMask20=1;
    t=traceOriginalPrimaryRenderPass({obj},1,providers);
    require(!t.events.empty(),"primary source cull should not be required for flag32=0");
    obj.id=0;threw=false;
    try{traceOriginalPrimaryRenderPass({obj},1,providers);}catch(const std::invalid_argument&){threw=true;}
    require(threw,"original actor ID validation missing");
}
}
int main(){
    try{test();std::cout<<"ORIGINAL_RETAIL_RENDER_PASSES_PASS: primary secondary source_calls order masks optional_cull selector actor_flags dynamic_effect\n";return 0;}
    catch(const std::exception& e){std::cerr<<"ORIGINAL_RETAIL_RENDER_PASSES_FAIL: "<<e.what()<<"\n";return 1;}
}
