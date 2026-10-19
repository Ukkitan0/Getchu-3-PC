#include "ae3/stage_scripts.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
namespace fs=std::filesystem;
namespace {
void require(bool ok,const char* description){
    if(!ok)throw std::runtime_error(description);
}
void write(const fs::path& file,const std::vector<std::uint8_t>& bytes){
    fs::create_directories(file.parent_path());
    std::ofstream out(file,std::ios::binary);
    if(!out)throw std::runtime_error("stage fixture output unavailable");
    out.write(reinterpret_cast<const char*>(bytes.data()),
              static_cast<std::streamsize>(bytes.size()));
    if(!out)throw std::runtime_error("stage fixture output truncated");
}
}
int main(){
    try{
        const std::vector<std::uint8_t> lua50{0x1b,'L','u','a',0x50,0,1,4,4,4,8,0};
        const std::vector<std::uint8_t> lua51{0x1b,'L','u','a',0x51,0};
        const std::vector<std::uint8_t> fake{0x1b,'L','u','X',0x50};
        const auto valid=ae3::inspectScript("debug/us/stage/cty_a/area.luc",lua50);
        require(valid.kind==ae3::ScriptKind::lua50_bytecode,"Lua 5.0 header unrecognized");
        require(valid.areaScript,"real area.luc level name unrecognized");
        require(valid.bytes==lua50.size(),"script byte length changed");
        const auto unsupported=ae3::inspectScript("area.luc",lua51);
        require(unsupported.kind==ae3::ScriptKind::other_lua_bytecode,
                "must not silently accept wrong Lua bytecode version");
        require(unsupported.bytecodeVersion==0x51,"wrong version reported");
        require(ae3::inspectScript("area.luc",fake).kind==ae3::ScriptKind::unsupported,
                "corrupt bytecode header accepted");
        require(ae3::inspectScript("compiled.LUC",lua50).areaScript==false,
                "non-area script incorrectly marked as level");
        require(ae3::inspectScript("area.lua",{'-','-',' ','a','r','e','a'}).kind==
                ae3::ScriptKind::lua_source,"plain Lua source rejected");

        fs::path root=fs::temp_directory_path()/"getchu3_native_stage_fixture";
        fs::create_directories(root);
        try{
            write(root/"stage1"/"area.luc",lua50);
            write(root/"stage2"/"area.luc",lua51);
            write(root/"global"/"main.luc",lua50);
            write(root/"other"/"not_a_script.bin",fake);
            auto scripts=ae3::discoverLooseScripts(root);
            require(scripts.size()==3,"recursive game stage scan missed scripts");
            std::size_t areas=0,lua50count=0,lua51count=0;
            for(const auto& script:scripts){
                areas+=script.areaScript?1:0;
                lua50count+=script.kind==ae3::ScriptKind::lua50_bytecode?1:0;
                lua51count+=script.kind==ae3::ScriptKind::other_lua_bytecode?1:0;
            }
            require(areas==2,"level script area count wrong");
            require(lua50count==2,"Lua 5.0 source classification wrong");
            require(lua51count==1,"Lua 5.1 version mismatch hidden");
        }catch(...){fs::remove_all(root);throw;}
        fs::remove_all(root);
        std::cout<<"NATIVE_LUA50_STAGE_DISCOVERY_OK: header detection, stage traversal, version validation\n";
        return 0;
    }catch(const std::exception& error){
        std::cerr<<"NATIVE_STAGE_TEST_FAILED: "<<error.what()<<"\n";
        return 1;
    }
}
