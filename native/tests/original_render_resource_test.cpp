#include "ae3/original_render_resource.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
namespace {
void check(bool c,const char* m){if(!c)throw std::runtime_error(m);}
void run(){
    ae3::OriginalRenderResourceOwner actor{};
    check(!ae3::originalReadActorFlag00(actor) && !ae3::originalReadActorFlag08(actor) &&
          !ae3::originalReadActorFlag10(actor),"unallocated actor resource must be inactive");
    auto& resource=ae3::originalEnsureRenderActorResource(actor);
    check(actor.state.has_value(),"source +0x110 allocation missing");
    check(!resource.flag00&&!resource.flag08&&!resource.selector.active10,
          "source 0x26A510 must initialize original flag bytes");
    for(auto p:resource.references40to50)check(p==0,"source 0x26A510 must zero resource pointers");
    check(!resource.value04&&!resource.value0c&& !resource.selected14Initialized,
          "retail allocator does not explicitly initialize scalar payloads or selector");
    bool missing=false;
    try{(void)ae3::originalReadResourceIndex(resource);}catch(const std::runtime_error&){missing=true;}
    check(missing,"unspecified original selector was invented");
    ae3::originalActivateResourceValue04(resource,0.25f);
    ae3::originalActivateResourceValue0c(resource,0.75f);
    check(ae3::originalReadActorFlag00(actor)==1&&ae3::originalReadActorFlag08(actor)==1&&
          resource.value04.value()==0.25f&&resource.value0c.value()==0.75f,
          "original scalar source state not preserved");
    ae3::originalSelectResourceIndex(resource,0x8000000fu);
    check(ae3::originalReadActorFlag10(actor)==1&&
          ae3::originalReadResourceIndex(resource)==0x8000000fu,
          "original render resource index or activation missing");
    ae3::originalClearResourceIndex(resource);
    check(ae3::originalReadActorFlag10(actor)==0&&
          ae3::originalReadResourceIndex(resource)==0x8000000fu,
          "retail selector clear must preserve index value");
    ae3::originalSetResourceFlag08(resource,0);
    ae3::originalSetResourceFlag00(resource,0xfe);
    check(ae3::originalReadActorFlag00(actor)==0xfe&&!ae3::originalReadActorFlag08(actor),
          "source byte accessors must preserve raw u8 flags");
    auto* original=&resource;
    check(&ae3::originalEnsureRenderActorResource(actor)==original,
          "existing retail owner resource must be kept, not reallocated");
    bool nanRejected=false;
    try{ae3::originalActivateResourceValue04(resource,NAN);}catch(const std::invalid_argument&){nanRejected=true;}
    check(nanRejected,"nonfinite source scalar accepted");
}
}
int main(){
    try{run();std::cout<<"ORIGINAL_RENDER_RESOURCE_PASS: retail_actor_resource_owner zeroed_fields scalar_modes selector_persistence\n";return 0;}
    catch(const std::exception& e){std::cerr<<"ORIGINAL_RENDER_RESOURCE_FAIL: "<<e.what()<<"\n";return 1;}
}
