#include "ae3/actor_world.hpp"
#include "ae3/level_scene.hpp"
#include "ae3/lua_stage.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
int main(int argc,char** argv){
    if(argc!=2 && argc!=3){
        std::cerr<<"Usage: Getchu3ActorWorldProbe <private-stage-GLB-folder> [private-area.lua]\n";
        return 2;
    }
    try{
        namespace fs=std::filesystem;
        const fs::path root=fs::u8path(argv[1]);
        const fs::path script=argc==3?fs::u8path(argv[2]):root/"area.lua";
        std::ifstream source(script,std::ios::binary);
        if(!source)throw std::runtime_error("no owner-local Lua stage source");
        std::string text((std::istreambuf_iterator<char>(source)),{});
        ae3::NativeStageWorld luaWorld;
        ae3::LuaStage lua(luaWorld);
        lua.executeSource(text,script.filename().string());
        lua.callSetupArea("spawn_default");
        const fs::path plc=root/"placeholder.plc";
        const auto backdrop=ae3::loadNativeLevelFolder(
            root,fs::is_regular_file(plc)?plc:fs::path{});
        ae3::NativeActorWorld world(backdrop.scene);
        const fs::path bindings=root/"actor_bindings.txt";
        world.bind(luaWorld,root,fs::is_regular_file(bindings)?bindings:fs::path{});
        world.tick(.25);
        const auto& report=world.report();
        std::cout<<"NATIVE_LUA_MULTIPLE_ACTORS_BOUND\n"
                 <<"Original Lua new_ent descriptors: "<<report.stageEntities<<"\n"
                 <<"Actual native 3D scene actors: "<<report.actorsCreated<<"\n"
                 <<"Independent skeleton animators: "<<report.animatedActors<<"\n"
                 <<"Unbound script entities: "<<report.unboundEntities<<"\n"
                 <<"Unresolved/invalid actor models: "<<report.modelsRejected<<"\n"
                 <<"Exact original PLC actor spawn matches: "<<report.matchedOriginalPlcTags<<"\n"
                 <<"Unverified actor positions: "<<report.unresolvedSpawnPositions<<"\n"
                 <<"Original background triangles: "<<backdrop.scene.triangles.size()/3<<"\n"
                 <<"Combined native world triangles: "<<world.scene().triangles.size()/3<<"\n";
        for(const auto& actor:world.actors())
            std::cout<<actor.entityClass<<" "<<actor.name<<" "<<actor.modelName
                     <<" position "<<actor.position[0]<<","
                     <<actor.position[1]<<","<<actor.position[2]
                     <<" animated="<<actor.hasAnimation
                     <<" source="<<actor.positionSource<<"\n";
        for(const auto& note:report.diagnostics)
            std::cerr<<"Binding diagnostic: "<<note<<"\n";
        return report.actorsCreated?0:1;
    }catch(const std::exception& e){
        std::cerr<<"NATIVE_LUA_ACTOR_BINDING_ERROR: "<<e.what()<<"\n";
        return 1;
    }
}
