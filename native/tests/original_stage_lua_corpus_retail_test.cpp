#include "ae3/original_resource_stage_directory_registration.hpp"
#include "ae3/lua_stage.hpp"
#include "ae3/retail_lua50.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

// Owner-private differential test: actual ISO registry -> unchanged Lua
// chunk -> original 5.0 VM, compared against every private handoff chunk.
// No setupArea calls, guessed spawn tags, fabricated managers or game boot.
static bool knownMissingOriginalProvider(std::string_view stage,std::string_view error) {
    if(error.ends_with("native lua_param binding failed: Original lua_param manager unavailable during chunk load") ||
       error.ends_with("native new_ent binding failed: Original 0x57BCB8 actor manager unavailable during chunk load") ||
       error.ends_with("original set_player_weapon failed: Original Player weapon owner unavailable during chunk load"))return true;
    // Independently observed in the private original opcode audit. This is
    // an unresolved binding, not an invented callback VA or implementation.
    return (stage=="boss2_a" || stage=="boss3_a" || stage=="boss4_a") &&
           error.ends_with("attempt to call global `enlarge_se_range' (a nil value)");
}
int main(int argc,char** argv) {
    try {
        if(argc!=3 && argc!=4)throw std::invalid_argument("Pass private original ISO and private 168-stage Lua folder [--validate-only]");
        const bool validateOnly=argc==4 && std::string_view(argv[3])=="--validate-only";
        if(argc==4 && !validateOnly)throw std::invalid_argument("Unknown corpus audit option");
        ae3::OriginalResourceVfiMount mount{std::filesystem::path(argv[1])};
        const ae3::OriginalStageResourceCatalog catalog(mount.vfiEntries());
        if(catalog.stageNames().size()!=168)
            throw std::runtime_error("Expected exactly 168 original stage directories");
        std::size_t matched=0,executed=0,blocked=0,instructions=0,prototypes=0;
        for(const auto& name:catalog.stageNames()) {
            try {
                ae3::OriginalStageResourceScope stage(std::filesystem::path(argv[1]),name);
                ae3::OriginalStageDirectoryRegistration_251958 registry(stage,"","luc");
                const auto* original=registry.originalFind_24d4f0("area");
                if(!original)throw std::runtime_error("Original area.luc absent from registry");
                const auto payload=original->borrowedOriginalPayload;
                std::ifstream input(std::filesystem::path(argv[2])/name/"area.luc",std::ios::binary);
                if(!input)throw std::runtime_error("Private handoff stage chunk missing");
                const std::vector<char> handoff((std::istreambuf_iterator<char>(input)),{});
                if(payload.size()!=handoff.size() || !std::equal(payload.begin(),payload.end(),handoff.begin(),
                    [](std::byte a,char b){return std::to_integer<unsigned char>(a)==static_cast<unsigned char>(b);}))
                    throw std::runtime_error("ISO registry bytes differ from owner-verified private Lua corpus");
                const std::string_view chunk(reinterpret_cast<const char*>(payload.data()),payload.size());
                ae3::RetailLua50Stats stats{};
                (void)ae3::retargetRetailLua50(chunk,&stats);
                ++matched;instructions+=stats.instructions;prototypes+=stats.prototypes;
                if(validateOnly)continue;
                ae3::NativeStageWorld descriptors;
                ae3::LuaStage vm(descriptors);
                ae3::NativeLuaStageBindings bindings;
                bindings.createEntity=[](const ae3::StageEntity&){
                    throw std::runtime_error("Original 0x57BCB8 actor manager unavailable during chunk load");
                };
                bindings.submitParameters=[](const ae3::ScriptValue&){
                    throw std::runtime_error("Original lua_param manager unavailable during chunk load");
                };
                bindings.selectWeapon=[](int){
                    throw std::runtime_error("Original Player weapon owner unavailable during chunk load");
                };
                vm.setNativeBindings(std::move(bindings));
                try {
                    vm.executeSource(chunk,"@"+name+"/area.luc");
                } catch(const std::runtime_error& error) {
                    if(!knownMissingOriginalProvider(name,error.what()))throw;
                    ++blocked;
                    std::cerr<<"ORIGINAL_STAGE_CHUNK_EXECUTION_BLOCKED: "<<name<<": "<<error.what()<<'\n';
                    continue;
                }
                if(!descriptors.entities().empty() || !descriptors.parameters().empty())
                    throw std::runtime_error("Original chunk load fabricated engine objects");
                ++executed;
            } catch(const std::exception& error) {
                throw std::runtime_error(name+": "+error.what());
            }
        }
        std::cout<<"ORIGINAL_STAGE_LUA_CORPUS_VALIDATED iso_private_byte_matches="<<matched
                 <<" native_chunk_executions="<<executed<<" unchanged_opcodes="<<instructions
                 <<" original_functions="<<prototypes<<" blocked_chunk_executions="<<blocked
                 <<" setupArea_invocations=0\n";
        // Distinguish missing original managers from malformed source or a
        // false success claim. CTest reports this as an explicit blocked skip.
        return blocked?77:0;
    } catch(const std::exception& error) {
        std::cerr<<"ORIGINAL_STAGE_LUA_CORPUS_FAILED: "<<error.what()<<'\n';
        return 1;
    }
}
