#pragma once
#include "ae3/original_stage_spatial_loader.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ae3 {
// Original 0x21D70C..0x21D7B8: an action of type 1 selects a
// uint16-counted list from stage header source+0x4C (index 5).
// The selected uint16 indexes designate 16-byte vectors in the
// stage source+0x40 (index 2) table. 0x21B0B0 allocates
// count*0x10 bytes, and 0x21B240 stores each original SQ vector.
// The native owner models exact source vectors; it does NOT fake
// the original 8-byte child pointer or its 32-bit allocation VA.
inline std::size_t
originalNavMaterializeType1_21d70c(const OriginalStageSpatialSource& source,
                                   OriginalStageSpatialLoaded& graph) {
    const auto& offsets=graph.sourceHeader.relativeOffsets;
    std::vector<OriginalNavType1Child_21b0b0> newlyDecoded;
    std::vector<OriginalStageSpatialAction> pending;
    newlyDecoded.reserve(graph.pendingOriginalActions.size());
    pending.reserve(graph.pendingOriginalActions.size());
    const auto read16=[&](std::uint64_t address)->std::uint16_t {
        if(address>source.loadedBytes.size() || source.loadedBytes.size()-address<2)
            throw std::out_of_range("Original type-1 nav list truncated");
        return std::uint16_t(std::to_integer<std::uint8_t>(source.loadedBytes[address]))|
            (std::uint16_t(std::to_integer<std::uint8_t>(source.loadedBytes[address+1]))<<8);
    };
    for(const auto& a:graph.pendingOriginalActions) {
        if(a.type!=1){pending.push_back(a);continue;}
        if(a.sourceRecordIndex>=graph.records.size() || a.actionIndex>=3)
            throw std::out_of_range("Original type-1 nav action record slot invalid");
        const std::uint64_t listPosition=std::uint64_t(offsets[5])+std::uint64_t(a.argument)*2;
        const std::uint16_t n=read16(listPosition);
        const std::uint64_t listEnd=listPosition+2ull+std::uint64_t(n)*2;
        if(listEnd>source.loadedBytes.size())
            throw std::out_of_range("Original type-1 child list exceeds source allocation");
        OriginalNavType1Child_21b0b0 child{};
        child.sourceRecordIndex=a.sourceRecordIndex;
        child.sourceActionIndex=a.actionIndex;
        child.sourceTableSelector=a.argument;
        child.originalVectors.reserve(n);
        for(std::uint32_t i=0;i<n;++i) {
            const std::uint16_t vectorID=read16(listPosition+2u+std::uint64_t(i)*2u);
            const std::uint64_t vectorOff=std::uint64_t(offsets[2])+std::uint64_t(vectorID)*16;
            if(vectorOff>source.loadedBytes.size()||source.loadedBytes.size()-vectorOff<16)
                throw std::out_of_range("Original type-1 nav vector reference outside source allocation");
            std::array<std::byte,16> originalVector{};
            for(std::size_t b=0;b<16;++b)originalVector[b]=source.loadedBytes[vectorOff+b];
            child.originalVectors.push_back(originalVector);
        }
        newlyDecoded.push_back(std::move(child));
    }
    // Stage all child allocations and validate every source range before
    // changing graph ownership. Retail EE has unchecked failure behavior.
    const auto decodedCount=newlyDecoded.size();
    graph.originalType1Children.insert(graph.originalType1Children.end(),
        std::make_move_iterator(newlyDecoded.begin()),std::make_move_iterator(newlyDecoded.end()));
    graph.pendingOriginalActions=std::move(pending);
    return decodedCount;
}
} // namespace ae3
