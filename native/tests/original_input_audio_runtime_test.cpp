#include "ae3/original_pad_runtime.hpp"
#include "ae3/original_audio_runtime.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void expect(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
struct GadgetRecorder final:ae3::OriginalGadgetSelectionSink {
    std::vector<ae3::OriginalGadgetSelectionEvent> seen;
    void onOriginalGadgetSelection(const ae3::OriginalGadgetSelectionEvent& e) override {seen.push_back(e);}
};
struct SoundRecorder final:ae3::OriginalSoundEventSink {
    std::vector<ae3::OriginalSoundEvent> seen;
    bool allow=true;
    bool submitOriginalSoundEvent(const ae3::OriginalSoundEvent& e) override {
        if(!allow)return false;
        seen.push_back(e);
        return true;
    }
};
ae3::ScriptValue fieldString(std::string value){
    ae3::ScriptValue f;f.kind=ae3::ScriptValue::Kind::text;f.text=std::move(value);return f;
}
void padFramesAndLoss(){
    ae3::OriginalPadFrameRuntime timeline;
    ae3::OriginalPadObservation physical;
    physical.connected=true;
    physical.pressedMask=ae3::originalPadBit(ae3::OriginalPadButton::cross)
                        |ae3::originalPadBit(ae3::OriginalPadButton::r1);
    physical.sticks={0,255,128,128};
    const auto first=timeline.advance(33,physical);
    expect(first.newlyPressed==physical.pressedMask,"initial held buttons lost");
    expect(first.newlyReleased==0 && first.raw.sticks==physical.sticks,
           "raw first observation changed");
    const auto repeat=timeline.advance(34,physical);
    expect(repeat.newlyPressed==0 && repeat.newlyReleased==0,
           "synthesized repeated press");
    const auto loss=timeline.advance(35,{});
    expect(loss.newlyReleased==physical.pressedMask && !loss.raw.connected,
           "device loss must release old buttons");
    expect(loss.raw.sticks==std::array<std::uint8_t,4>{128,128,128,128},
           "disconnected sticks must be neutral");
    const auto recovered=timeline.advance(36,physical);
    expect(recovered.newlyPressed==physical.pressedMask,"reconnected pad missing edge");
    bool rejected=false;
    try{timeline.advance(36,physical);}catch(const std::invalid_argument&){rejected=true;}
    expect(rejected,"duplicate native game frame accepted");
    timeline.reset();
    expect(timeline.advance(0,physical).newlyPressed==physical.pressedMask,
           "frame runtime reset retained held buttons");
}
void originalGadgetScriptSelection(){
    using ae3::originalSetPlayerWeaponSelection_512000;
    expect(originalSetPlayerWeaponSelection_512000("empty")==0,"empty mismatch");
    expect(originalSetPlayerWeaponSelection_512000("net")==1,"net mismatch");
    expect(originalSetPlayerWeaponSelection_512000("club")==2,"club mismatch");
    expect(originalSetPlayerWeaponSelection_512000("Club")==3,"unexpected case normalization");
    expect(originalSetPlayerWeaponSelection_512000("other")==3,"retail fallback lost");
    GadgetRecorder g;
    ae3::forwardOriginalGadgetSelection(std::nullopt,10,g);
    expect(g.seen.empty(),"fabricated unset gadget state");
    ae3::forwardOriginalGadgetSelection(2,11,g);
    expect(g.seen.size()==1 && g.seen[0].gameFrame==11 &&
           g.seen[0].originalSelection==2,"gadget event lost actual script selection");
}
void originalSeParameters(){
    ae3::NativeControllerEvent event;
    event.opcode="se";
    event.parameters.kind=ae3::ScriptValue::Kind::object;
    auto parsed=ae3::originalSeLuaFields_2191d4(event);
    expect(parsed && !parsed->name && !parsed->targetName &&
           parsed->volume==1.0f && !parsed->volumeSupplied,
           "retail se default volume or absent string fields changed");
    event.parameters.object["name"]=fieldString("original.se.id");
    event.parameters.object["target_name"]=fieldString("stage_actor");
    ae3::ScriptValue v;v.kind=ae3::ScriptValue::Kind::number;v.number=0.25;
    event.parameters.object["vol"]=v;
    parsed=ae3::originalSeLuaFields_2191d4(event);
    expect(parsed && parsed->name=="original.se.id" &&
           parsed->targetName=="stage_actor" &&
           parsed->volumeSupplied && parsed->volume==0.25f,
           "retail se name/target_name/vol mapping wrong");
    event.parameters.object["vol"]=fieldString("not a numeric Lua value");
    parsed=ae3::originalSeLuaFields_2191d4(event);
    expect(parsed && parsed->volume==1.f && !parsed->volumeSupplied,
           "non-numeric field should not fabricate numeric se volume");
    event.opcode="se_stop";
    expect(!ae3::originalSeLuaFields_2191d4(event),
           "se_stop incorrectly decoded as se");
}
void originalEntitySoundParameters(){
    ae3::NativeControllerEvent e;
    e.parameters.kind=ae3::ScriptValue::Kind::object;
    e.opcode="add_entity_se";
    e.parameters.object["exdb_se_id"]=fieldString("original.exdb.sound");
    e.parameters.object["entity_name"]=fieldString("actor");
    e.parameters.object["watch_rollerhandle_name"]=fieldString("roller");
    ae3::ScriptValue num;num.kind=ae3::ScriptValue::Kind::number;num.number=-0.5;
    e.parameters.object["sign"]=num;
    ae3::ScriptValue b;b.kind=ae3::ScriptValue::Kind::boolean;b.boolean=true;
    e.parameters.object["is_3d"]=b;
    auto f=ae3::originalEntitySoundLuaFields(e);
    expect(f && f->exdbSeId=="original.exdb.sound" &&
           f->entityName=="actor" && f->watchRollerhandleName=="roller" &&
           f->sign && *f->sign==-0.5f && f->is3d && *f->is3d &&
           !f->is2d && !f->markName,"add_entity_se parameters lost");
    e.opcode="kick_entity_se";
    e.parameters.object["mark_name"]=fieldString("mark");
    f=ae3::originalEntitySoundLuaFields(e);
    expect(f && f->exdbSeId=="original.exdb.sound" &&
           f->entityName=="actor" && f->markName=="mark" &&
           !f->sign && !f->is2d && !f->is3d && !f->watchRollerhandleName,
           "kick_entity_se parameters contaminated by unrelated sound opcode");
    e.opcode="kick_entity_effect";
    expect(!ae3::originalEntitySoundLuaFields(e),
           "effect command misclassified as source audio command");
}
void sourceSoundRoutesAndBackpressure(){
    const struct {const char* op;std::uint32_t va,ctor,bytes;} known[]={
        {"se",0x2191d4,0x373264,0xa0},
        {"se_stop",0x2190d8,0x3732fc,0x70},
        {"add_entity_se",0x219454,0x3738dc,0xd0},
        {"kick_entity_se",0x2195d0,0x373960,0xb0}};
    ae3::OriginalSoundEventRuntime runtime;
    ae3::OriginalAudioDescriptorValues resource;
    resource.sourceStrings={"test_original_name","song.mid","system","bank.bd","bank.hd"};
    resource.volumeScale=0.375f;
    for(int i=0;i<4;++i){
        ae3::NativeControllerEvent e;
        e.gameSeconds=i*0.25;
        e.opcode=known[i].op;
        e.parameters.kind=ae3::ScriptValue::Kind::object;
        e.parameters.object["name"]=fieldString("untouched_source_name");
        const auto route=ae3::originalSoundRoute(e.opcode);
        expect(route && route->originalDispatchVa==known[i].va &&
               route->originalConstructorVa==known[i].ctor &&
               route->originalObjectBytes==known[i].bytes,
               "original route/constructor/size mismatch");
        expect(runtime.enqueue(e,"retail_entity",i==0?
            std::optional<ae3::OriginalAudioDescriptorValues>{resource}:std::nullopt),
            "original stage sound command not queued");
    }
    ae3::NativeControllerEvent unrelated;
    unrelated.opcode="kick_entity_effect";
    expect(!runtime.enqueue(unrelated),"invented sound command from effect");
    expect(!ae3::originalSoundRoute("message"),"non-sound opcode classified as sound");
    SoundRecorder sink;
    sink.allow=false;
    expect(runtime.pump(sink)==0 && runtime.pending()==4,
           "backend rejection lost original sound commands");
    sink.allow=true;
    expect(runtime.pump(sink,2)==2 && runtime.pending()==2,
           "partial audio pump was not bounded");
    expect(runtime.pump(sink)==2 && runtime.pending()==0,
           "original sound event queue not drained");
    for(std::size_t i=0;i<sink.seen.size();++i){
        expect(sink.seen[i].originalControllerEvent.opcode==known[i].op &&
               sink.seen[i].sourceRoute.originalDispatchVa==known[i].va,
               "sound opcode ordering or VA lost");
        expect(sink.seen[i].originalEntityName=="retail_entity",
               "source event actor identity lost");
        expect(sink.seen[i].originalControllerEvent.parameters.member("name")->text==
               "untouched_source_name","Lua sound parameters modified");
        expect(sink.seen[i].originalControllerEvent.gameSeconds==i*.25,
               "timeline sound time changed");
    }
    expect(sink.seen.front().sourceDescriptor->value(3)=="bank.bd" &&
           sink.seen.front().sourceDescriptor->volumeScale==0.375f,
           "original asset-owned descriptor lost");
    expect(!sink.seen[1].sourceDescriptor,"fabricated descriptor on se_stop");
    runtime.clearOnOriginalStageTeardown();
    expect(runtime.pending()==0,"sound stage teardown not cleared");
}
void portableWin32AdapterNoInput(){
#ifndef _WIN32
    ae3::OriginalKeyboardBindings mappings;
    expect(!ae3::pollWin32OriginalKeyboard(nullptr,mappings).connected &&
           !ae3::pollWin32OriginalGamepad(nullptr).connected,
           "non-Windows Win32 adapter unexpectedly emitted input");
#endif
}
} // namespace
int main(){
    try{
        padFramesAndLoss();
        originalGadgetScriptSelection();
        originalSeParameters();
        originalEntitySoundParameters();
        sourceSoundRoutesAndBackpressure();
        portableWin32AdapterNoInput();
        std::cout<<"ORIGINAL_INPUT_AUDIO_BOUNDARY_TEST_PASSED "
                    "raw_pad=1 edges=1 disconnect=1 gadget_lua=1 "
                    "retail_sound_routes=4 queue_backpressure=1 descriptor_passthrough=1\n";
    }catch(const std::exception& e){
        std::cerr<<"ORIGINAL_INPUT_AUDIO_BOUNDARY_TEST_FAILED: "<<e.what()<<'\n';
        return 1;
    }
}
