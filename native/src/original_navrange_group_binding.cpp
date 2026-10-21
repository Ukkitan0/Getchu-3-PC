#include "ae3/original_navrange_group_binding.hpp"

namespace ae3 {
OriginalNavGroupBuildResult originalNavBuildGroupsSourceAware_4e4060(
    OriginalNavGroupRegistry& registry, const OriginalNavGroupSource& source,
    std::span<const OriginalSpatialRecord70> records,
    std::span<const std::byte> nav, OriginalNavrangeSourceEligibility& eligibility) {
    // Preserve original magic and zero-group skips before decoding vectors.
    if(!originalNavGroupHeaderMatches_4e4080(source.originalResourceHeader))
        return {};
    if(originalNavGroupKeys_4e4060(source).empty())
        return {};
    const auto frame=originalNavrangeSourceFrame_4e4110(source.originalResourceHeader);
    class OriginalSourceAdapter final : public OriginalNavGroupEligibility {
    public:
        OriginalSourceAdapter(const OriginalNavrangeResourceFrame& frame,
                              OriginalNavrangeSourceEligibility& callback)
          : frame_(frame),callback_(callback){}
        float call_original_21ef3c(const OriginalNavGroupKey20& key,
             const OriginalSpatialRecord70& polygon,
             const OriginalVU0Vec4& centroid) override {
            return callback_.call_original_21ef3c(key,polygon,centroid,frame_);
        }
    private:
        const OriginalNavrangeResourceFrame& frame_;
        OriginalNavrangeSourceEligibility& callback_;
    } adapter(frame,eligibility);
    return originalNavBuildGroups_4e4060(registry,source,records,nav,adapter);
}
} // namespace ae3
