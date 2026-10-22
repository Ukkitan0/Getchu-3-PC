#include "ae3/original_resource_stage_directory_registration.hpp"
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
        // Explicit owner-selected stage root. Original runtime stage selection
        // and DirInfo allocation remain unrecovered; this is an integration
        // diagnostic, not a substitute for that original engine lifecycle.
        ae3::OriginalStageResourceScope stage(std::filesystem::path(argv[1]),stageId);
        // Name lookup alone is ambiguous: area.eff/area.eft/area.luc
        // share the original name "area". Apply the recovered source type
        // filter BEFORE the original first-name-match lookup.
        ae3::OriginalStageDirectoryRegistration_251958 registry(stage,"","luc");
        const std::string entry="debug/us/stage/"+stageId+"/area.luc";
        const auto* script=registry.originalFind_24d4f0("area");
        if(!script)throw std::runtime_error("Original area resource absent from stage registry");
        // Keep scope and registry alive until AFTER the Lua VM is destroyed.
        // 0x24D858 -> 0x2519FC -> 0x251A30 -> 0x24D4F0 supplies the
        // original payload, never a separately guessed/extracted Lua file.
        const auto blob=script->borrowedOriginalPayload;
        const std::string_view original(reinterpret_cast<const char*>(blob.data()),blob.size());
        ae3::RetailLua50Stats stats{};
        auto host=ae3::retargetRetailLua50(original,&stats);
        std::cout<<"ORIGINAL_STAGE_REGISTRY_LUA_CONNECTED: true\n"
                 <<"Registered original source directory entries: "<<registry.registeredRecords().size()<<"\n"
                 <<"ORIGINAL_GAME_STAGE_LUA50: "<<entry<<"\n"
                 <<"Authentic original script bytes: "<<blob.size()<<"\n"
                 <<"Native host-representable bytecode bytes: "<<host.size()<<"\n"
                 <<"Unchanged game Lua opcodes: "<<stats.instructions<<"\n"
                 <<"Original game Lua functions: "<<stats.prototypes<<"\n"
                 <<"Original game Lua constants: "<<stats.constants<<"\n"
                 <<"No PS2 console CPU, BIOS, IOP or graphics simulation used.\n";
        ae3::NativeStageWorld world;
        // No invented new-game save/progression or actor factory. Original
        // callbacks that need live manager state must report that dependency.
        ae3::LuaStage game(world);
        ae3::NativeLuaStageBindings bindings;
        bindings.createEntity=[](const ae3::StageEntity& entity){
            throw std::runtime_error("Original actor manager unavailable: 0x57BCB8 initialization and Player/Monkey scheduler; requested "+entity.entityClass);
        };
        bindings.submitParameters=[](const ae3::ScriptValue&){
            throw std::runtime_error("Original lua_param consumer and stage-owner lifetime are not recovered");
        };
        bindings.selectWeapon=[](int){
            throw std::runtime_error("Original Player weapon/action owner is not initialized");
        };
        game.setNativeBindings(std::move(bindings));
        game.executeSource(original,entry);
        std::cout<<"ORIGINAL_GAME_LUA_CHUNK_EXECUTED: true\n";
        if(argc!=4){
            std::cout<<"ORIGINAL_GAME_SETUPAREA_NOT_REQUESTED: pass an explicit original spawn tag\n"
                     <<"Original stage selection, actor scheduling and gameplay remain unrecovered.\n";
            return 0;
        }
        const std::string spawn=argv[3];
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
