#pragma once
#include "ae3/original_resource_record_registration.hpp"
#include "ae3/original_resource_stage_scope.hpp"
#include <cstddef>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace ae3 {
// Source-backed bridge: VFI filename and allocation -> 0x24D858
// name/type record -> 0x2519FC source type selector ->
// 0x251A0C registration vector -> 0x24D4F0 named lookup.
// Each requested stage path MUST exist in the actual USA VFI index.
// The caller supplies actual original resource traversal order from
// the upstream loader, which is not yet completely reconstructed.
//
// This bridge NEVER guesses stage filenames/extensions and never
// synthesizes EE guest addresses. Its byte spans borrow the stage
// scope's immutable owner allocations. Destroy this bridge before the
// stage scope and every consumer of its returned views.
class OriginalStageScopedRegistration_25182c final {
public:
    OriginalStageScopedRegistration_25182c(
             OriginalStageResourceScope& realStage,
             std::span<const std::string_view> originalStageRelativePathsInSourceOrder,
             const char* sourceTypeSelector)
      : stage_(realStage) {
        inputs_.reserve(originalStageRelativePathsInSourceOrder.size());
        for(const auto& relativePath:originalStageRelativePathsInSourceOrder) {
            const auto index=stage_.catalog().findExact(stage_.stageName(),relativePath);
            if(!index)
                throw std::out_of_range("Original registration VFI file absent from actual stage");
            const auto& source=stage_.catalog().entries()[*index];
            const auto originalLeaf=std::string_view(source.stageRelativePath);
            const auto slash=originalLeaf.rfind('/');
            const auto basename=slash==std::string_view::npos ? originalLeaf
                                                           : originalLeaf.substr(slash+1);
            const auto bytes=stage_.file(source.stageRelativePath);
            inputs_.push_back({basename,std::as_bytes(bytes),*index});
        }
        registered_=std::make_unique<OriginalStageResourceRegistration_25182c>(
            inputs_,sourceTypeSelector);
    }
    OriginalStageScopedRegistration_25182c(const OriginalStageScopedRegistration_25182c&)=delete;
    OriginalStageScopedRegistration_25182c& operator=(const OriginalStageScopedRegistration_25182c&)=delete;
    OriginalStageScopedRegistration_25182c(OriginalStageScopedRegistration_25182c&&)=delete;
    OriginalStageScopedRegistration_25182c& operator=(OriginalStageScopedRegistration_25182c&&)=delete;

    std::span<const OriginalStageRegisteredRecord_25182c> records() const noexcept {
        return registered_->records();
    }
    const OriginalStageNamedEntry16_24d4f0* originalFind_24d4f0(
            std::string_view originalName) const noexcept {
        return registered_->originalFind_24d4f0(originalName);
    }
private:
    OriginalStageResourceScope& stage_;
    std::vector<OriginalStageSourceRecordInput_25182c> inputs_;
    std::unique_ptr<OriginalStageResourceRegistration_25182c> registered_;
};
} // namespace ae3
