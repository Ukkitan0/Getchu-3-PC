#pragma once
#include "ae3/original_attribute_lookup.hpp"
#include <algorithm>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

namespace ae3 {
// Original 0x2BB1AC compares the source C string with this literal.
// When equal, 0x2BB3BC preserves the caller's fallback string object.
inline constexpr std::string_view OriginalEmptyStrings32_2bb1ac = "EmptyStrings32";

// Source-level 0x2BB3BC: resolve name with 0x5419EC, load the value's
// source-relative +0x08 offset through 0x54204C, construct its C string
// using 0x35AB88, then check 0x2BB1AC before choosing the original
// supplied default. The host string owns its bytes; no guest string
// layout or PS2 instructions are executed by this implementation.
inline std::string originalAttributeString_2bb3bc(
    std::span<const std::byte> originalBuffer,
    const OriginalAttributeKeyNode* decodedIndex,
    std::string_view originalKey,
    std::string_view originalDefault) {
    const auto* entry=originalAttributeFind_5419ec(decodedIndex,originalKey);
    if(!entry)return std::string(originalDefault);
    const auto offset=static_cast<std::size_t>(entry->sourceValueOffset);
    if(offset>=originalBuffer.size())
        throw std::out_of_range("Original string attribute pointer outside source allocation");
    const auto tail=originalBuffer.subspan(offset);
    const auto terminator=std::find(tail.begin(),tail.end(),std::byte{0});
    if(terminator==tail.end())
        throw std::out_of_range("Original string attribute lacks NUL terminator in allocation");
    const auto length=static_cast<std::size_t>(terminator-tail.begin());
    const std::string sourceValue(
        reinterpret_cast<const char*>(tail.data()),length);
    if(sourceValue==OriginalEmptyStrings32_2bb1ac)
        return std::string(originalDefault);
    return sourceValue;
}

// With an already-decoded ORIGINAL attribute tree and its loaded byte
// allocation, the audio descriptor's five original strings and one
// numeric field can now be resolved with no handwritten fixture callback.
// This intentionally does NOT deserialize the original tree or load a
// retail sound configuration from disk. Those remain separate blockers.
class OriginalAudioDecodedAttributeSource final:public OriginalAudioDescriptorSource {
public:
    OriginalAudioDecodedAttributeSource(
        std::span<const std::byte> originalBuffer,
        const OriginalAttributeKeyNode* originalDecodedIndex) noexcept
        : buffer_(originalBuffer),index_(originalDecodedIndex){}
    std::string originalStringField_2bb3bc(
        std::string_view key,std::string_view fallback) override {
        return originalAttributeString_2bb3bc(buffer_,index_,key,fallback);
    }
    float originalFloatField_2bb1d0(
        std::string_view key,float fallback) override {
        return originalAttributeFloat_2bb1d0(buffer_,index_,key,fallback);
    }
private:
    std::span<const std::byte> buffer_;
    const OriginalAttributeKeyNode* index_;
};

inline OriginalAudioDescriptorValues originalAudioDescriptorFromDecodedAttributes_1bb9a8(
    std::span<const std::byte> originalLoadedBytes,
    const OriginalAttributeKeyNode* originalDecodedIndex) {
    OriginalAudioDecodedAttributeSource source(originalLoadedBytes,originalDecodedIndex);
    return originalAudioDescriptorFields_1bb9a8(source);
}
} // namespace ae3
