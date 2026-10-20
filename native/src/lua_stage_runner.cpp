#include "ae3/lua_stage.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

int main(int argc,char** argv){
    if(argc<2 || argc>3){
        std::cerr<<"Usage: Getchu3LuaStageRunner <original-area.luc|recovered-area.lua> [spawn_tag]\n"
                 <<"Runs Lua 5.0 stage setupArea through native C++ bindings. "
                   "Does not run the PS2 game and is not yet playable.\n";
        return 2;
    }
    try {
        const std::filesystem::path path=std::filesystem::u8path(argv[1]);
        if(path.extension()!=".lua" && path.extension()!=".luc")
            throw std::runtime_error("expected original owned area.luc or recovered .lua file");
        std::ifstream stream(path,std::ios::binary);
        if(!stream)throw std::runtime_error("could not open local original AE3 stage script");
        std::string code((std::istreambuf_iterator<char>(stream)),
                          std::istreambuf_iterator<char>());
        ae3::NativeStageWorld world;
        ae3::LuaStage lua(world);
        lua.executeSource(code,path.filename().string());
        lua.callSetupArea(argc==3?argv[2]:"spawn_default");
        std::cout<<"NATIVE_LUA50_STAGE_EXECUTED source="<<path.filename().string()<<"\n"
                 <<"Native entities declared: "<<world.entities().size()<<"\n"
                 <<"Lua parameter declarations: "<<world.parameters().size()<<"\n"
                 <<"Original retail Lua 5.0 code executed on native host CPU, NO PS2 hardware.\n";
        for(const auto& entity:world.entities()){
            std::cout<<entity.entityClass<<"\t"<<entity.name<<"\t"<<entity.tagName<<"\n";
        }
        std::cout<<"Native entity descriptors only: real AI, world rendering, "
                   "collision and animations are not implemented yet.\n";
        return 0;
    }catch(const std::exception& error){
        std::cerr<<"NATIVE_LUA50_STAGE_FAILED: "<<error.what()<<"\n";
        return 1;
    }
}
