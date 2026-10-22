#include "ae3/controller_timeline.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
namespace {
void check(bool yes,const char* message){if(!yes)throw std::runtime_error(message);}
void scriptsActuallyAdvanceHostTime(){
    const char* luaSource=R"LUA(
        function setupArea(opts)
          new_ent("Controller",{name="test"},c_order(1,
              c_ctrl("se",{name="train_start"}),
              c_ctrl("wait",{time=1}),
              c_order(2,c_ctrl("value",{entity_name="train",argv0=4.5})),
              c_ctrl("trig_wait",{name="crate",flag=true}),
              c_ctrl("message_b",{target_name="ape",message="spawn"})))
        end
    )LUA";
    ae3::NativeStageWorld world;
    ae3::LuaStage lua(world);
    lua.executeSource(luaSource);
    lua.callSetupArea("entrance");
    check(world.entities().size()==1,"native Lua controller entity not created");
    ae3::NativeControllerTimeline timeline(world.entities()[0].controllers);
    auto tick=timeline.advance(0);
    check(tick.size()==1 && tick[0].opcode=="se",
          "native c_ctrl action did not dispatch event at time 0");
    check(tick[0].gameSeconds==0,"initial native event time wrong");
    check(!timeline.finished(),"native timeline ended before wait");
    check(timeline.advance(.5).empty(),"Lua wait ignored");
    check(std::abs(timeline.elapsed()-.5)<1e-6,"Lua wait measured wrong");
    tick=timeline.advance(.5);
    check(tick.size()==2,"nested c_order 2 repetitions not executed");
    check(tick[0].opcode=="value" && tick[1].opcode=="value",
          "nested controller opcode lost");
    check(tick[0].parameters.member("argv0")->numberOr()==4.5,
          "Lua numeric params lost in event dispatch");
    check(std::abs(tick[0].gameSeconds-1)<1e-6,"native wait completion time wrong");
    check(timeline.isWaitingForTrigger(),"native trig_wait was not enforced");
    check(!timeline.signal("incorrect"),"unrelated native signal released wait");
    check(!timeline.signal("crate",false),"wrong signal flag released wait");
    check(timeline.advance(2).empty(),"trig_wait advanced without event");
    check(timeline.signal("crate",true),"native trigger was not accepted");
    tick=timeline.advance(0);
    check(tick.size()==1 && tick[0].opcode=="message_b",
          "original Lua event message never dispatched to host");
    check(std::abs(tick[0].gameSeconds-3)<1e-6,"trigger elapsed wall-clock mismatch");
    check(timeline.finished(),"one-shot controller failed to complete");
}
void infiniteLoopAndGuard(){
    const char* source=R"LUA(
        function setupArea(opts)
          new_ent("Controller",{name="loop"},c_order(-1,
             c_ctrl("se",{name="tick"}),
             c_ctrl("wait",{time=0.25})))
        end
    )LUA";
    ae3::NativeStageWorld world;
    ae3::LuaStage lua(world);lua.executeSource(source);lua.callSetupArea("a");
    ae3::NativeControllerTimeline loop(world.entities()[0].controllers);
    auto events=loop.advance(1.0);
    check(events.size()>=4,"Lua repeat(-1) did not loop with fixed native timesteps");
    check(!loop.finished(),"infinite Lua controller unexpectedly finished");
    check(std::abs(loop.elapsed()-1.0)<1e-6,"native time accumulator drift");
    bool threw=false;
    try{loop.advance(-1);}catch(const std::runtime_error&){threw=true;}
    check(threw,"negative native delta accepted");
}

void originalDefaultWaitFromRetail(){
    ae3::NativeStageWorld world;
    ae3::LuaStage lua(world);
    lua.executeSource(R"LUA(
        function setupArea(opts)
          new_ent("Controller",{name="retail-default"},
              c_order(1,c_ctrl("wait",{}),
                        c_ctrl("se",{name="after_default_wait"})))
        end
    )LUA");
    lua.callSetupArea("a");
    ae3::NativeControllerTimeline timeline(world.entities()[0].controllers);
    check(timeline.advance(0).empty(),"default wait should not dispatch at t=0");
    check(timeline.advance(0.5).empty(),"retail default wait ended at 0.5s");
    const auto events=timeline.advance(0.5);
    check(events.size()==1 && events[0].opcode=="se","retail 1s default wait missing");
    check(std::abs(events[0].gameSeconds-1.0)<1e-6,
          "retail default wait duration is not exactly one second");
}

void zeroRepeat(){
    ae3::NativeStageWorld world;
    ae3::LuaStage lua(world);
    lua.executeSource(R"LUA(
        function setupArea(opts)
          new_ent("Controller",{name="skip"},c_order(0,c_ctrl("se",{name="never"})))
        end
    )LUA");
    lua.callSetupArea("entry");
    ae3::NativeControllerTimeline skip(world.entities()[0].controllers);
    check(skip.advance(10).empty(),"zero-repeat Lua controller executed unexpectedly");
    check(skip.finished(),"zero-repeat native controller did not terminate");
}
}
int main(){
    try{
        scriptsActuallyAdvanceHostTime();
        infiniteLoopAndGuard();
        zeroRepeat();
        originalDefaultWaitFromRetail();
        std::cout<<"NATIVE_LUA50_CONTROLLER_TIMELINE_PASSED "
                    "nested_orders=1 wait=1 trig_wait=1 repeat_forever=1 dispatch=1\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"NATIVE_LUA50_CONTROLLER_TIMELINE_FAILED: "<<e.what()<<"\n";
        return 1;
    }
}
