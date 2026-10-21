#pragma once
#include "ae3/original_nav_group_registry.hpp"
#include "ae3/original_navrange_source_frame.hpp"

namespace ae3 {
// Source-grounded extension of the existing required eligibility callback.
// No fabricated eligibility result is permitted by this interface.
class OriginalNavrangeSourceEligibility {
public:
    virtual ~OriginalNavrangeSourceEligibility()=default;
    virtual float call_original_21ef3c(
        const OriginalNavGroupKey20& originalGroupKey,
        const OriginalSpatialRecord70& originalPolygon,
        const OriginalVU0Vec4& originalCentroid,
        const OriginalNavrangeResourceFrame& originalHeaderFrame)=0;
};
// Same recovered registry and polygon scan, but now presents the
// original +0x20/+0x30/+0x40 vectors with each polygon's centroid.
OriginalNavGroupBuildResult originalNavBuildGroupsSourceAware_4e4060(
    OriginalNavGroupRegistry& registry,
    const OriginalNavGroupSource& source,
    std::span<const OriginalSpatialRecord70> records,
    std::span<const std::byte> nav,
    OriginalNavrangeSourceEligibility& eligibility);
} // namespace ae3
