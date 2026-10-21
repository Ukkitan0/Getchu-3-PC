#pragma once
#include "ae3/original_audio_descriptor.hpp"
#include <bit>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>

namespace ae3 {
// Source 0x5419EC compares node+0x10 key against the query, descends
// right (+0x0C) when nodeKey < query, and left (+0x08) otherwise.
// These are decoded native nodes, never overlays of retail EE memory.
struct OriginalAttributeKeyNode {
    std::string_view originalKey;
    std::uint32_t sourceValueOffset=0;
    const OriginalAttributeKeyNode* left=nullptr;
    const OriginalAttributeKeyNode* right=nullptr;
};
inline const OriginalAttributeKeyNode* originalAttributeFind_5419ec(
    const OriginalAttributeKeyNode* sourceRoot,
    std::string_view originalQuery) noexcept {
    const OriginalAttributeKeyNode* candidate=nullptr;
    for(auto* node=sourceRoot;node!=nullptr;) {
        if(node->originalKey.compare(originalQuery)<0)
            node=node->right;
        else {
            candidate=node;
            node=node->left;
        }
    }
    if(candidate==nullptr||originalQuery.compare(candidate->originalKey)<0)
        return nullptr;
    return candidate;
}

// Both original numeric readers resolve a name, obtain a source-relative
// value offset, then check the first 32-bit value for 0x7FFFFFFF.
// The original 0x2BB194 sentinel means "use the caller's default".
inline constexpr std::uint32_t OriginalAttributeMissingWord_2bb194=0x7fffffffu;
inline std::uint32_t originalAttributeReadWord(
    std::span<const std::byte> originalSerializedBuffer,std::size_t offset) {
    if(offset>originalSerializedBuffer.size()||
       originalSerializedBuffer.size()-offset<4)
        throw std::out_of_range("Original serialized attribute word outside loaded allocation");
    return std::uint32_t(std::to_integer<unsigned char>(originalSerializedBuffer[offset])) |
        (std::uint32_t(std::to_integer<unsigned char>(originalSerializedBuffer[offset+1]))<<8) |
        (std::uint32_t(std::to_integer<unsigned char>(originalSerializedBuffer[offset+2]))<<16) |
        (std::uint32_t(std::to_integer<unsigned char>(originalSerializedBuffer[offset+3]))<<24);
}
inline std::optional<std::uint32_t> originalAttributeWord_2bb194(
    std::span<const std::byte> originalBuffer,
    const OriginalAttributeKeyNode* originalIndex,
    std::string_view name) {
    const auto* node=originalAttributeFind_5419ec(originalIndex,name);
    if(!node)return std::nullopt;
    const auto value=originalAttributeReadWord(originalBuffer,node->sourceValueOffset);
    if(value==OriginalAttributeMissingWord_2bb194)return std::nullopt;
    return value;
}
inline float originalAttributeFloat_2bb1d0(
    std::span<const std::byte> originalBuffer,
    const OriginalAttributeKeyNode* originalIndex,
    std::string_view name,float defaultValue) {
    const auto value=originalAttributeWord_2bb194(originalBuffer,originalIndex,name);
    return value?std::bit_cast<float>(*value):defaultValue;
}
inline std::uint32_t originalAttributeIntBits_2bb2b0(
    std::span<const std::byte> originalBuffer,
    const OriginalAttributeKeyNode* originalIndex,
    std::string_view name,std::uint32_t defaultValue) {
    const auto value=originalAttributeWord_2bb194(originalBuffer,originalIndex,name);
    return value.value_or(defaultValue);
}

// 0x2BB3BC's original string-object decoding remains unresolved:
// leave its required source callback intact. Connect only the recovered
// float path to the existing authentic audio descriptor initializer.
class OriginalAudioNumericAttributeBridge final:public OriginalAudioDescriptorSource {
public:
    OriginalAudioNumericAttributeBridge(
        OriginalAudioDescriptorSource& originalStringProvider,
        std::span<const std::byte> originalSerializedBuffer,
        const OriginalAttributeKeyNode* decodedAttributeIndex) noexcept
        : originalStringProvider_(originalStringProvider),
          originalBuffer_(originalSerializedBuffer),index_(decodedAttributeIndex){}
    std::string originalStringField_2bb3bc(
        std::string_view name,std::string_view fallback) override {
        return originalStringProvider_.originalStringField_2bb3bc(name,fallback);
    }
    float originalFloatField_2bb1d0(
        std::string_view name,float fallback) override {
        return originalAttributeFloat_2bb1d0(originalBuffer_,index_,name,fallback);
    }
private:
    OriginalAudioDescriptorSource& originalStringProvider_;
    std::span<const std::byte> originalBuffer_;
    const OriginalAttributeKeyNode* index_;
};
} // namespace ae3
