#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <bit>
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

// Original 0x2BA3AC reads exactly source+0x08 bytes from source+0x00.
// Only LF (0x0A) commits the current string to the original vector.
// The original 0x627C68 find helper checks literal "sizest" at
// 0x6F8E08 and then, on a later non-"sizest" line, "b:" at
// 0x6F8E10; finding the latter terminates source line scanning.
// A final source fragment lacking LF is *not* emitted by 0x2BA3AC.
struct OriginalAttributeLineScan_2ba3ac {
    std::vector<std::string> committedLines;
    bool sourceSawSizeMarker=false;
    bool terminatedAtBColon=false;
    std::size_t consumedSourceBytes=0;
};
inline OriginalAttributeLineScan_2ba3ac originalAttributeScanLines_2ba3ac(
        std::string_view originalSource) {
    OriginalAttributeLineScan_2ba3ac result;
    std::string currentLine;
    for(std::size_t i=0;i<originalSource.size();++i) {
        if(originalSource[i]!='\n') {
            currentLine.push_back(originalSource[i]);
            continue;
        }
        result.committedLines.push_back(std::move(currentLine));
        currentLine.clear();
        const auto& last=result.committedLines.back();
        if(last.find("sizest")!=std::string::npos) {
            result.sourceSawSizeMarker=true;
        } else if(result.sourceSawSizeMarker &&
                  last.find("b:")!=std::string::npos) {
            result.terminatedAtBColon=true;
            result.consumedSourceBytes=i+1;
            return result;
        }
    }
    result.consumedSourceBytes=originalSource.size();
    return result;
}
inline std::vector<std::string> originalAttributePhysicalLineSegments_2ba3ac(
        std::string_view originalSource) {
    return originalAttributeScanLines_2ba3ac(originalSource).committedLines;
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
// Original 0x2BA6BC requires at least 4 colon-separated fields.
// Its 0x2BA6F0 call to 0x428C48 reads field index 3 (sp+0x0C)
// and stores the signed low 32 bits at allocated record+0x20.
// 0x428C48 delegates to the original base-10 strtol implementation.
// This is equivalent for ordinary in-range decimal input; overflow
// and locale-sensitive behavior have not been differentially tested.
inline std::optional<std::int32_t> originalAttributeFourthInteger_2ba6f0(
        const OriginalAttributeTokenizedLine& input) {
    if(input.colonFields.size()<4)return std::nullopt;
    const std::string& bytes=input.colonFields[3];
    const long long sourceNumber=std::strtoll(bytes.c_str(),nullptr,10);
    // Original 0x428C48 truncates the low word then sign-extends.
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(sourceNumber));
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
