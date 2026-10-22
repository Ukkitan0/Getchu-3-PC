#pragma once
#include "ae3/archive.hpp"
#include "ae3/original_resource_vfi_mount.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace ae3 {
// Host inventory of names ACTUALLY present beneath retail USA
// debug/us/stage/<stage>/, as evidenced by the retail ISO's VFI index
// and the recovered owned-stage path in app_win32.cpp.
// Enumeration is not a claim about the game's original load order.
inline constexpr std::string_view OriginalUsStageVfiPrefix="debug/us/stage/";
inline std::string originalResourceAsciiFold(std::string_view input) {
    std::string output(input);
    for(char& value:output)
        if(value>='A' && value<='Z')value=static_cast<char>(value-'A'+'a');
    return output;
}
inline bool originalResourceIsSz(std::string_view name) noexcept {
    if(name.size()<3)return false;
    const auto suffix=name.substr(name.size()-3);
    return suffix==".sz"||suffix==".SZ"||suffix==".sZ"||suffix==".Sz";
}
inline bool originalResourceValidStageName(std::string_view name) noexcept {
    if(name.empty() || name.size()>70)return false;
    for(unsigned char c:name)
        if(!((c>='A'&&c<='Z')||(c>='a'&&c<='z')||
             (c>='0'&&c<='9')||c=='_'))return false;
    return true;
}
inline bool originalResourceValidStageRelativeName(std::string_view path) noexcept {
    if(path.empty()||path.front()=='/'||path.back()=='/')return false;
    std::size_t pos=0;
    while(pos<path.size()){
        const auto slash=path.find('/',pos);
        const auto end=slash==std::string_view::npos?path.size():slash;
        const auto part=path.substr(pos,end-pos);
        if(part.empty()||part=="."||part=="..")return false;
        for(unsigned char c:part)
            if(c<' '||c==127||c==':'||c=='\\')return false;
        pos=end+1;
    }
    return true;
}
struct OriginalStageVfiResource {
    VfiEntry sourceEntry;             // original DATA.BIN-relative VFI offset and size
    std::string stageName;           // original stage directory
    std::string stageRelativePath;   // EXACT original VFI member name
    std::string uncompressedName;    // strips source '.sz' suffix only
    bool originalSzCompressed=false;
};
class OriginalStageResourceCatalog final {
public:
    explicit OriginalStageResourceCatalog(std::span<const VfiEntry> originalVfiEntries) {
        std::unordered_set<std::string> unique;
        for(const auto& source:originalVfiEntries){
            if(source.path.size()<=OriginalUsStageVfiPrefix.size())continue;
            const auto folded=originalResourceAsciiFold(source.path);
            if(!std::string_view(folded).starts_with(OriginalUsStageVfiPrefix))continue;
            const auto tail=std::string_view(source.path).substr(OriginalUsStageVfiPrefix.size());
            const auto slash=tail.find('/');
            if(slash==std::string_view::npos)continue;
            const auto stage=tail.substr(0,slash), local=tail.substr(slash+1);
            if(!originalResourceValidStageName(stage) ||
               !originalResourceValidStageRelativeName(local))
                throw std::invalid_argument("Malformed original USA stage VFI path");
            if(!unique.insert(folded).second)
                throw std::invalid_argument("Ambiguous duplicate original stage VFI path");
            const auto stageName=std::string(stage);
            if(std::none_of(stages_.begin(),stages_.end(),[&](const std::string& value){
                return originalResourceAsciiFold(value)==originalResourceAsciiFold(stageName);
            }))stages_.push_back(stageName);
            const bool compressed=originalResourceIsSz(local);
            entries_.push_back({source,stageName,std::string(local),
                std::string(compressed?local.substr(0,local.size()-3):local),compressed});
        }
    }
    std::span<const std::string> stageNames() const noexcept {return stages_;}
    std::span<const OriginalStageVfiResource> entries() const noexcept {return entries_;}
    std::vector<std::size_t> entriesForStage(std::string_view stage) const {
        if(!originalResourceValidStageName(stage))
            throw std::invalid_argument("Invalid original USA stage identifier");
        const auto folded=originalResourceAsciiFold(stage);
        std::vector<std::size_t> result;
        for(std::size_t i=0;i<entries_.size();++i)
            if(originalResourceAsciiFold(entries_[i].stageName)==folded)result.push_back(i);
        return result;
    }
    // Do not guess foo.pck alias for a source whose actual name is foo.pck.sz.
    std::optional<std::size_t> findExact(std::string_view stage,
                                        std::string_view stageRelativePath) const {
        if(!originalResourceValidStageName(stage) ||
           !originalResourceValidStageRelativeName(stageRelativePath))
            throw std::invalid_argument("Invalid original USA stage resource selection");
        const auto wantedStage=originalResourceAsciiFold(stage);
        const auto wantedName=originalResourceAsciiFold(stageRelativePath);
        for(std::size_t i=0;i<entries_.size();++i)
            if(originalResourceAsciiFold(entries_[i].stageName)==wantedStage &&
               originalResourceAsciiFold(entries_[i].stageRelativePath)==wantedName)return i;
        return std::nullopt;
    }
    std::vector<std::uint8_t> readExact(OriginalResourceVfiMount& mount,
                                      std::string_view stage,
                                      std::string_view stageRelativePath) const {
        const auto index=findExact(stage,stageRelativePath);
        if(!index)throw std::out_of_range("Original stage resource missing from owner-owned USA ISO");
        return mount.readOriginalFile(entries_.at(*index).sourceEntry.path);
    }
    std::unique_ptr<OriginalPckResourcePackage> openExactPck(
                OriginalResourceVfiMount& mount,std::string_view stage,
                std::string_view stageRelativePath) const {
        const auto index=findExact(stage,stageRelativePath);
        if(!index)throw std::out_of_range("Original stage PCK missing from owner-owned USA ISO");
        if(!std::string_view(originalResourceAsciiFold(entries_.at(*index).uncompressedName)).ends_with(".pck"))
            throw std::invalid_argument("Original VFI source entry is not PCK");
        return mount.openPck(entries_.at(*index).sourceEntry.path);
    }
private:
    std::vector<std::string> stages_;
    std::vector<OriginalStageVfiResource> entries_;
};
} // namespace ae3
