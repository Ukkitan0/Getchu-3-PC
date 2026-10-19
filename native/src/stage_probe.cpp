#include "ae3/stage_scripts.hpp"
#include <filesystem>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
namespace fs=std::filesystem;

int main(int argc,char** argv) {
    if(argc<2) {
        std::cerr<<"Usage: Getchu3StageProbe <owned ISO|DATA.BIN|extracted-folder>\n"
                 <<"Identifies original game Lua 5.0 area.luc stage scripts; DOES NOT "
                   "execute console code or load PS2 hardware.\n";
        return 2;
    }
    try {
        fs::path source=fs::u8path(argv[1]);
        std::vector<ae3::ScriptDescriptor> found;
        if(fs::is_directory(source)) {
            found=ae3::discoverLooseScripts(source);
        } else {
            ae3::Archive game(source);
            found=ae3::discoverArchiveScripts(game);
        }
        std::map<std::string,std::size_t> counts;
        std::size_t levels=0;
        for(const auto& script:found) {
            ++counts[ae3::scriptKindName(script.kind)];
            if(script.areaScript)++levels;
        }
        std::cout<<"Getchu 3 native stage analysis (PS2 emulation: NONE)\n"
                 <<"Script files identified: "<<found.size()<<"\n"
                 <<"Area scripts identified: "<<levels<<"\n";
        for(const auto& [kind,count]:counts)std::cout<<kind<<": "<<count<<"\n";
        for(const auto& script:found) {
            std::cout<<script.path<<"\t"
                     <<ae3::scriptKindName(script.kind)<<"\t"
                     <<script.bytes<<" bytes\t"
                     <<(script.areaScript?"AREA":"other")<<"\n";
        }
        if(levels==0) {
            std::cout<<"No area.luc found at this archive layer. Some AE3 scripts "
                        "are nested inside .sz/.pck; extract the owned DATA.BIN "
                        "with Saru3VfiTool and pass its extracted folder.\n";
        }
        std::cout<<"Detection does not imply scripts are executable by native game engine.\n";
        return 0;
    }catch(const std::exception& error) {
        std::cerr<<"Native stage discovery failed: "<<error.what()<<"\n";
        return 1;
    }
}
