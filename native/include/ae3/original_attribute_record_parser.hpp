#pragma once
#include "ae3/original_attribute_text_tokens.hpp"
#include "ae3/original_attribute_tree_owner.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace ae3 {
// Retail original function 0x2BA584: the 2nd preprocessed input line
// is tokenized with ':'. Its first and fourth fields are converted by
// 0x428C48 and stored to the original aggregate at +0x1C and +0x20.
struct OriginalAttributeRecordHeader_2ba584 {
    std::int32_t sourceChildCountAt1c=0;
    std::int32_t sourceFourthWordAt20=0;
};
inline OriginalAttributeRecordHeader_2ba584 originalAttributeHeaderFields_2ba6a4_2ba6f0(
    const OriginalAttributeTokenizedLine& sourceHeader) {
    if(sourceHeader.colonFields.size()<4)
        throw std::invalid_argument("Original 0x2BA584 header requires four colon-separated fields");
    const auto read=[&](std::size_t n) {
        OriginalAttributeTokenizedLine temp;
        temp.colonFields={"", "", "", sourceHeader.colonFields[n]};
        return *originalAttributeFourthInteger_2ba6f0(temp);
    };
    return {read(2),read(3)};
}

// Each source 0x2BA714 allocation requests 0x34 bytes. The original
// stores the first character of field 0 at +0x00, retains an original
// string handle from field 2 at +0x04, and writes decimal field 1 at
// +0x08. Other fields of the 0x34-byte allocation are not modeled.
struct OriginalAttributeParsedChild34_2ba714 {
    std::uint8_t sourceFirstCharacterAt00=0;
    std::string sourceKeyAt04;
    std::int32_t sourceValueOffsetAt08=0;
    std::size_t sourceLineIndex=0;
};
inline OriginalAttributeParsedChild34_2ba714 originalAttributeChildFields_2ba7d8_2ba8b0(
    const OriginalAttributeTokenizedLine& sourceLine,std::size_t lineIndex=0) {
    if(sourceLine.colonFields.size()<3 || sourceLine.colonFields[0].empty())
        throw std::invalid_argument("Original 0x2BA714 child requires a leading character and three fields");
    // 0x2BA7FC..0x2BA818 reads byte 0 of field 0.
    // 0x2BA854 uses field 2 as source for original string assignment.
    // 0x2BA8A8 uses field 1 as decimal input and stores at +0x08.
    OriginalAttributeTokenizedLine numberField;
    numberField.colonFields={"", "", "", sourceLine.colonFields[1]};
    return {static_cast<std::uint8_t>(static_cast<unsigned char>(sourceLine.colonFields[0][0])),
        sourceLine.colonFields[2],*originalAttributeFourthInteger_2ba6f0(numberField),lineIndex};
}

struct OriginalAttributeParsedBody_2ba584 {
    OriginalAttributeRecordHeader_2ba584 sourceHeader;
    std::vector<OriginalAttributeParsedChild34_2ba714> sourceChildren;
};
// The original source at +0x0C is a preprocessed array of line-string
// objects. At 0x2BA5E8 it loads entry 1 for the header; child loop
// 0x2BA71C begins at entry 2 and advances one entry per allocation.
// Input here is PRETOKENIZED line content. Earlier newline filtering,
// later trailer rows and original disk format are independent gaps.
inline OriginalAttributeParsedBody_2ba584 originalAttributeRecordsFromTokens_2ba584(
    const OriginalAttributeTokenization& source) {
    if(source.sourceLines.size()<2)
        throw std::invalid_argument("Original attribute parser missing second line header");
    OriginalAttributeParsedBody_2ba584 out;
    out.sourceHeader=originalAttributeHeaderFields_2ba6a4_2ba6f0(source.sourceLines[1]);
    const auto signedCount=out.sourceHeader.sourceChildCountAt1c;
    if(signedCount==0)return out; // 0x2BA700 BEQZ
    if(signedCount<0)
        throw std::invalid_argument("Negative original child count would overflow retail unsigned loop");
    const std::size_t count=static_cast<std::size_t>(signedCount);
    if(count>source.sourceLines.size()-2)
        throw std::out_of_range("Original attribute source has fewer child lines than its header count");
    out.sourceChildren.reserve(count);
    for(std::size_t i=0;i<count;++i)
        out.sourceChildren.push_back(originalAttributeChildFields_2ba7d8_2ba8b0(source.sourceLines[i+2],i+2));
    return out;
}

// Original 0x2BA9C0 writes a POINTER to the allocated +0x34 child
// into the map node at +0x14. Numeric/string getters later dereference
// child +0x08 to recover an offset into the original value buffer.
// Flatten that indirection only for the existing HOST-native decoded
// tree API: key -> source value offset. This does NOT reproduce the
// retail pointer layout and cannot serialize/restore original nodes.
inline void originalAttributeBindDecodedChildren_2ba9c0(
    OriginalAttributeTreeOwner& decodedOwner,
    std::span<const OriginalAttributeParsedChild34_2ba714> sourceChildren) {
    for(const auto& child:sourceChildren)
        (void)decodedOwner.originalBindParsed_2ba9c0(
            child.sourceKeyAt04,
            static_cast<std::uint32_t>(child.sourceValueOffsetAt08));
}
} // namespace ae3
