#include "ae3/original_render_context.hpp"
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace {
void test(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
void run(){
    ae3::OriginalRenderContextSlots direct{};
    ae3::originalSetRenderSource828(direct,0x2222,0xfee137a9u);
    test(direct.source818==0xfee137a9u&&ae3::originalGetRenderSource828(direct)==0x2222,
         "source 3B76D0 32-bit nested payload not stored");
    ae3::originalSetRenderSource82c(direct,0x4444);
    ae3::originalSetRenderSource830(direct,0x5555);
    ae3::originalSetRenderSource834(direct,0x6666);
    direct.source824=0x3333;
    test(ae3::originalGetRenderSource824(direct)==0x3333&&
         ae3::originalGetRenderSource82c(direct)==0x4444&&
         ae3::originalGetRenderSource830(direct)==0x5555&&
         ae3::originalGetRenderSource834(direct)==0x6666,
         "original resource offsets not read back correctly");
    ae3::OriginalRenderContextSlots selected{}, defaultContext{};
    ae3::OriginalRetailPassTrace trace{};
    using ae3::OriginalRetailDrawOp;
    using ae3::OriginalRetailBindArg;
    trace.events={
        {9,OriginalRetailDrawOp::SelectResource,0x2802D4,17},
        {9,OriginalRetailDrawOp::BindResource,0x3B76F8,17,OriginalRetailBindArg::savedSlot},
        {9,OriginalRetailDrawOp::DrawActor,0x2803B4,17},
        {9,OriginalRetailDrawOp::BindResource,0x3B76F8,17,OriginalRetailBindArg::none},
        {9,OriginalRetailDrawOp::DrawActorMode,0x26831C,17,OriginalRetailBindArg::none,1},
        {9,OriginalRetailDrawOp::RestoreResource,0x3B76E8,17,OriginalRetailBindArg::savedState},
        {10,OriginalRetailDrawOp::RestoreResource,0x3B76E8,0,OriginalRetailBindArg::savedSlot},
    };
    ae3::OriginalRenderExternalPointers ptrs{0xB000,0xA000,0xC000};
    std::vector<std::uint32_t> invoked;
    const auto resolve=[&](ae3::OriginalRenderId id,std::uint32_t idx)->ae3::OriginalRenderContextSlots&{
        test(id==9||id==10,"renderer resolved unknown original actor");
        return idx==17?selected:defaultContext;
    };
    const auto draw=[&](const ae3::OriginalRetailDrawEvent& e,ae3::OriginalRenderContextSlots& s){
        test(e.id==9,"source callback called for unexpected actor");
        if(e.sourceVa==0x2803B4)test(s.source830==0xB000,"renderer state not bound before original draw");
        if(e.sourceVa==0x26831C)test(s.source830==0,"original zero resource reset not preserved");
        invoked.push_back(e.sourceVa);
    };
    ae3::applyOriginalRetailRenderTrace(trace,ptrs,resolve,draw);
    test(selected.source834==0xC000&&selected.source830==0&&selected.source82c==0xA000,
         "original render registers not in final retail state");
    test(defaultContext.source82c==0xB000,"retail default resource not restored");
    test(invoked==std::vector<std::uint32_t>({0x2803B4,0x26831C}),
         "source draw callback sequence not forwarded");
    bool failed=false;
    try{ae3::applyOriginalRetailRenderTrace(trace,ptrs,resolve,{});}
    catch(const std::runtime_error&){failed=true;}
    test(failed,"unrecovered original graphics routine silently skipped");
    failed=false;
    try{ae3::applyOriginalRetailRenderTrace(trace,ptrs,{},draw);}
    catch(const std::invalid_argument&){failed=true;}
    test(failed,"original resource lookup silently guessed");
}
}
int main(){
    try{run();std::cout<<"ORIGINAL_RENDER_CONTEXT_PASS: decoded source registers exact slot setters getters and stateful callback sequencing\n";return 0;}
    catch(const std::exception& e){std::cerr<<"ORIGINAL_RENDER_CONTEXT_FAIL: "<<e.what()<<"\n";return 1;}
}
