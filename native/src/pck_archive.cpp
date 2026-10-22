#include "ae3/pck_archive.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace ae3 {
namespace {
constexpr std::uint64_t maxPckBytes=512ull*1024*1024;
std::uint32_t word(const std::vector<std::uint8_t>& data,std::size_t at){
    if(at>data.size()||data.size()-at<4)throw std::runtime_error("PCK truncated DWORD");
    return std::uint32_t(data[at]) | (std::uint32_t(data[at+1])<<8) |
           (std::uint32_t(data[at+2])<<16) | (std::uint32_t(data[at+3])<<24);
}
bool inside(std::uint64_t at,std::uint64_t len,std::uint64_t total){
    return at<=total && len<=total-at;
}
// Private authentic USA woods_c/{bg,character,exdb,navigation,apelodtex}.pck
// records independently confirm name/type are ABSOLUTE source-file
// string offsets, not pool-relative identifiers.
std::string nameAt(const std::vector<std::uint8_t>& data,
                   std::size_t table,std::uint32_t absolute){
    constexpr std::size_t pool=16;
    if(table<pool || absolute<pool || absolute>=table)
        throw std::runtime_error("original PCK string offset outside absolute pool");
    const std::size_t at=absolute;
    if(at>pool && data[at-1]!=0)
        throw std::runtime_error("original PCK offset does not point to a source string start");
    std::size_t end=at;
    while(end<table && data[end]!=0) {
        if(data[end]<32||data[end]>126)
            throw std::runtime_error("original PCK has non-printable source identifier");
        if(end-at>=255)
            throw std::runtime_error("original PCK source string exceeds bounded identifier length");
        ++end;
    }
    if(end==table || end==at)
        throw std::runtime_error("original PCK source string missing or unterminated");
    return std::string(reinterpret_cast<const char*>(data.data()+at),end-at);
}
PckAssetKind detect(const std::vector<std::uint8_t>& b,std::size_t p,std::size_t n){
    if(n>=8 && std::memcmp(b.data()+p,"I3D_BIN",7)==0)return PckAssetKind::mesh;
    if(n>=8 && std::memcmp(b.data()+p,"I3D_I3M",8)==0)return PckAssetKind::animation;
    if(n>=8 && std::memcmp(b.data()+p,"I3D_I3C",8)==0)return PckAssetKind::collision;
    if(n>=4 && std::memcmp(b.data()+p,"TIM2",4)==0)return PckAssetKind::texture;
    return PckAssetKind::other;
}
std::string cleanName(std::string name){
    // Directly store names only as diagnostics: no asset may escape the
    // destination directory when we extract locally.
    if(name.empty())return "unnamed";
    std::replace_if(name.begin(),name.end(),[](unsigned char c){
        return !std::isalnum(c) && c!='_' && c!='-';
    },'_');
    if(name.size()>80)name.resize(80);
    return name;
}
} // namespace
const char* pckExtension(PckAssetKind kind){
    switch(kind){
    case PckAssetKind::mesh:return ".i3d";
    case PckAssetKind::animation:return ".i3m";
    case PckAssetKind::collision:return ".i3c";
    case PckAssetKind::texture:return ".tm2";
    default:return ".bin";
    }
}
PckArchive::PckArchive(const std::filesystem::path& path){
    std::error_code error;
    const auto count=std::filesystem::file_size(path,error);
    if(error||count<16||count>maxPckBytes)
        throw std::runtime_error("PCK source file missing, truncated or oversized");
    std::ifstream stream(path,std::ios::binary);
    if(!stream)throw std::runtime_error("cannot open original PCK");
    bytes_.resize(static_cast<std::size_t>(count));
    if(!stream.read(reinterpret_cast<char*>(bytes_.data()),
                    static_cast<std::streamsize>(bytes_.size())))
        throw std::runtime_error("short original PCK file read");
    parse();
}
PckArchive::PckArchive(std::vector<std::uint8_t> data):bytes_(std::move(data)){parse();}
void PckArchive::parse(){
    if(bytes_.size()<16 || bytes_.size()>maxPckBytes ||
       std::memcmp(bytes_.data(),"PCK\0",4)!=0)
        throw std::runtime_error("not a valid Ape Escape 3 PCK container");
    const std::uint64_t table=word(bytes_,4);
    const std::uint64_t count=word(bytes_,8);
    if(table<16||table>bytes_.size()||count>100000 ||
       !inside(table,count*16,bytes_.size()))
        throw std::runtime_error("corrupted PCK string pool or entry table");
    members_.reserve(static_cast<std::size_t>(count));
    for(std::uint64_t i=0;i<count;++i){
        const auto at=static_cast<std::size_t>(table+i*16);
        const auto nameId=word(bytes_,at);
        const auto type=word(bytes_,at+4);
        const auto payload=word(bytes_,at+8);
        const auto size=word(bytes_,at+12);
        if(!inside(payload,size,bytes_.size()))
            throw std::runtime_error("PCK member exceeds original source bounds");
        if(payload<table+count*16)
            throw std::runtime_error("PCK member overlaps entry/name table");
        members_.push_back({static_cast<std::size_t>(i),
                            nameAt(bytes_,static_cast<std::size_t>(table),nameId),
                            type,payload,size,
                            detect(bytes_,payload,size),
                            nameAt(bytes_,static_cast<std::size_t>(table),type)});
    }
}
std::vector<std::uint8_t> PckArchive::read(const PckMember& entry) const{
    if(entry.index>=members_.size() ||
       entry.offset!=members_[entry.index].offset ||
       entry.size!=members_[entry.index].size)
        throw std::runtime_error("PCK member does not match parsed archive");
    return {bytes_.begin()+entry.offset,bytes_.begin()+entry.offset+entry.size};
}
void PckArchive::extractTo(const std::filesystem::path& folder) const{
    if(folder.empty())throw std::runtime_error("PCK extraction destination missing");
    std::error_code error;
    if(std::filesystem::exists(folder,error)){
        if(error || !std::filesystem::is_directory(folder,error) ||
           !std::filesystem::is_empty(folder,error))
            throw std::runtime_error("PCK output already exists and is not empty");
    }else std::filesystem::create_directories(folder);
    for(const auto& entry:members_){
        std::string ordinal=std::to_string(entry.index);
        while(ordinal.size()<4)ordinal="0"+ordinal;
        const auto name=ordinal+"_"+cleanName(entry.name)+pckExtension(entry.kind);
        std::ofstream out(folder/name,std::ios::binary|std::ios::trunc);
        if(!out)throw std::runtime_error("PCK asset output file creation failed");
        if(entry.size){
            out.write(reinterpret_cast<const char*>(bytes_.data()+entry.offset),
                      static_cast<std::streamsize>(entry.size));
        }
        if(!out)throw std::runtime_error("PCK original asset output write failed");
    }
}
} // namespace ae3
