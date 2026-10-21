#pragma once
#include "ae3/original_navrange_group_binding.hpp"
#include "ae3/original_navrange_extent_score.hpp"

namespace ae3 {
// Explicit, opt-in source-derived mathematical eligibility.
// The retail game caller excludes a polygon when F20 != 0.0f.
// This implementation is not advertised as bit-exact VU0 emulation
// and is not silently substituted for other eligibility callbacks.
class OriginalNavrangeRecoveredEligibility final:
    public OriginalNavrangeSourceEligibility {
public:
    float call_original_21ef3c(
        const OriginalNavGroupKey20&,
        const OriginalSpatialRecord70&,
        const OriginalVU0Vec4& centroid,
        const OriginalNavrangeResourceFrame& headerFrame) override {
        return originalNavrangeScore_21ef3c(headerFrame,centroid).f20;
    }
};

// Wires original navrange resource -> keyed owner -> actual polygons ->
// native recovered eligibility without a manually supplied predicate.
// Use only after checking actual source assets/precision behavior.
inline OriginalNavGroupBuildResult originalNavBuildGroupsRecovered_4e4060(
    OriginalNavGroupRegistry& registry,
    const OriginalNavGroupSource& source,
    std::span<const OriginalSpatialRecord70> records,
    std::span<const std::byte> nav) {
    OriginalNavrangeRecoveredEligibility predicate;
    return originalNavBuildGroupsSourceAware_4e4060(
        registry,source,records,nav,predicate);
}
} // namespace ae3
