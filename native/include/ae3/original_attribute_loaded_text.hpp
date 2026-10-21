#pragma once
#include "ae3/original_attribute_record_trailer.hpp"
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace ae3 {
// Native composition of the original 0x2BB03C call chain:
// text -> original 0x2BA3AC line scan -> original 0x2BA584
// header/children/trailer -> original parsed-key insertion.
// Input must be the retail parser's already-loaded source-text bytes.
// This is NOT the original file loader or a guess at disk encoding.
struct OriginalAttributeLoadedText_2bb03c {
    OriginalAttributeMaterializedFields_2ba584 parsed;
    std::size_t committedPhysicalLines=0;
};
inline OriginalAttributeLoadedText_2bb03c originalAttributeBindLoadedText_2bb03c(
        OriginalAttributeTreeOwner& owner,
        std::string_view originalLoadedText,
        std::uint32_t originalOwnerBasePointer) {
    auto tokens=originalAttributeTokenizeSource_2bb03c(originalLoadedText);
    auto values=originalAttributeStructuredFields_2ba584(tokens,originalOwnerBasePointer);
    // Validate the source before changing host ownership. Original
    // cleanup at 0x2BA218 precedes parsing; exception behavior here
    // deliberately provides stronger host safety than retail code.
    owner.clear();
    originalAttributeBindDecodedChildren_2ba9c0(owner,values.body.sourceChildren);
    return {std::move(values),tokens.physicalSegmentCount};
}
} // namespace ae3
