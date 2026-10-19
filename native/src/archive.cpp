#include "ae3/archive.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace ae3 {
namespace {
std::uint16_t u16(const std::vector<std::uint8_t>& b, std::size_t pos) {
    if (pos > b.size() || b.size()-pos < 2) throw std::runtime_error("truncated u16");
    return std::uint16_t(b[pos]) | (std::uint16_t(b[pos+1]) << 8);
}
std::uint32_t u32(const std::vector<std::uint8_t>& b, std::size_t pos) {
    if (pos > b.size() || b.size()-pos < 4) throw std::runtime_error("truncated u32");
    return std::uint32_t(u16(b,pos)) | (std::uint32_t(u16(b,pos+2)) << 16);
}
bool bounded(std::uint64_t start, std::uint64_t count, std::uint64_t total) {
    return start <= total && count <= total-start;
}
std::string cstr(const std::vector<std::uint8_t>& b, std::size_t start, std::size_t count) {
    if (start > b.size() || count > b.size()-start) throw std::runtime_error("bad name bounds");
    std::size_t end = start;
    while (end < start + count && b[end] != 0) ++end;
    return std::string(reinterpret_cast<const char*>(b.data()+start),end-start);
}
std::string normalized(std::string v) {
    std::replace(v.begin(),v.end(),'\\','/');
    if (v.empty() || v.front()=='/' || v.find(':') != std::string::npos ||
        v.find("..") != std::string::npos || v.find('\0') != std::string::npos)
        throw std::runtime_error("unsafe VFI path");
    for (unsigned char c:v) if (c < 32 || c==127) throw std::runtime_error("invalid VFI path byte");
    return v;
}
std::string upper(std::string a) {
    std::transform(a.begin(),a.end(),a.begin(),[](unsigned char c){return char(std::toupper(c));});
    return a;
}
constexpr std::uint64_t sector = 2048;
struct IsoRecord {
    std::string name;
    std::uint64_t start = 0, size = 0;
    bool folder = false;
};
IsoRecord isoRecord(const std::vector<std::uint8_t>& directory,std::size_t at) {
    if (at >= directory.size() || directory[at] < 34 || at + directory[at] > directory.size())
        throw std::runtime_error("malformed ISO9660 directory record");
    std::uint8_t n = directory[at+32];
    if (33u+n > directory[at]) throw std::runtime_error("truncated ISO name");
    IsoRecord item;
    item.name = std::string(reinterpret_cast<const char*>(directory.data()+at+33),n);
    item.start = std::uint64_t(u32(directory,at+2)) * sector;
    item.size = u32(directory,at+10);
    item.folder = (directory[at+25] & 2) != 0;
    return item;
}
Span findDataBin(DiskFile& input) {
    if (input.size() < 17*sector) throw std::runtime_error("ISO too small for volume descriptor");
    auto volume = input.read(16*sector,sector);
    if (volume[0]!=1 || std::memcmp(volume.data()+1,"CD001",5)!=0 || volume[6]!=1)
        throw std::runtime_error("not an ISO9660 primary volume descriptor");
    auto root = isoRecord(volume,156);
    if (!root.folder || !bounded(root.start,root.size,input.size()) || root.size>32*1024*1024)
        throw std::runtime_error("bad ISO root directory");
    auto contents=input.read(root.start,static_cast<std::size_t>(root.size));
    std::size_t off=0;
    while (off < contents.size()) {
        if (contents[off]==0) {
            off=((off/sector)+1)*sector;
            continue;
        }
        auto entry=isoRecord(contents,off);
        if (!entry.folder && upper(entry.name)=="DATA.BIN;1") {
            if(!bounded(entry.start,entry.size,input.size()))throw std::runtime_error("ISO DATA.BIN exceeds disc bounds");
            return {entry.start,entry.size};
        }
        off += contents[off];
    }
    throw std::runtime_error("ISO contains no root DATA.BIN;1 (expected Ape Escape 3 disc)");
}
struct Folder { std::string name; std::uint16_t parent=0; };
} // namespace

