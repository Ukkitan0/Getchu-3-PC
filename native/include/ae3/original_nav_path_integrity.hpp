#pragma once
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

namespace ae3 {
// Host-native integrity preflight for a source-derived 0x220994 path.
// It does NOT reconstruct an additional original EE routine and does
// not change the original 0x221104 first-match / no-match slot-0 return.
// Rejected paths are corrupted caller-owned snapshots, not alternate
// routes or disconnected edges to be silently searched around.
inline void originalNavValidateNativeSourceRouteIntegrity(
    std::span<const std::byte> nav, std::size_t recordCount,
    std::span<const std::uint32_t> admitted,
    std::span<const std::uint32_t> forward,
    std::int32_t selectedSearchIndex) {
    const auto read16=[&](std::size_t pos)->std::uint16_t {
        if(pos>nav.size() || nav.size()-pos<2)
            throw std::out_of_range("Original navigation route descriptor truncated");
        return std::uint16_t(std::to_integer<unsigned>(nav[pos])) |
            (std::uint16_t(std::to_integer<unsigned>(nav[pos+1]))<<8);
    };
    const auto read32=[&](std::size_t pos)->std::uint32_t {
        return std::uint32_t(read16(pos)) | (std::uint32_t(read16(pos+2))<<16);
    };
    const std::size_t originalCount=read32(0x24);
    const std::size_t table=read32(0x3c);
    if(originalCount!=recordCount || table>nav.size() ||
       recordCount>(nav.size()-table)/0x1eu)
        throw std::out_of_range("Original route source graph descriptor count mismatch");
    constexpr std::size_t workspaceCapacity=256; // 0x3800 / 0x38
    if(admitted.size()>workspaceCapacity)
        throw std::out_of_range("Original 0x2207D0 route workspace overflow");
    if(selectedSearchIndex < -1 ||
       (selectedSearchIndex>=0 &&
        static_cast<std::size_t>(selectedSearchIndex)>=admitted.size()))
        throw std::out_of_range("Original route destination search index invalid");
    if((selectedSearchIndex==-1) != forward.empty())
        throw std::invalid_argument("Original route destination and forward path disagree");
    if(admitted.empty() && (!forward.empty() || selectedSearchIndex!=-1))
        throw std::invalid_argument("Original route missing source admission");
    std::vector<unsigned char> inWorkspace(recordCount,0);
    for(const auto source:admitted) {
        if(source>=recordCount || inWorkspace[source])
            throw std::invalid_argument("Original route admitted source ID invalid or repeated");
        inWorkspace[source]=1;
    }
    if(forward.empty())return; // genuine route-mode explicit $8=-1
    if(forward.size()>admitted.size() || forward.front()!=admitted.front() ||
       forward.back()!=admitted[static_cast<std::size_t>(selectedSearchIndex)])
        throw std::invalid_argument("Original 0x220994 source root/endpoint mismatch");
    std::vector<unsigned char> alreadyOnRoute(recordCount,0);
    for(std::size_t i=0;i<forward.size();++i){
        const auto source=forward[i];
        if(source>=recordCount || !inWorkspace[source] || alreadyOnRoute[source])
            throw std::invalid_argument("Original 0x220994 forward path has an unadmitted or cycled record");
        alreadyOnRoute[source]=1;
        if(i+1==forward.size())break;
        const auto descriptor=table+std::size_t(source)*0x1eu;
        const auto linkCount=read16(descriptor+2);
        if(linkCount>3)throw std::out_of_range("Original polygon has more than three source links");
        bool linked=false;
        for(std::size_t k=0;k<linkCount;++k){
            if(read16(descriptor+4+2*k)==forward[i+1]){
                linked=true;
                break; // original 0x221104 uses first matching source slot
            }
        }
        if(!linked)
            throw std::invalid_argument("Original 0x220994 route step has no directed source adjacency");
    }
}
} // namespace ae3
