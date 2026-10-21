#pragma once
#include "ae3/original_attribute_record_parser.hpp"
#include <bit>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace ae3 {
// Original 0x2BA9F8..0x2BAB60: after child allocations, source
// loads penultimate preprocessed line (end-8) and final line (end-4).
// Each is split with ':' and token index 1 converted via 0x428C48.
// The penultimate value goes to parser-owner +0x24 (0x2BAA98).
// The last value is added (32-bit) to owner+0x00 and written to +0x04.
// These are owner fields, NOT fields of an individual 0x34 child.
struct OriginalAttributeRecordTrailer_2baa90 {
    std::int32_t sourcePenultimateWordAt24=0;
    std::int32_t sourceFinalRebaseDelta=0;
    std::uint32_t originalRebasedPointerAt04=0;
};
inline OriginalAttributeRecordTrailer_2baa90 originalAttributeTrailerFields_2baa90_2bab60(
    const OriginalAttributeTokenization& source,
    std::size_t parsedChildCount,
    std::uint32_t originalOwnerBasePointerAt00) {
    // The source indexes children at 2..count+1, and the final
    // two distinct trailer lines from end-2 and end-1.
    if(source.sourceLines.size()<4 ||
       parsedChildCount>source.sourceLines.size()-4)
        throw std::out_of_range("Original attribute source is missing two trailer lines");
    const auto convert=[](const OriginalAttributeTokenizedLine& line) {
        if(line.colonFields.size()<2)
            throw std::invalid_argument("Original attribute trailer needs two colon fields");
        OriginalAttributeTokenizedLine adapted;
        adapted.colonFields={"", "", "", line.colonFields[1]};
        return *originalAttributeFourthInteger_2ba6f0(adapted);
    };
    const auto beforeLast=convert(source.sourceLines[source.sourceLines.size()-2]);
    const auto last=convert(source.sourceLines.back());
    const auto rebased=originalOwnerBasePointerAt00+std::bit_cast<std::uint32_t>(last);
    return {beforeLast,last,rebased};
}
struct OriginalAttributeMaterializedFields_2ba584 {
    OriginalAttributeParsedBody_2ba584 body;
    OriginalAttributeRecordTrailer_2baa90 trailer;
};
// Source-level native owner fields, still no guest allocations or
// original source-file format decoder. Reuses the earlier header/
// child source recovery without silently inventing memory addresses.
inline OriginalAttributeMaterializedFields_2ba584 originalAttributeStructuredFields_2ba584(
    const OriginalAttributeTokenization& source,
    std::uint32_t originalOwnerBasePointerAt00) {
    auto body=originalAttributeRecordsFromTokens_2ba584(source);
    auto trailer=originalAttributeTrailerFields_2baa90_2bab60(
        source,body.sourceChildren.size(),originalOwnerBasePointerAt00);
    return {std::move(body),trailer};
}
} // namespace ae3
