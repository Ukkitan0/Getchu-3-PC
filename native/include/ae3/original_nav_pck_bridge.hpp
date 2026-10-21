#pragma once
#include "ae3/original_navgraph_loader.hpp"
#include "ae3/original_resource_entry_vector.hpp"
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace ae3 {
// Original 0x4F0A5C requests literal "nav" from 0x26DF38, which
// jumps to the 0x26D24C type-filtered source PCK catalog builder.
// The same 16-byte resource entries are consumed by 0x26D214.
// Unlike originalNavReadPackage, this entry point receives the
// *already-decompressed* PCK allocation (e.g. original woods_c file).
inline std::vector<OriginalNavSource> originalNavReadNativePck_4f0a5c(
        std::span<const std::byte> originalPckAllocation) {
    auto selected=originalFilterResourceEntriesFromPck_26d24c(originalPckAllocation,"nav");
    std::vector<OriginalNavSource> result;
    result.reserve(selected.originalCount_26d120());
    for(const auto& entry:selected.entries()) {
        OriginalNavSource source;
        source.originalFilename=entry.sourceName;
        // 0x4F0AE8 uses the resource NAME, not its type, as input
        // to original 0x4F0FBC slot mapping (0 monkey,1 enemy,2 other).
        source.originalSlot=originalNavGraphSlot_4f0fbc(entry.sourceName);
        source.originalNavBytes.reserve(entry.sourcePayload.size());
        for(auto value:entry.sourcePayload)
            source.originalNavBytes.push_back(std::to_integer<std::uint8_t>(value));
        result.push_back(std::move(source));
    }
    return result;
}
struct OriginalNavRuntimeSourceBinding_4f0a5c {
    std::string_view originalName;
    std::uint32_t originalSourceRuntimeVA=0;
    std::uint32_t originalGraphOwnerVA=0;
};
// Native binding of the recovered source constructor to real bytes.
// The actual runtime allocation VA and registry-owner VA MUST be
// supplied by the real stage lifecycle. This routine refuses unknown
// identities and never fabricates original game memory pointers.
inline std::vector<OriginalStageSpatialLoaded> originalNavBuildNativePck_4f0a5c(
        const std::vector<OriginalNavSource>& sources,
        std::span<const OriginalNavRuntimeSourceBinding_4f0a5c> sourceBindings) {
    if(sources.size()!=sourceBindings.size())
        throw std::invalid_argument("Original nav source allocations missing");
    std::vector<OriginalStageSpatialLoaded> out;
    out.reserve(sources.size());
    for(std::size_t i=0;i<sources.size();++i) {
        const auto& source=sources[i];
        const auto& binding=sourceBindings[i];
        if(source.originalFilename!=binding.originalName ||
           !binding.originalSourceRuntimeVA || !binding.originalGraphOwnerVA)
            throw std::invalid_argument("Original nav source allocation identity mismatch");
        out.push_back(originalNavLoadGraph_4f0a5c(
            source,binding.originalSourceRuntimeVA,binding.originalGraphOwnerVA));
    }
    return out;
}
} // namespace ae3
