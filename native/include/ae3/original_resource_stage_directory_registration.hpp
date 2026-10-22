#pragma once
#include "ae3/original_resource_directory_order.hpp"
#include "ae3/original_resource_stage_registration.hpp"
#include <memory>
#include <span>
#include <string_view>
#include <vector>

namespace ae3 {
// Connected host-visible original resource chain:
// real stage's VFI child paths -> original 0x627E60 bytewise directory
// ordering / 0x49C150 in-order traversal -> 0x24D858 filename record
// -> 0x2519FC type filter -> 0x251A30 vector -> 0x24D4F0 lookup.
//
// The original runtime's DirInfo allocation and choice of stage directory
// are NOT reconstructed. Caller supplies the actual directory relative
// to its original stage. Only direct children present in genuine VFI are
// considered. This adapter never writes invented 32-bit EE pointers.
// The referenced OriginalStageResourceScope MUST outlive this object.
class OriginalStageDirectoryRegistration_251958 final {
public:
    OriginalStageDirectoryRegistration_251958(
         OriginalStageResourceScope& actualStage,
         std::string_view explicitSourceRelativeDirectory,
         const char* originalTypeSelector)
        : sourceOrder_(originalStageDirectoryVfiView_251958(
               actualStage.catalog(),actualStage.stageName(),
               explicitSourceRelativeDirectory)) {
        sourcePaths_.reserve(sourceOrder_.size());
        for(const auto& file:sourceOrder_)
            sourcePaths_.push_back(file.originalStageRelativePath);
        registered_=std::make_unique<OriginalStageScopedRegistration_25182c>(
             actualStage,sourcePaths_,originalTypeSelector);
    }
    OriginalStageDirectoryRegistration_251958(const OriginalStageDirectoryRegistration_251958&)=delete;
    OriginalStageDirectoryRegistration_251958& operator=(const OriginalStageDirectoryRegistration_251958&)=delete;
    OriginalStageDirectoryRegistration_251958(OriginalStageDirectoryRegistration_251958&&)=delete;
    OriginalStageDirectoryRegistration_251958& operator=(OriginalStageDirectoryRegistration_251958&&)=delete;

    std::span<const OriginalStageDirectoryVfiEntry_251958> actualDirectorySourceOrder() const noexcept {
        return sourceOrder_;
    }
    std::span<const OriginalStageRegisteredRecord_25182c> registeredRecords() const noexcept {
        return registered_->records();
    }
    const OriginalStageNamedEntry16_24d4f0* originalFind_24d4f0(
            std::string_view originalSourceName) const noexcept {
        return registered_->originalFind_24d4f0(originalSourceName);
    }
private:
    std::vector<OriginalStageDirectoryVfiEntry_251958> sourceOrder_;
    std::vector<std::string_view> sourcePaths_;
    std::unique_ptr<OriginalStageScopedRegistration_25182c> registered_;
};
} // namespace ae3
