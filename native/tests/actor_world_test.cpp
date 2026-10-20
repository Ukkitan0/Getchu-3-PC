#include "ae3/actor_world.hpp"
#include "ae3/lua_stage.hpp"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void require(bool yes,const char* why){if(!yes)throw std::runtime_error(why);}
void u32(std::vector<std::uint8_t>& b,std::uint32_t x){
    for(int i=0;i<4;++i)b.push_back(std::uint8_t(x>>(i*8)));
}
std::vector<std::uint8_t> triangle(){
    const std::string json=
        "{\"asset\":{\"version\":\"2.0\"},"
        "\"buffers\":[{\"byteLength\":36}],"
        "\"bufferViews\":[{\"buffer\":0,\"byteOffset\":0,\"byteLength\":36,\"target\":34962}],"
        "\"accessors\":[{\"bufferView\":0,\"componentType\":5126,\"count\":3,"
        "\"type\":\"VEC3\",\"min\":[0,0,0],\"max\":[1,1,0]}],"
        "\"meshes\":[{\"primitives\":[{\"attributes\":{\"POSITION\":0},\"mode\":4}]}],"
        "\"nodes\":[{\"mesh\":0}],\"scenes\":[{\"nodes\":[0]}],\"scene\":0}";
    std::string padded=json;
    while(padded.size()%4)padded.push_back(' ');
    std::vector<std::uint8_t> out;
    u32(out,0x46546C67);u32(out,2);
    u32(out,static_cast<std::uint32_t>(12+8+padded.size()+8+36));
    u32(out,static_cast<std::uint32_t>(padded.size()));u32(out,0x4E4F534A);
    out.insert(out.end(),padded.begin(),padded.end());
    u32(out,36);u32(out,0x004E4942);
    for(float p:{1.f,0.f,0.f,0.f,1.f,0.f,0.f,0.f,0.f}){
        std::uint32_t bits=0;std::memcpy(&bits,&p,4);u32(out,bits);
    }
    return out;
}
void write(const std::filesystem::path& p,const std::vector<std::uint8_t>& b){
    std::ofstream stream(p,std::ios::binary);
    stream.write(reinterpret_cast<const char*>(b.data()),std::streamsize(b.size()));
    require(bool(stream),"cannot create game-free actor GLB");
}
void integration(){
    namespace fs=std::filesystem;
    const auto root=fs::temp_directory_path()/"getchu3_native_lua_actor_world";
    fs::remove_all(root);
    fs::create_directories(root);
    try{
        write(root/"monkey.glb",triangle());
        write(root/"player.glb",triangle());
        // Original AE3 PLC tag_name-to-world-coordinate placement sample.
        std::vector<std::uint8_t> plc(0xc0+0x80);
        const char tag[]="ape_spawn_tag";
        std::memcpy(plc.data()+0xc0,tag,sizeof(tag));
        const std::uint32_t count=1;
        std::memcpy(plc.data(),&count,sizeof(count));
        const float originalPosition[3]={-2.f,1.f,4.f};
        std::memcpy(plc.data()+0xc0+0x40,originalPosition,sizeof(originalPosition));
        write(root/"placeholder.plc",plc);
        {
            std::ofstream file(root/"actor_bindings.txt");
            file<<"# Explicit owner-local name and class mapping\n"
                <<"player=player.glb\n"
                <<"class:Monkey=monkey.glb\n";
        }
        const std::string lua=R"LUA(
            function setupArea(opts)
              new_ent("Player",{name="player",pos={0,0,2}})
              new_ent("Monkey",{name="ape_01",pos={2,1,3}})
              new_ent("Monkey",{name="ape_02",tag_name="ape_spawn_tag"})
              new_ent("Controller",{name="event_only"})
              lua_param({namespace="real_stage"})
            end
        )LUA";
        ae3::NativeStageWorld stage;
        ae3::LuaStage vm(stage);
        vm.executeSource(lua);vm.callSetupArea("spawn");
        require(stage.entities().size()==4,"original Lua new_ent was not executed");
        ae3::StaticScene backdrop;
        backdrop.triangles={
            {10,0,10,1,1,1,1,0,0},
            {11,0,10,1,1,1,1,0,0},
            {10,1,10,1,1,1,1,0,0}
        };
        backdrop.batches={{0,3,-1}};
        ae3::NativeActorWorld world(backdrop);
        world.bind(stage,root,root/"actor_bindings.txt");
        const auto& report=world.report();
        require(report.stageEntities==4 && report.actorsCreated==3,
                "actual Lua stage actors not instantiated from private GLB assets");
        require(report.unboundEntities==1 && report.modelsRejected==0,
                "non-rendered script-only entity incorrectly loaded as model");
        require(world.actors().size()==3,"missing player and two monkey instances");
        require(world.scene().triangles.size()==12,
                "animated actor scene did not retain background plus three GLBs");
        require(world.scene().batches.size()==4,
                "original world meshes and native actor batch offsets lost");
        const auto& player=*world.actor("player");
        const auto& ape=*world.actor("ape_01");
        const auto& apeTag=*world.actor("ape_02");
        require(report.matchedOriginalPlcTags==1 &&
                apeTag.positionSource=="original_plc_exact_tag",
                "Lua monkey original tag_name was not resolved using actual PLC fields");
        require(std::abs(apeTag.position[0]+2.f)<1e-5f &&
                std::abs(apeTag.position[2]+4.f)<1e-5f,
                "original AE3 tag spawn coordinates were not correctly converted to host LH");
        require(player.entityClass=="Player" && ape.entityClass=="Monkey",
                "original Lua entity identity lost");
        const auto& vertices=world.scene().triangles;
        require(std::abs(vertices[player.firstVertex].x-1)<1e-5 &&
                std::abs(vertices[player.firstVertex].z+2)<1e-5,
                "Lua player source coordinates were not converted to native LH correctly");
        require(std::abs(vertices[ape.firstVertex].x-3)<1e-5 &&
                std::abs(vertices[ape.firstVertex].y-1)<1e-5 &&
                std::abs(vertices[ape.firstVertex].z+3)<1e-5,
                "independent Lua monkey positions were not applied to their own meshes");
        require(world.moveActor("player",5,2,-6,0),
                "native movement did not locate Lua spawned player");
        require(std::abs(world.scene().triangles[player.firstVertex].x-6)<1e-5,
                "native walking input did not move actual player model");
        require(std::abs(world.scene().triangles[ape.firstVertex].x-3)<1e-5,
                "moving Lua player incorrectly moved unrelated monkey");
        require(!world.moveActor("missing",0,0,0,0),
                "native scene incorrectly moved an unknown actor");
        require(!world.setActorClip("ape_01",0),
                "actor without original animation incorrectly accepted a clip");
        world.tick(.016);
        const std::string bad=R"LUA(
            function setupArea(opts)
              new_ent("Monkey",{name="ape_01"})
            end
        )LUA";
        ae3::NativeStageWorld second;
        ae3::LuaStage vm2(second);
        vm2.executeSource(bad);vm2.callSetupArea("s");
        {
            std::ofstream file(root/"bad_bindings.txt");
            file<<"class:Monkey=../escape.glb\n";
        }
        world.bind(second,root,root/"bad_bindings.txt");
        require(world.report().actorsCreated==0 &&
                world.report().modelsRejected==1,
                "private actor override could escape model-root security scope");
        // Native Lua model_name must resolve the actual original model
        // basename despite PCK's collision-safe numbered extraction name.
        fs::create_directories(root/"pck_assets"/"bg");
        write(root/"pck_assets"/"bg"/"0000_woods_actor.glb",triangle());
        ae3::NativeStageWorld originalActor;
        ae3::LuaStage originalVm(originalActor);
        originalVm.executeSource(R"LUA(
            function setupArea(opts)
              new_ent("Monkey",{name="original_monkey",model_name="woods_actor",pos={1,0,2}})
            end
        )LUA");
        originalVm.callSetupArea("start");
        ae3::NativeActorWorld extractedWorld(backdrop);
        extractedWorld.bind(originalActor,root);
        require(extractedWorld.report().actorsCreated==1 &&
                extractedWorld.actor("original_monkey")->modelName=="0000_woods_actor.glb",
                "retail Lua model_name failed to bind uniquely to numbered PCK-extracted I3D GLB");
        fs::remove_all(root);
    }catch(...){fs::remove_all(root);throw;}
}
}
int main(){
    try{
        integration();
        std::cout<<"NATIVE_LUA_ACTOR_WORLD_PASSED actors=3 monkey_instances=2 original_plc_tags=1 "
                    "gpu_batches=4 player_movement=1 private_asset_guards=1\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"NATIVE_LUA_ACTOR_WORLD_FAILED: "<<e.what()<<"\n";
        return 1;
    }
}
