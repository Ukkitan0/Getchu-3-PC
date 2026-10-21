#pragma once
#include <cstddef>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace ae3 {
// Original shared source string delimiter helper 0x1F28C4: it appends
// every completed delimiter-terminated token, including empty tokens;
// on end-of-input it appends the trailing token only if nonempty.
// This is a native algorithm, not an R5900 interpreter.
inline std::vector<std::string> originalSplitDelimited_1f28c4(
        std::string_view originalSource, char delimiter) {
    std::vector<std::string> output;
    std::size_t tokenStart=0;
    for(std::size_t i=0;i<originalSource.size();++i) {
        if(originalSource[i]!=delimiter)continue;
        output.emplace_back(originalSource.substr(tokenStart,i-tokenStart));
        tokenStart=i+1;
    }
    if(tokenStart<originalSource.size())
        output.emplace_back(originalSource.substr(tokenStart));
    return output;
}

// 0x2BA3AC scans original input with source LF (0x0A) as its line
// delimiter. This returns lexical segments WITHOUT inventing the
// source's later structural filters or descriptor attributes.
inline std::vector<std::string> originalAttributePhysicalLineSegments_2ba3ac(
        std::string_view originalSource) {
    return originalSplitDelimited_1f28c4(originalSource,'\n');
}

struct OriginalAttributeTokenizedLine {
    std::string sourceLine;
    std::vector<std::string> colonFields;
};
// At 0x2BA5F4 the actual parser calls shared 0x1F28C4 with A2=0x3A
// (':'). The original node allocator and tree entry creation happen
// later at 0x2BA714..0x2BA9C0, NOT in this tokenizer.
inline OriginalAttributeTokenizedLine originalAttributeSplitColon_2ba5f4(
        std::string_view sourceLine) {
    return {std::string(sourceLine),originalSplitDelimited_1f28c4(sourceLine,':')};
}
struct OriginalAttributeTokenization {
    std::vector<OriginalAttributeTokenizedLine> sourceLines;
    std::size_t physicalSegmentCount=0;
};
// Source lifecycle 0x2BB03C: init 0x2BA218, LF pass 0x2BA3AC,
// colon pass 0x2BA584. This native lexical boundary stops BEFORE
// the unported data->tree entry materialization within 0x2BA584.
// Preserve even empty physical lines here for the downstream original
// parser; the retail semantic filtering of those rows is not modeled.
inline OriginalAttributeTokenization originalAttributeTokenizeSource_2bb03c(
        std::string_view loadedText) {
    OriginalAttributeTokenization result;
    auto lines=originalAttributePhysicalLineSegments_2ba3ac(loadedText);
    result.physicalSegmentCount=lines.size();
    result.sourceLines.reserve(lines.size());
    for(const auto& line:lines)
        result.sourceLines.push_back(originalAttributeSplitColon_2ba5f4(line));
    return result;
}
} // namespace ae3
