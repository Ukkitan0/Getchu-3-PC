#include "ae3/original_render_actor_entry.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
namespace {
void check(bool ok,const char* msg){if(!ok)throw std::runtime_error(msg);}
void run(){
    ae3::OriginalRenderActorEntryState actor{};
    actor.flag30=1;actor.modeF8=1;actor.accumulatorF0=10.0f;actor.scaleF4=2.f;
    std::string history;
    std::int32_t state=-1;
    ae3::OriginalRenderActorEntryCallbacks callbacks{
        [&]{history+='A';},
        [&]{history+='B';return state;},
        [&]{history+='C';}
    };
    auto status=ae3::originalAdvanceRetailRenderActorEntry(actor,.125f,0,callbacks);
    check(status==ae3::OriginalRenderActorEntryResult::continueRender &&
          actor.accumulatorF0==10.25f && history=="AB",
          "original 0x26831C source scalar integration/negative callback skip wrong");
    state=0;history.clear();actor.modeF8=2;
    status=ae3::originalAdvanceRetailRenderActorEntry(actor,.25f,0,callbacks);
    check(actor.accumulatorF0==10.25f && history=="ABC",
          "source callback 0x23CAE8 nonnegative branch or modeF8 guard wrong");
    history.clear();actor.flag30=0;actor.modeF8=1;
    status=ae3::originalAdvanceRetailRenderActorEntry(actor,.125f,0,callbacks);
    check(status==ae3::OriginalRenderActorEntryResult::returnZero &&
          actor.accumulatorF0==10.5f && history=="ABC",
          "source actor disabled branch occurs AFTER scalar update");
    actor.flag30=1;
    status=ae3::originalAdvanceRetailRenderActorEntry(actor,.125f,1,callbacks);
    check(status==ae3::OriginalRenderActorEntryResult::returnZero,
          "source global render inhibit gate wrong");
    bool missing=false;
    try{ae3::originalAdvanceRetailRenderActorEntry(actor,.125f,0,{});}
    catch(const std::runtime_error&){missing=true;}
    check(missing,"unrecovered original child-state methods silently omitted");
    bool bad=false;
    try{ae3::originalAdvanceRetailRenderActorEntry(actor,NAN,0,callbacks);}
    catch(const std::invalid_argument&){bad=true;}
    check(bad,"invalid source tick was silently inserted into model state");
}
}
int main(){
    try{run();std::cout<<"ORIGINAL_RENDER_ACTOR_ENTRY_PASS: initial_state virtual_checks timed_float first_draw_gate\n";return 0;}
    catch(const std::exception& e){std::cerr<<"ORIGINAL_RENDER_ACTOR_ENTRY_FAIL: "<<e.what()<<"\n";return 1;}
}
