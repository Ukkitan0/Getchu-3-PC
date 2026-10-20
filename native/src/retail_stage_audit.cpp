#include "ae3/archive.hpp"
#include "ae3/lua_stage.hpp"
#include "ae3/retail_lua50.hpp"
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
bool safeStage(const std::string& value){
    if(value.empty()||value.size()>70)return false;
    for(unsigned char c:value)
        if(!std::isalnum(c)&&c!='_')return false;
    return true;
}
}
int main(int argc,char** argv){
    if(argc!=3 && argc!=4){
        std::cerr<<"Usage: Getchu3RetailStageAudit <your-own-Ape-Escape-3-USA.iso> "
                   "<real-stage-id, e.g. seaside_a> [original-spawn-tag]\n";
        return 2;
    }
    try{
        const std::string stageId=argv[2];
        if(!safeStage(stageId))throw std::runtime_error("invalid original game stage name");
        ae3::Archive iso(std::filesystem::u8path(argv[1]));
        const std::string entry="debug/us/stage/"+stageId+"/area.luc";
        const auto index=iso.find(entry);
        if(!index)throw std::runtime_error("owned original ISO has no real stage "+entry);
        const auto& source=iso.entries()[*index];
        const auto blob=iso.read(source);
        const std::string_view original(reinterpret_cast<const char*>(blob.data()),blob.size());
        ae3::RetailLua50Stats stats{};
        auto host=ae3::retargetRetailLua50(original,&stats);
        std::cout<<"ORIGINAL_GAME_STAGE_LUA50: "<<entry<<"\n"
                 <<"Authentic original script bytes: "<<blob.size()<<"\n"
                 <<"Native host-representable bytecode bytes: "<<host.size()<<"\n"
                 <<"Unchanged game Lua opcodes: "<<stats.instructions<<"\n"
                 <<"Original game Lua functions: "<<stats.prototypes<<"\n"
                 <<"Original game Lua constants: "<<stats.constants<<"\n"
                 <<"No PS2 console CPU, BIOS, IOP or graphics simulation used.\n";
        ae3::NativeStageWorld world;
        // Original game's save Data layout, blank new-game inventory/capture
        // state only. This is not a decoded user save game.
        ae3::PlayerProgress newGame{};
        ae3::LuaStage game(world,&newGame);
        game.executeSource(original,entry);
        std::cout<<"ORIGINAL_GAME_LUA_CHUNK_EXECUTED: true\n";
        const std::string spawn=argc==4?argv[3]:"spawn_default";
        try{
            game.callSetupArea(spawn);
        }catch(const std::exception& e){
            // Do not silently stub missing game functions. The exception
            // identifies the next original native engine binding to
            // reconstruct from SCUS_975.01 disassembly and real Lua calls.
            std::cerr<<"ORIGINAL_GAME_SETUPAREA_BLOCKED: "<<e.what()<<"\n";
            std::cerr<<"Stage is NOT PLAYABLE; this shows a real missing "
                        "engine feature, not a fabricated successful scene.\n";
            return 3;
        }
        std::cout<<"ORIGINAL_GAME_SETUPAREA_COMPLETED: true\n"
                 <<"Original native Player/Monkey/other entity descriptors: "
                 <<world.entities().size()<<"\n"
                 <<"Original Lua parameters: "<<world.parameters().size()<<"\n";
        for(const auto& e:world.entities())
            std::cout<<"  "<<e.entityClass<<": "<<e.name
                     <<" spawn tag "<<e.tagName<<"\n";
        // Passing this audit indicates only the authentic Lua setup
        // executed on native Lua 5.0, NOT fully playable original gameplay.
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"ORIGINAL_GAME_SCRIPT_FAILED: "<<e.what()<<"\n";
        return 1;
    }
}