DiskFile::DiskFile(const std::filesystem::path& file):file_(file,std::ios::binary) {
    if (!file_)throw std::runtime_error("cannot open game data file");
    file_.seekg(0,std::ios::end);
    auto n=file_.tellg();
    if(n<0)throw std::runtime_error("cannot stat game data");
    length_=static_cast<std::uint64_t>(n);
}
std::vector<std::uint8_t> DiskFile::read(std::uint64_t start,std::size_t bytes) {
    if(!bounded(start,bytes,length_))throw std::runtime_error("file read exceeds bounds");
    if(start>static_cast<std::uint64_t>(std::numeric_limits<std::streamoff>::max()))
        throw std::runtime_error("file offset exceeds stream type");
    file_.clear();
    file_.seekg(static_cast<std::streamoff>(start),std::ios::beg);
    std::vector<std::uint8_t> buffer(bytes);
    if(bytes && !file_.read(reinterpret_cast<char*>(buffer.data()),static_cast<std::streamsize>(bytes)))
        throw std::runtime_error("short host file read");
    return buffer;
}
Archive::Archive(const std::filesystem::path& ownedGameData):
    input_(ownedGameData),source_(ownedGameData) {
    if(input_.size() < 32)throw std::runtime_error("game data input too small");
    auto magic=input_.read(0,4);
    if(magic[0]=='V'&&magic[1]=='F'&&magic[2]=='I'&&magic[3]==0) {
        kind_="native VFI DATA.BIN";
        data_={0,input_.size()};
    }else {
        kind_="ISO9660 direct archive mounting";
        data_=findDataBin(input_);
    }
    auto hdr=input_.read(data_.offset,32);
    if(u32(hdr,0)!=0x00494656)throw std::runtime_error("DATA.BIN is not VFI");
    const std::uint64_t dataStart=std::uint64_t(u32(hdr,8))*sector;
    const auto files=u16(hdr,16),folders=u16(hdr,18);
    const auto infoOff=u32(hdr,20),folderOff=u32(hdr,24),infoEnd=u32(hdr,28);
    if (infoOff<32 || folderOff<infoOff || infoEnd<folderOff ||
        dataStart>data_.size || infoEnd>dataStart || infoEnd>32*1024*1024)
        throw std::runtime_error("malformed VFI table bounds");
    const auto table=input_.read(data_.offset,infoEnd);
    struct Raw { std::string name;std::uint16_t parent;std::uint64_t off;std::uint32_t size; };
    std::vector<Raw> raw;
    std::size_t p=infoOff;
    for(unsigned i=0;i<files;++i){
        if(p+12>folderOff)throw std::runtime_error("truncated VFI file table");
        auto n=u16(table,p),parent=u16(table,p+2);
        if(n<13||p+n>folderOff)throw std::runtime_error("invalid VFI file record");
        Raw entry{normalized(cstr(table,p+12,n-12)),parent,std::uint64_t(u32(table,p+4))*sector,u32(table,p+8)};
        if(!bounded(entry.off,entry.size,data_.size))throw std::runtime_error("VFI member exceeds DATA.BIN");
        if(entry.off<dataStart)throw std::runtime_error("VFI member overlaps its index");
        raw.push_back(std::move(entry));
        p+=n;
    }
    std::unordered_map<std::uint16_t,Folder> dirs;
    p=folderOff;
    for(unsigned i=0;i<folders;++i) {
        if(p+8>infoEnd)throw std::runtime_error("truncated VFI folder table");
        std::size_t relative=p-folderOff;
        if(relative>65535)throw std::runtime_error("VFI folder table overflows parent offset");
        auto n=u16(table,p);
        if(n<9||p+n>infoEnd)throw std::runtime_error("invalid VFI folder record");
        dirs.emplace(static_cast<std::uint16_t>(relative),
                     Folder{normalized(cstr(table,p+8,n-8)),u16(table,p+4)});
        p+=n;
    }
    entries_.reserve(raw.size());
    for(const auto& file:raw){
        std::string name=file.name;
        std::uint16_t parent=file.parent;
        std::unordered_set<std::uint16_t> seen;
        for(unsigned depth=0;parent && depth<64;++depth){
            if(!seen.insert(parent).second)throw std::runtime_error("cyclic VFI folder chain");
            auto it=dirs.find(parent);
            if(it==dirs.end())throw std::runtime_error("missing VFI parent folder");
            name=it->second.name + "/" + name;
            parent=it->second.parent;
        }
        if(parent)throw std::runtime_error("VFI folder chain too deep");
        entries_.push_back({normalized(name),file.off,file.size});
    }
}
std::vector<std::uint8_t> Archive::read(const VfiEntry& item) {
    if(item.size > 128*1024*1024)throw std::runtime_error("asset exceeds per-read budget");
    if(!bounded(item.offset,item.size,data_.size))throw std::runtime_error("asset outside DATA.BIN");
    return input_.read(data_.offset+item.offset,item.size);
}
std::optional<std::size_t> Archive::find(const std::string& path) const {
    auto requested=upper(path);
    for(std::size_t i=0;i<entries_.size();++i)
        if(upper(entries_[i].path)==requested)return i;
    return {};
}
std::string assetType(const std::string& name){
    auto dot=name.rfind('.');
    return dot==std::string::npos?"unknown":upper(name.substr(dot+1));
}
} // namespace ae3
