#include "ae3/stage_scripts.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <system_error>

namespace ae3 {
namespace {
bool suffix(const std::string& s,const char* ext){
    auto n=std::char_traits<char>::length(ext);
    if(s.size()<n)return false;
    for(std::size_t i=0;i<n;++i){
        if(std::tolower(static_cast<unsigned char>(s[s.size()-n+i]))!=
           std::tolower(static_cast<unsigned char>(ext[i])))return false;
    }
    return true;
}
bool areaName(const std::string& name){
    auto slash=name.find_last_of("/\\");
    return suffix(name.substr(slash==std::string::npos?0:slash+1),"area.luc");
}
}
const char* scriptKindName(ScriptKind kind) {
    switch(kind){
    case ScriptKind::lua50_bytecode:return "lua50_bytecode";
    case ScriptKind::other_lua_bytecode:return "incompatible_lua_version";
    case ScriptKind::lua_source:return "lua_source";
    default:return "unsupported";
    }
}
ScriptDescriptor inspectScript(const std::string& name,
                               const std::vector<std::uint8_t>& contents){
    ScriptDescriptor desc{};
    desc.path=name;desc.bytes=contents.size();desc.areaScript=areaName(name);
    if(contents.size()>=5 && contents[0]==0x1b && contents[1]=='L' &&
       contents[2]=='u' && contents[3]=='a'){
        desc.bytecodeVersion=contents[4];
        desc.kind=contents[4]==0x50?ScriptKind::lua50_bytecode:
                                   ScriptKind::other_lua_bytecode;
    } else if(suffix(name,".lua") && !contents.empty() &&
              std::find(contents.begin(),contents.end(),std::uint8_t(0))==contents.end()){
        desc.kind=ScriptKind::lua_source;
    }
    // Merely detecting a Lua signature does NOT mean that it has been
    // decompiled, linked to native game APIs, or executed successfully.
    return desc;
}
std::vector<ScriptDescriptor> discoverArchiveScripts(Archive& game,std::size_t limit){
    std::vector<ScriptDescriptor> found;
    for(const auto& file:game.entries()){
        if(!suffix(file.path,".luc") && !suffix(file.path,".lua"))continue;
        if(file.size>limit){
            ScriptDescriptor desc{};
            desc.path=file.path;desc.bytes=file.size;desc.areaScript=areaName(file.path);
            found.push_back(desc);
            continue;
        }
        found.push_back(inspectScript(file.path,game.read(file)));
    }
    return found;
}
std::vector<ScriptDescriptor> discoverLooseScripts(const std::filesystem::path& folder,
                                                    std::size_t limit){
    if(!std::filesystem::is_directory(folder))
        throw std::runtime_error("no extracted game stage directory");
    std::vector<ScriptDescriptor> found;
    std::error_code error;
    auto root=std::filesystem::weakly_canonical(folder,error);
    if(error)throw std::runtime_error("cannot resolve extracted stage directory");
    for(auto it=std::filesystem::recursive_directory_iterator(
              root,std::filesystem::directory_options::skip_permission_denied);
         it!=std::filesystem::recursive_directory_iterator();++it){
        if(it->is_symlink())continue;
        if(!it->is_regular_file())continue;
        const auto& path=it->path();
        auto name=path.filename().string();
        if(!suffix(name,".luc") && !suffix(name,".lua"))continue;
        auto relative=path.lexically_relative(root).generic_string();
        auto size=it->file_size();
        if(size>limit){
            ScriptDescriptor desc{};
            desc.path=relative;desc.bytes=size;desc.areaScript=areaName(relative);
            found.push_back(std::move(desc));
            continue;
        }
        std::ifstream src(path,std::ios::binary);
        if(!src)throw std::runtime_error("cannot read extracted stage script");
        std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
        if(size && !src.read(reinterpret_cast<char*>(bytes.data()),
                             static_cast<std::streamsize>(size)))
            throw std::runtime_error("short native stage script read");
        found.push_back(inspectScript(relative,bytes));
    }
    std::sort(found.begin(),found.end(),[](const auto& a,const auto& b){
        return a.path<b.path;
    });
    return found;
}
} // namespace ae3
