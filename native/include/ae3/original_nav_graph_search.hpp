#pragma once
#include "ae3/original_nav_type1_route.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

namespace ae3 {
// Source 0x220C00 constructs a list of 0x38-byte search records.
// +0x00 visited; +0x04 source spatial record pointer; +0x08 adjacency
// count; +0x0C three neighbor indices; +0x18 three edge types;
// +0x24 three source action indices; +0x30 predecessor; +0x34 score.
// Native fields are typed, not binary-compatible guest pointers.
struct OriginalNavSearchEdge_220c00 {
    std::uint32_t neighborSearchIndex=0;
    std::uint16_t originalKind=0;
    std::uint16_t originalActionIndex=0;
};
struct OriginalNavSearchNode_220c00 {
    std::uint32_t originalSpatialRecordIndex=0;
    OriginalVU0Vec4 originalCentroid{};
    std::vector<OriginalNavSearchEdge_220c00> sourceEdges;
    bool visited=false;
    OriginalNavRouteCost_220e8c route{0xffffffffu,std::numeric_limits<float>::max()};
};
struct OriginalNavSearchResult_220d5c {
    std::size_t visitedCount=0;
    std::size_t reachableCount=0;
};
namespace original_nav_search_detail {
inline std::uint16_t load16(std::span<const std::byte> data,std::uint64_t off) {
    if(off>data.size() || data.size()-static_cast<std::size_t>(off)<2)
        throw std::out_of_range("Original NavGraph polygon source halfword truncated");
    const auto i=static_cast<std::size_t>(off);
    return static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(data[i]) |
         (std::uint16_t(std::to_integer<std::uint8_t>(data[i+1]))<<8));
}
inline OriginalVU0Vec4 sourceCentroid(const OriginalSpatialRecord70& rec,
                                     std::uint16_t vertexCount) {
    if(vertexCount==0 || vertexCount>3)
        throw std::out_of_range("Original 0x70 spatial record has unsupported vertex count");
    std::array<OriginalVU0Vec4,3> vertices{};
    static_assert(sizeof(OriginalVU0Vec4)==16);
    static_assert(std::endian::native==std::endian::little);
    for(unsigned i=0;i<vertexCount;++i)
        std::memcpy(&vertices[i],rec.originalFields.data()+0x40+16*i,16);
    return originalVU0Mean_21b468({vertices.data(),vertexCount});
}
} // namespace original_nav_search_detail
// Reconstructs original 0x220C00's lookup/index operation. The input
// source IDs select a subset of original graph records (original stage
// search only includes available records); missing neighboring IDs are
// intentionally not added, mirroring original 0x220D14's -1 lookup.
inline std::vector<OriginalNavSearchNode_220c00> originalNavBuildSearchNodes_220c00(
    const OriginalStageSpatialLoaded& originalGraph,
    std::span<const std::byte> originalNavBytes,
    std::span<const std::uint32_t> originalSourceIDs) {
    std::vector<OriginalNavSearchNode_220c00> nodes;
    nodes.reserve(originalSourceIDs.size());
    const auto records=originalGraph.records.size();
    const auto base=std::uint64_t(originalGraph.sourceHeader.relativeOffsets[1]);
    for(std::uint32_t id:originalSourceIDs) {
        if(id>=records)throw std::out_of_range("Original selected graph record index invalid");
        for(const auto& existing:nodes)
            if(existing.originalSpatialRecordIndex==id)
                throw std::invalid_argument("Original search record duplicated in selected subset");
        const auto position=base+std::uint64_t(id)*0x1e;
        const auto count=original_nav_search_detail::load16(originalNavBytes,position+0x16);
        OriginalNavSearchNode_220c00 node{};
        node.originalSpatialRecordIndex=id;
        node.originalCentroid=original_nav_search_detail::sourceCentroid(originalGraph.records[id],count);
        nodes.push_back(std::move(node));
    }
    for(auto& node:nodes) {
        const auto position=base+std::uint64_t(node.originalSpatialRecordIndex)*0x1e;
        const auto edgeCount=original_nav_search_detail::load16(originalNavBytes,position+2);
        // The original search node has exactly three slots per field.
        if(edgeCount>3)throw std::out_of_range("Original search node has more than three edge slots");
        node.sourceEdges.reserve(edgeCount);
        for(std::uint16_t j=0;j<edgeCount;++j) {
            const auto target=original_nav_search_detail::load16(originalNavBytes,position+4+2*j);
            const auto kind=original_nav_search_detail::load16(originalNavBytes,position+0x0a+2*j);
            if(target>=records)throw std::out_of_range("Original NavGraph adjacency points beyond source graph");
            // The original 0x220E7C..0x220F14 branches explicitly
            // cover classes 0, 1, 2. Other classes are not decoded.
            if(kind>2)throw std::invalid_argument("Unknown original navigation route edge class");
            for(std::size_t k=0;k<nodes.size();++k)
                if(nodes[k].originalSpatialRecordIndex==target) {
                    node.sourceEdges.push_back({static_cast<std::uint32_t>(k),kind,j});
                    break;
                }
        }
    }
    return nodes;
}
inline std::vector<OriginalNavSearchNode_220c00> originalNavBuildAllSearchNodes_220c00(
    const OriginalStageSpatialLoaded& graph,std::span<const std::byte> raw) {
    std::vector<std::uint32_t> ids;
    ids.reserve(graph.records.size());
    for(std::size_t i=0;i<graph.records.size();++i) {
        if(i>std::numeric_limits<std::uint32_t>::max())throw std::out_of_range("Original graph size exceeds source IDs");
        ids.push_back(static_cast<std::uint32_t>(i));
    }
    return originalNavBuildSearchNodes_220c00(graph,raw,ids);
}
// Actual 0x220D5C..0x220F38 lowest-unvisited-score traversal.
// On entry it sets the FIRST search node predecessor to -1 and cost 0.
// Other node costs/visited flags must come from the source search-node
// setup; original 0x220D5C doesn't reset them. The initialized native
// builder above uses source sentinel FLT_MAX and visited=false.
inline OriginalNavSearchResult_220d5c originalNavExecuteSearch_220d5c(
    std::span<OriginalNavSearchNode_220c00> nodes,
    const OriginalStageSpatialLoaded& originalGraph) {
    if(nodes.empty())return {};
    // Validate all topology and missing original type-1 child ownership
    // before mutating any route score. Host safety, not retail parity.
    for(const auto& node:nodes) {
        if(node.originalSpatialRecordIndex>=originalGraph.records.size())
            throw std::out_of_range("Original NavGraph search node missing source polygon");
        for(const auto& edge:node.sourceEdges) {
            if(edge.neighborSearchIndex>=nodes.size() || edge.originalActionIndex>=3 || edge.originalKind>2)
                throw std::out_of_range("Original NavGraph search edge invalid");
            if(edge.originalKind==1)
                (void)originalNavType1ActionDistance_220f28(originalGraph,
                    node.originalSpatialRecordIndex,edge.originalActionIndex);
        }
    }
    nodes.front().route.originalPredecessorAt30=0xffffffffu;
    nodes.front().route.originalDistanceAt34=0.f;
    OriginalNavSearchResult_220d5c result{};
    constexpr float unvisitedSentinel=std::numeric_limits<float>::max();
    for(;;) {
        float minimum=unvisitedSentinel;
        std::size_t selected=nodes.size();
        for(std::size_t i=0;i<nodes.size();++i) {
            // Original 0x220DC4 visited != 0 skips candidate.
            // Original C.OLT.S means ties favor the FIRST node.
            if(!nodes[i].visited && nodes[i].route.originalDistanceAt34<minimum) {
                minimum=nodes[i].route.originalDistanceAt34;
                selected=i;
            }
        }
        if(selected==nodes.size() || minimum==unvisitedSentinel)break;
        auto& current=nodes[selected];
        current.visited=true;
        ++result.visitedCount;
        for(const auto& edge:current.sourceEdges) {
            auto& neighbor=nodes[edge.neighborSearchIndex];
            if(neighbor.visited)continue;
            float distance;
            if(edge.originalKind==1) {
                distance=originalNavType1ActionDistance_220f28(originalGraph,
                    current.originalSpatialRecordIndex,edge.originalActionIndex);
            } else {
                // Original classes 0 and 2 go through the direct
                // centroid XYZ distance branch at 0x220EC8.
                distance=originalVU0DistanceXYZ_21b234(
                    current.originalCentroid,neighbor.originalCentroid);
            }
            (void)originalNavRelaxCandidate_220e8c(neighbor.route,
                current.route.originalDistanceAt34,distance,
                static_cast<std::uint32_t>(selected));
        }
    }
    for(const auto& node:nodes)
        if(node.route.originalDistanceAt34<unvisitedSentinel)++result.reachableCount;
    return result;
}
} // namespace ae3
