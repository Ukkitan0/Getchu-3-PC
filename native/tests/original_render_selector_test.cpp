#include "ae3/original_render_selector.hpp"
#include <cstdint>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
void run(){
    ae3::OriginalRenderSelector s{};
    require(!ae3::originalActorRenderSelectorActive(nullptr),"missing actor handle should return 0");
    require(ae3::originalReadRenderSelectorValue(s)==0,"initial selector wrong");
    ae3::originalSetRenderSelector(s,0x80000007u);
    require(ae3::originalReadRenderSelectorActive(s)==1,"selector enable field not set");
    require(ae3::originalReadRenderSelectorValue(s)==0x80000007u,"32-bit selector not retained");
    const auto selected=ae3::originalRenderDispatch(&s,1);
    require(selected.selectedResourcePath&&selected.resourceIndex==0x80000007u&&
            selected.actorFlag33Path,"retail selected draw route not followed");
    ae3::originalClearRenderSelector(s);
    require(ae3::originalReadRenderSelectorActive(s)==0,"clear must unset enabled byte");
    require(ae3::originalReadRenderSelectorValue(s)==0x80000007u,
            "original clear does not reset selected value");
    const auto skipped=ae3::originalRenderDispatch(&s,1);
    require(!skipped.selectedResourcePath&&skipped.actorFlag33Path,
            "retail actor secondary render pass must remain available");
    const auto missing=ae3::originalRenderDispatch(nullptr,0);
    require(!missing.selectedResourcePath&&!missing.actorFlag33Path,
            "absent actor handle must disable selected path");
    ae3::originalSetRenderSelector(s,0);
    require(ae3::originalRenderDispatch(&s,0).selectedResourcePath,
            "selector index zero is valid when resource enable is nonzero");
    s.active10=255;
    require(ae3::originalReadRenderSelectorActive(s)==255,
            "source lbu must preserve noncanonical enabled byte");
}
} // namespace
int main(){
    try{run();std::cout<<"ORIGINAL_RENDER_SELECTOR_PASS: retail_set_read_clear actor_optional dispatch\n";return 0;}
    catch(const std::exception& e){std::cerr<<"ORIGINAL_RENDER_SELECTOR_FAIL: "<<e.what()<<"\n";return 1;}
}
