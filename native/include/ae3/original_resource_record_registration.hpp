#pragma once
#include "ae3/original_resource_named_lookup.hpp"
#include "ae3/original_resource_registration_filter.hpp"
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ae3 {
// Source ELF: 0x24D858..0x24D9E8 constructs the 0x10-byte stage
// resource record. At +0/+4 it constructs source name/type strings;
// at +8/+0xC it copies the supplied original pointer and byte count.
// 0x627CF8 searches for the FIRST '.', and 0x627038 is called with
// source offsets (0,dot) and (dot+1,-1), respectively.
// Names lacking '.' take the source's empty-string fallback.
// It is the CALLER's responsibility to pass the original source leaf
// filename, not an invented name or a full filesystem path.
struct OriginalStageFilenameParts_24d858 {
    std::string originalNameAt00;
    std::string originalTypeAt04;
};
inline OriginalStageFilenameParts_24d858 originalStageSplitFilename_24d858(
       std::string_view sourceLeafFilename) {
    const auto dot=sourceLeafFilename.find('.');
    if(dot==std::string_view::npos)return {};
    return {std::string(sourceLeafFilename.substr(0,dot)),
            std::string(sourceLeafFilename.substr(dot+1))};
}

// Source bytes are borrowed. The caller's source allocation must outlive
// the registry and all views given to consumers of original 0x24D4F0.
struct OriginalStageSourceRecordInput_25182c {
    std::string_view originalLeafFilename;
    std::span<const std::byte> originalBytes;
    std::size_t originalSourceIndex=0;
};
struct OriginalStageRegisteredRecord_25182c {
    std::string originalNameAt00;
    std::string originalTypeAt04;
    std::span<const std::byte> originalPayloadAt08;
    std::uint32_t originalSizeAt0c=0;
    std::size_t originalSourceIndex=0;
};

// Recovered selection+append segment of 0x25182C:
// 0x2519AC calls 0x24D858 to construct the name,type,data,size
// record; 0x2519D4..0x251A08 filters by TYPE at entry+4;
// 0x251A0C..0x251A40 appends the 0x10-byte record into the caller's
// vector (0x50ACD4, or 0x50A238 when first/growing).
// The surrounding directory-tree traversal is NOT implemented here;
// caller supplies its original file records in actual traversal order.
class OriginalStageResourceRegistration_25182c final {
public:
    OriginalStageResourceRegistration_25182c(
           std::span<const OriginalStageSourceRecordInput_25182c> actualSourceFiles,
           const char* originalTypeSelector) {
        records_.reserve(actualSourceFiles.size());
        for(const auto& source:actualSourceFiles) {
            const auto names=originalStageSplitFilename_24d858(source.originalLeafFilename);
            if(!originalResourceAcceptSelector_2519d4(names.originalTypeAt04,
                                                       originalTypeSelector))
                continue;
            if(source.originalBytes.size()>UINT32_MAX)
                throw std::out_of_range("Original 0x24D858 source byte count exceeds EE u32");
            records_.push_back({names.originalNameAt00,
                                names.originalTypeAt04,
                                source.originalBytes,
                                static_cast<std::uint32_t>(source.originalBytes.size()),
                                source.originalSourceIndex});
        }
        sourceViews_.reserve(records_.size());
        for(const auto& item:records_)
            sourceViews_.push_back({item.originalNameAt00,
                                    item.originalPayloadAt08,
                                    item.originalSourceIndex});
    }
    OriginalStageResourceRegistration_25182c(const OriginalStageResourceRegistration_25182c&)=delete;
    OriginalStageResourceRegistration_25182c& operator=(const OriginalStageResourceRegistration_25182c&)=delete;
    OriginalStageResourceRegistration_25182c(OriginalStageResourceRegistration_25182c&&)=delete;
    OriginalStageResourceRegistration_25182c& operator=(OriginalStageResourceRegistration_25182c&&)=delete;

    std::span<const OriginalStageRegisteredRecord_25182c> records() const noexcept {
        return records_;
    }
    std::span<const OriginalStageNamedEntry16_24d4f0> namedEntries() const noexcept {
        return sourceViews_;
    }
    const OriginalStageNamedEntry16_24d4f0* originalFind_24d4f0(
            std::string_view originalName) const noexcept {
        return originalStageNamedLookup_24d4f0(sourceViews_,originalName);
    }
private:
    std::vector<OriginalStageRegisteredRecord_25182c> records_;
    std::vector<OriginalStageNamedEntry16_24d4f0> sourceViews_;
};
} // namespace ae3
