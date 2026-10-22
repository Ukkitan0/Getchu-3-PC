#pragma once
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace ae3 {
// The original 0x24D4F0 walks [owner+0, owner+4) in 0x10-byte
// source-entry strides. It compares the requested full name to the
// string object at entry+0; on match it returns the pointer at entry+8.
// A mismatch advances by exactly 16; exhaustion returns null.
// This is a host-native *view* of the recovered lookup control flow,
// not a guessed stage filename map, guest-address allocation, or
// an implementation of the original object's string representation.
struct OriginalStageNamedEntry16_24d4f0 {
    std::string_view sourceName;
    std::span<const std::byte> borrowedOriginalPayload;
    std::size_t originalSourceIndex=0;
};
inline const OriginalStageNamedEntry16_24d4f0* originalStageNamedLookup_24d4f0(
    std::span<const OriginalStageNamedEntry16_24d4f0> originalEntries,
    std::string_view requestedOriginalName) noexcept {
    for(const auto& item:originalEntries)
        if(item.sourceName==requestedOriginalName)
            return &item;
    return nullptr;
}
} // namespace ae3
