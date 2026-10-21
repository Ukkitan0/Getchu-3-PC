#pragma once
#include "ae3/original_stage_spatial_loader.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace ae3 {
// Source 0x21D6DC..0x21D708 handles action type 2. The original
// 0x20-byte source table resides at stage+original offset(+0x44).
// Each action argument is an unsigned 16-bit table index; the
// resolved pointer is saved at record+0x10+4*actionIndex.
struct OriginalNavSourceReference_21d6dc {
    std::uint32_t sourceRecordIndex=0;
    std::uint16_t sourceActionIndex=0;
    std::uint16_t sourceTableIndex=0;
    std::uint32_t originalSourceRecordVA=0;
};
inline std::vector<OriginalNavSourceReference_21d6dc> originalNavResolveType2_21d6dc(
        const OriginalStageSpatialSource& source,OriginalStageSpatialLoaded& built) {
    std::vector<OriginalNavSourceReference_21d6dc> resolved;
    std::vector<OriginalStageSpatialAction> stillPending;
    resolved.reserve(built.pendingOriginalActions.size());
    stillPending.reserve(built.pendingOriginalActions.size());
    for(const auto& action:built.pendingOriginalActions) {
        if(action.type!=2) {stillPending.push_back(action);continue;}
        if(action.sourceRecordIndex>=built.records.size() || action.actionIndex>=3)
            throw std::out_of_range("Original nav action exceeds 0x70 record slots");
        const std::uint64_t byteOffset=std::uint64_t(built.sourceHeader.relativeOffsets[3])+
            std::uint64_t(action.argument)*0x20u;
        if(byteOffset>source.loadedBytes.size() || source.loadedBytes.size()-byteOffset<0x20)
            throw std::out_of_range("Original nav action source table entry outside allocation");
        const std::uint64_t va=std::uint64_t(source.runtimeBaseVA)+byteOffset;
        if(!source.runtimeBaseVA || va>std::numeric_limits<std::uint32_t>::max())
            throw std::out_of_range("Original nav source VA missing or overflowed");
        resolved.push_back({action.sourceRecordIndex,action.actionIndex,action.argument,
            static_cast<std::uint32_t>(va)});
    }
    // Do not alter the output on invalid input. All refs were checked.
    for(const auto& ref:resolved) {
        auto& fields=built.records[ref.sourceRecordIndex].originalFields;
        const std::size_t at=0x10u+std::size_t(ref.sourceActionIndex)*4u;
        for(unsigned k=0;k<4;++k)
            fields[at+k]=std::byte((ref.originalSourceRecordVA>>(8*k))&0xffu);
    }
    built.pendingOriginalActions=std::move(stillPending);
    return resolved;
}
} // namespace ae3
