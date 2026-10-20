#include "ae3/lua_stage.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
void check(bool ok,const char* why){
    if(!ok)throw std::runtime_error(why);
}
std::string get(const ae3::ScriptValue& v,const std::string& name){
    auto* f=v.member(name);
    if(!f || f->kind!=ae3::ScriptValue::Kind::text)
        throw std::runtime_error("missing original Lua table string property "+name);
    return f->text;
}
void runRealScriptSample(){
    // Written from original-style binding vocabulary, not an emulator.
    // A real Lua 5.0 parser, evaluator and call stack MUST run these branches.
    const std::string code=R"LUA(
      script_description = "native stage binding test"
      lua_param({ namespace = "piece_00", piece_type = "CUBIC", piece_num = 9 })
      function brk_wbox_l(name, sound, num)
        lua_param({ namespace = name, len_add_sensor = 10000 })
        new_ent("Breakable", {
           name=name, model_name="wbox_l", flg_save=false, shake_force=0.2 })
        new_ent("Controller", {name=sound}, c_order(1,
           c_ctrl("trig_wait", {name=name,flag=true}),
           c_ctrl("se", {name="st_brk_woodbox", target_name=name}),
           c_order(num,c_ctrl("message_b", {message="spawn",target_name=name}))))
      end
      function setupArea(opts)
        if opts.spawn_tag == "entrance" then
          new_ent("Player", {name="player",tag_name=opts.spawn_tag})
          new_ent("Monkey", {name="ape_cty_01",tag_name="ape_01",speed=4.5})
          new_ent("Monkey", {name="ape_cty_02",tag_name="ape_02"})
          brk_wbox_l("crate", "crate_se", 3)
        end
        gw_set_flag("stage_started")
        gw_set_flag("always_true_even_with_false_second_arg", false)
        lw_set_flag("default_on")
        area_set_flag("door_open", false)
        if gw_get_flag("stage_started") and not area_get_flag("door_open") then
           new_ent("Button", {name="door_switch"})
        end
      end
    )LUA";
    ae3::NativeStageWorld world;
    ae3::LuaStage script(world);
    script.executeSource(code);
    check(world.parameters().size()==1,"top level lua_param did not execute");
    script.callSetupArea("entrance");
    check(world.entities().size()==6,"real Lua 5.0 setupArea entity side effects lost");
    check(world.entities()[0].entityClass=="Player","player native entity missing");
    check(world.entities()[0].tagName=="entrance","spawn options not passed to Lua");
    check(world.entities()[1].entityClass=="Monkey","original enemy not constructed");
    check(std::abs(world.entities()[1].fields.member("speed")->numberOr()-4.5)<1e-6,
          "original Lua numeric entity params discarded");
    check(world.entities()[3].entityClass=="Breakable","Lua function spawning broken");
    const auto* booleanField=world.entities()[3].fields.member("flg_save");
    check(booleanField && booleanField->kind==ae3::ScriptValue::Kind::boolean &&
          !booleanField->boolean,"Lua booleans lost in C++ binding");
    check(world.entities()[4].entityClass=="Controller","Lua controller not captured");
    const auto& order=world.entities()[4].controllers;
    check(get(order,"kind")=="order","controller c_order not faithfully represented");
    const auto* count=order.member("repeat");
    check(count && count->numberOr()==1,"controller repeat count lost");
    const auto* children=order.member("children");
    check(children && children->kind==ae3::ScriptValue::Kind::array &&
          children->array.size()==3,"nested controller command graph missing");
    check(get(children->array[0],"opcode")=="trig_wait","controller event missing");
    check(get(children->array[2],"kind")=="order","nested c_order not preserved");
    check(world.parameters().size()==2,"function-local lua_param did not execute");
    check(world.flag("gw","stage_started"),"global world Lua set_flag lost");
    check(world.flag("gw","always_true_even_with_false_second_arg"),
          "original gw_set_flag always sets true, regardless of Lua argument two");
    check(world.flag("lw","default_on"),
          "original lw_set_flag defaults to true without a second boolean argument");
    check(!world.flag("area","door_open"),"area flag must be false");
    // Lua condition must be evaluated, not merely parsed by a regex.
    check(world.entities().back().name=="door_switch","nested condition not executed");
}
void originalProgressBindingTests(){
    // Grounded in original ELF virtual methods 0x617924 and 0x617958;
    // costume status is a fixed original Data field compared to exactly 2.
    ae3::PlayerProgress progress{};
    progress.capture_flag[29]=1;
    progress.costume_possess[7]=2;
    progress.costume_possess[3]=0;
    ae3::NativeStageWorld world;
    ae3::LuaStage stage(world,&progress);
    stage.executeSource(R"LUA(
      function setupArea(opts)
        if get_capture_flag(29) and get_costume_possess() then
          new_ent('Monkey',{name='persisted_monkey_captured'})
        end
        if not get_capture_flag(30) and not get_costume_possess_ninja() then
          new_ent('Player',{name='missing_capture_and_costume'})
        end
      end
    )LUA");
    stage.callSetupArea("entry");
    check(world.entities().size()==2,
          "native original AE3 Lua bindings ignored real capture/costume save flags");
    check(world.entities()[0].name=="persisted_monkey_captured",
          "original captured monkey condition did not use recovered Data field");
    check(world.entities()[1].name=="missing_capture_and_costume",
          "original uncaptured monkey condition ignored false save state");
    progress.costume_possess[7]=1;
    ae3::NativeStageWorld exactWorld;
    ae3::LuaStage exact(exactWorld,&progress);
    exact.executeSource("function setupArea(opts) if get_costume_possess() then new_ent('Player',{name='wrong'}) end end");
    exact.callSetupArea("entry");
    check(exactWorld.entities().empty(),
          "original costume query compares to 2, not truthy/nonzero");
    ae3::NativeStageWorld badWorld;
    ae3::LuaStage bad(badWorld,&progress);
    bad.executeSource("function setupArea(opts) return get_capture_flag(570) end");
    bool rejected=false;
    try{bad.callSetupArea("entry");}
    catch(const std::runtime_error&){rejected=true;}
    check(rejected,"real capture flag boundary allowed invalid original save index");
}
void originalEngineBindingsFromElf(){
    // These are direct translations of two original machine-code callbacks,
    // not a synthetic gameplay implementation or a guessed gadget system.
    struct Example {int playerType;const char* sex;const char* weapon;int code;};
    const Example cases[]={
        {0,"boy","empty",0},
        {1,"girl","net",1},
        {5,"nil","club",2},
        {-1,"nil","other-original-code",3}
    };
    for(const auto& test:cases){
        ae3::PlayerProgress progress{};
        progress.player_type=test.playerType;
        ae3::NativeStageWorld world;
        ae3::LuaStage native(world,&progress);
        const std::string chunk=
            std::string("function setupArea(o) new_ent('OriginalResult',{name=get_sex_distinction()}); ") +
            "set_player_weapon('"+test.weapon+"') end";
        native.executeSource(chunk);
        native.callSetupArea("entry");
        check(world.entities().size()==1,"original sex callback lost return value");
        check(world.entities()[0].name==test.sex,
              "original ELF get_sex_distinction returned incorrect text");
        check(world.playerWeaponSelection().has_value(),
              "original ELF set_player_weapon did not store selection");
        check(*world.playerWeaponSelection()==test.code,
              "original ELF weapon string-to-integer mapping changed");
        world.clear();
        check(!world.playerWeaponSelection().has_value(),
              "game weapon selection leaked after native stage reset");
    }
    // This callback reads original progression Data rather than returning
    // an invented player kind when the Data singleton has not been loaded.
    ae3::NativeStageWorld world;
    ae3::LuaStage missing(world);
    missing.executeSource("function setupArea(o) return get_sex_distinction() end");
    bool blocked=false;
    try{missing.callSetupArea("entry");}
    catch(const std::runtime_error&){blocked=true;}
    check(blocked,"game sex query did not reject missing original Data");
}
void errorCases(){
    ae3::NativeStageWorld world;
    ae3::LuaStage lua(world);
    bool rejected=false;
    try{lua.executeSource("\x1bLua\x50\x01\x02");}
    catch(const std::runtime_error&){rejected=true;}
    check(rejected,"original PS2 bytecode was incorrectly executed as host bytecode");
    rejected=false;
    try{lua.executeSource("function setupArea(");}
    catch(const std::runtime_error&){rejected=true;}
    check(rejected,"syntax error silently accepted");
    rejected=false;
    try{lua.callSetupArea("entry");}
    catch(const std::runtime_error&){rejected=true;}
    check(rejected,"missing setupArea call silently accepted");
    // No operating system/environment or file access exposed to area scripts.
    lua.executeSource("assert(io==nil and os==nil and dofile==nil and loadfile==nil)");
    lua.executeSource("function setupArea(opts) new_ent('Player',{name='safe'}) end");
    lua.callSetupArea("any");
    check(world.entities().size()==1,"Lua interpreter failed to recover after syntax error");
}
}
int main(){
    try{
        runRealScriptSample();
        originalProgressBindingTests();
        originalEngineBindingsFromElf();
        errorCases();
        std::cout<<"NATIVE_REAL_LUA50_EXECUTION_PASSED: "
                    "setupArea,new_ent,lua_param,c_order,c_ctrl,flags,control flow\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"NATIVE_LUA50_RUNTIME_TEST_FAILED: "<<e.what()<<"\n";
        return 1;
    }
}
