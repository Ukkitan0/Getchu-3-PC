#include "ae3/original_nav_group_registry.hpp"
#include <bit>
#include <limits>
#include <stdexcept>
#include <utility>
namespace ae3 {
namespace {
std::uint32_t read32(std::span<const std::byte> source,std::size_t at) {
    if(at>source.size()||source.size()-at<4)
        throw std::out_of_range("Original 0x4E4060 source header truncated");
    return std::uint32_t(std::to_integer<unsigned char>(source[at]))|
        (std::uint32_t(std::to_integer<unsigned char>(source[at+1]))<<8)|
        (std::uint32_t(std::to_integer<unsigned char>(source[at+2]))<<16)|
        (std::uint32_t(std::to_integer<unsigned char>(source[at+3]))<<24);
}
}
bool originalNavGroupHeaderMatches_4e4080(std::span<const std::byte> header) noexcept {
    constexpr char magic[]="navrange";
    if(header.size()<8)return false;
    for(std::size_t i=0;i<8;i++)
        if(header[i]!=std::byte(static_cast<unsigned char>(magic[i])))return false;
    return true;
}
std::vector<OriginalNavGroupKey20> originalNavGroupKeys_4e4060(
    const OriginalNavGroupSource& input) {
    const auto source=input.originalResourceHeader;
    if(!originalNavGroupHeaderMatches_4e4080(source))return {};
    const auto signedCount=std::bit_cast<std::int32_t>(read32(source,0x54));
    if(signedCount<=0)return {}; // original BLEZ
    const auto guestPtr=read32(source,0x5c);
    const auto count=static_cast<std::size_t>(signedCount);
    if(!guestPtr)throw std::invalid_argument("Original +0x5c group key allocation missing");
    if(count>std::numeric_limits<std::size_t>::max()/0x20)
        throw std::out_of_range("Original group count overflow");
    const std::size_t bytesNeeded=count*0x20;
    const OriginalNavGroupAllocation* allocated=nullptr;
    std::size_t start=0;
    for(const auto& segment:input.originalAllocations) {
        if(guestPtr<segment.originalBaseVA)continue;
        const std::uint64_t difference=std::uint64_t(guestPtr)-segment.originalBaseVA;
        if(difference>segment.bytes.size() ||
           bytesNeeded>segment.bytes.size()-static_cast<std::size_t>(difference))continue;
        if(allocated)throw std::invalid_argument("Ambiguous original group-key allocation");
        allocated=&segment;start=static_cast<std::size_t>(difference);
    }
    if(!allocated)throw std::out_of_range("Original 32-byte group-key allocation unresolved");
    std::vector<OriginalNavGroupKey20> result(count);
    for(std::size_t i=0;i<count;i++)
        for(std::size_t j=0;j<0x20;j++)
            result[i].originalBytes[j]=allocated->bytes[start+i*0x20+j];
    return result;
}
OriginalNavListOwner* OriginalNavGroupRegistry::originalFind_36d6f4(
    const OriginalNavGroupKey20& key) noexcept {
    for(auto& entry:entries_)if(entry.originalKey==key)return entry.owner.get();
    return nullptr;
}
const OriginalNavListOwner* OriginalNavGroupRegistry::originalFind_36d6f4(
    const OriginalNavGroupKey20& key) const noexcept {
    for(const auto& entry:entries_)if(entry.originalKey==key)return entry.owner.get();
    return nullptr;
}
OriginalNavListOwner& OriginalNavGroupRegistry::originalFindOrCreate_4e4060(
    const OriginalNavGroupKey20& key) {
    if(auto* existing=originalFind_36d6f4(key))return *existing;
    auto owner=std::make_unique<OriginalNavListOwner>();
    owner->originalCreate_21f604();
    entries_.reserve(entries_.size()+1);
    entries_.push_back({key,std::move(owner)});
    return *entries_.back().owner;
}
void OriginalNavGroupRegistry::originalClear_4e3fec() noexcept {
    for(auto& entry:entries_)entry.owner->originalDispose_21f690();
    entries_.clear();
}
OriginalNavGroupBuildResult originalNavBuildGroups_4e4060(
    OriginalNavGroupRegistry& registry,const OriginalNavGroupSource& source,
    std::span<const OriginalSpatialRecord70> records,
    std::span<const std::byte> nav,
    OriginalNavGroupEligibility& eligibility) {
    const auto keys=originalNavGroupKeys_4e4060(source);
    OriginalNavGroupBuildResult result{};
    result.sourceGroups=keys.size();
    for(const auto& key:keys) {
        const bool existed=registry.originalFind_36d6f4(key)!=nullptr;
        auto& owner=registry.originalFindOrCreate_4e4060(key);
        if(!existed)++result.ownersCreated;
        struct Adapter final:OriginalNavPopulationPredicate {
            const OriginalNavGroupKey20& sourceKey;
            OriginalNavGroupEligibility& sourcePredicate;
            Adapter(const OriginalNavGroupKey20& k,OriginalNavGroupEligibility& p)
                :sourceKey(k),sourcePredicate(p){}
            float call_original_21ef3c(
                const OriginalSpatialRecord70& polygon,
                const OriginalVU0Vec4& centroid) override {
                return sourcePredicate.call_original_21ef3c(sourceKey,polygon,centroid);
            }
        } adapter(key,eligibility);
        result.sourcePolygonsInserted+=
            originalNavPopulateList_21f6d4(owner,records,nav,adapter);
    }
    return result;
}
} // namespace ae3
