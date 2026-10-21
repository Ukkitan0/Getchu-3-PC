#pragma once
#include "ae3/original_nav_linked_owner.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>
namespace ae3 {
struct OriginalNavGroupKey20 {
    std::array<std::byte,0x20> originalBytes{};
    bool operator==(const OriginalNavGroupKey20&) const = default;
};
struct OriginalNavGroupAllocation {
    std::uint32_t originalBaseVA=0;
    std::span<const std::byte> bytes;
};
struct OriginalNavGroupSource {
    std::span<const std::byte> originalResourceHeader;
    std::span<const OriginalNavGroupAllocation> originalAllocations;
};
// 0x4E4080 uses R5900 PCEQB/PAND/PCEQW/PPACH; compares only
// eight source bytes to retail "navrange" at 0x6E3F10.
bool originalNavGroupHeaderMatches_4e4080(std::span<const std::byte>) noexcept;
// 0x4E40AC/+0x54 signed count and +0x5C 32-bit allocation pointer;
// 0x4E40C4 advances through original 0x20-byte keys.
std::vector<OriginalNavGroupKey20> originalNavGroupKeys_4e4060(
    const OriginalNavGroupSource&);
class OriginalNavGroupRegistry {
public:
    OriginalNavListOwner* originalFind_36d6f4(const OriginalNavGroupKey20&) noexcept;
    const OriginalNavListOwner* originalFind_36d6f4(const OriginalNavGroupKey20&) const noexcept;
    OriginalNavListOwner& originalFindOrCreate_4e4060(const OriginalNavGroupKey20&);
    std::size_t size() const noexcept {return entries_.size();}
    void originalClear_4e3fec() noexcept;
private:
    struct Entry {
        OriginalNavGroupKey20 originalKey;
        std::unique_ptr<OriginalNavListOwner> owner;
    };
    std::vector<Entry> entries_;
};
class OriginalNavGroupEligibility {
public:
    virtual ~OriginalNavGroupEligibility()=default;
    // Untranslated retail 0x21EF3C COP2/MMI geometry. Never assume
    // success when the original game would filter a source polygon.
    virtual float call_original_21ef3c(const OriginalNavGroupKey20&,
        const OriginalSpatialRecord70&,const OriginalVU0Vec4&)=0;
};
struct OriginalNavGroupBuildResult {
    std::size_t sourceGroups=0;
    std::size_t ownersCreated=0;
    std::size_t sourcePolygonsInserted=0;
};
// Original 0x4E4060 outer group loop and owner lookup/insertion;
// 0x21F6D4 scans actual source polygons for each group.
OriginalNavGroupBuildResult originalNavBuildGroups_4e4060(
    OriginalNavGroupRegistry&,const OriginalNavGroupSource&,
    std::span<const OriginalSpatialRecord70>,
    std::span<const std::byte>,OriginalNavGroupEligibility&);
} // namespace ae3
