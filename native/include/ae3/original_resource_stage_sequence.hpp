#pragma once
#include "ae3/original_resource_stage_catalog.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace ae3 {
// USA retail source stage initialization: 0x22D7B4..0x22D970.
// These are four ORIGINAL lookup KEY strings at respective ELF
// literal addresses, and the original call sites/consumer VAs.
// An inferred filename extension is not part of these instructions.
struct OriginalStageInitLookup_22d7b4 {
    std::string_view sourceKey;
    std::uint32_t originalLiteralVA;
    std::uint32_t originalLookupCallVA;
    std::uint32_t originalConsumerCallVA;
    std::uint32_t originalConsumerVA;
};
// Upstream source stage registration: 0x22D7F8 calls wrapper 0x252BC4
// (which dispatches to 0x25182C) with A3 pointing to the literal "*"
// at original VA 0x6E7380. This occurs BEFORE the four resource-key
// lookups. This metadata does not reconstruct the object-container loader.
inline constexpr std::uint32_t OriginalStageWildcardLiteralVA_22d7f8 = 0x6e7380;
inline constexpr std::uint32_t OriginalStageWildcardSetupCall_22d7f8 = 0x22d7f8;
inline constexpr std::uint32_t OriginalStageWildcardWrapperVA_252bc4 = 0x252bc4;
inline constexpr std::uint32_t OriginalStageSourceRegistrationVA_25182c = 0x25182c;
inline constexpr std::string_view OriginalStageRegistrationWildcard_25182c = "*";

inline constexpr std::array<OriginalStageInitLookup_22d7b4,4>
    OriginalStageInitLookupOrder_22d7b4{{
        {"placeholder",0x6e7388,0x22d828,0x22d844,0x272294},
        {"navigation",0x6e7398,0x22d8c0,0x22d8c8,0x4f0a5c},
        {"actionseq",  0x6e73a8,0x22d8e8,0x22d904,0x30c798},
        {"monkeydb",   0x6e73b8,0x22d928,0x22d95c,0x25bfe0},
    }};

// This inventory is a source-EVIDENCE bridge, not a claim that stage
// keys map to extensions or that the original engine uses this heuristic.
// Candidate filenames are always exact names present in the real VFI
// catalog and are never synthesized. Only stage-root entries whose
// stem matches the source lookup key are reported.
struct OriginalStageLookupCandidates_22d7b4 {
    OriginalStageInitLookup_22d7b4 source;
    std::vector<std::size_t> actualVfiIndices; // indices into catalog.entries()
};
inline std::array<OriginalStageLookupCandidates_22d7b4,4>
originalStageLookupCandidates_22d7b4(
    const OriginalStageResourceCatalog& catalog, std::string_view sourceStage) {
    const auto stageIndices=catalog.entriesForStage(sourceStage);
    std::array<OriginalStageLookupCandidates_22d7b4,4> result{};
    for(std::size_t i=0;i<result.size();++i){
        result[i].source=OriginalStageInitLookupOrder_22d7b4[i];
        for(const auto index:stageIndices){
            const auto& entry=catalog.entries()[index];
            const auto name=originalResourceAsciiFold(entry.uncompressedName);
            if(name.find('/')!=std::string::npos)continue; // stage root only
            const auto period=name.find('.');
            if(period==std::string::npos)continue;
            if(std::string_view(name).substr(0,period)==result[i].source.sourceKey)
                result[i].actualVfiIndices.push_back(index);
        }
    }
    return result;
}
// Fail closed when an external asset binder requests "the" file for an
// original source key but the VFI evidence has none or multiple candidates.
inline const OriginalStageVfiResource& originalStageRequireUniqueCandidate_22d7b4(
    const OriginalStageResourceCatalog& catalog,
    const OriginalStageLookupCandidates_22d7b4& request) {
    if(request.actualVfiIndices.size()!=1)
        throw std::out_of_range("Original stage resource stem cannot be resolved uniquely from source evidence");
    const auto index=request.actualVfiIndices[0];
    if(index>=catalog.entries().size())
        throw std::out_of_range("Original source evidence index outside resource catalog");
    return catalog.entries()[index];
}
} // namespace ae3
