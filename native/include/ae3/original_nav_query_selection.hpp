#pragma once
#include "ae3/original_nav_graph_search.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ae3 {
// Original 0x2207D4 allocates 0x3800 bytes: 256 slots of 0x38
// each. Original 0x220AA4 recursively selects source spatial records
// based on the centroid-to-query XYZ distance and follows the polygon
// adjacency in source order (original 0x21B3F8, 0x21B404).
// A native vector of source record indexes stands in for *pointers*
// written to original search nodes +0x04, never guest memory addresses.
struct OriginalNavSelection_220aa4 {
    std::vector<std::uint32_t> sourceRecordIndices;
};
// 0x21F89C supplies a separate optional engine-side spatial eligibility
// check when ctx+0x08 and A3 are both nonzero. In that mode caller MUST
// supply the reconstructed eligibility test; this function never assumes
// that all candidates pass it.
using OriginalNavEligibility_21f89c = std::function<bool(std::uint32_t)>;
inline OriginalNavSelection_220aa4 originalNavSelectByRadius_220aa4(
    const OriginalStageSpatialLoaded& sourceGraph,
    std::span<const std::byte> originalNavBytes,
    std::uint32_t startSourceRecord,
    const OriginalVU0Vec4& queryPoint,
    float radius,
    bool requireAdditionalEligibility = false,
    const OriginalNavEligibility_21f89c& eligibility = {}) {
    if(startSourceRecord >= sourceGraph.records.size())
        throw std::out_of_range("Original graph radius search starting node missing");
    if(requireAdditionalEligibility && !eligibility)
        throw std::invalid_argument("Original 0x21F89C eligibility not supplied");
    const auto base = std::uint64_t(sourceGraph.sourceHeader.relativeOffsets[1]);
    std::vector<unsigned char> attempted(sourceGraph.records.size(),0);
    OriginalNavSelection_220aa4 result;
    result.sourceRecordIndices.reserve((std::min)(sourceGraph.records.size(),std::size_t{256}));
    // A manually maintained DFS stack preserves source-order recursion
    // without overflowing the host call stack on malformed retail graphs.
    struct Frame { std::uint32_t id; std::uint16_t nextNeighbor; bool entered; };
    std::vector<Frame> stack{{startSourceRecord,0,false}};
    while(!stack.empty()) {
        auto& frame=stack.back();
        const auto id=frame.id;
        const auto desc=base+std::uint64_t(id)*0x1e;
        if(!frame.entered) {
            frame.entered=true;
            if(attempted[id]) {stack.pop_back();continue;}
            attempted[id]=1;
            // Source 0x220AF0 tests the per-graph generation marker
            // through 0x21B014; a selected source record is marked
            // at 0x220B8C before recursively visiting its neighbors.
            if(requireAdditionalEligibility && !eligibility(id)){
                stack.pop_back();continue;
            }
            const auto count=original_nav_search_detail::load16(originalNavBytes,desc+0x16);
            const auto center=original_nav_search_detail::sourceCentroid(sourceGraph.records[id],count);
            if(!(originalVU0DistanceXYZ_21b234(center,queryPoint)<radius)){
                stack.pop_back();continue;
            }
            if(result.sourceRecordIndices.size()>=256)
                throw std::length_error("Original 0x3800-byte navigation query workspace exhausted");
            result.sourceRecordIndices.push_back(id);
        }
        const auto degree=original_nav_search_detail::load16(originalNavBytes,desc+2);
        if(degree>3)throw std::out_of_range("Original 3-slot adjacency exceeded");
        if(frame.nextNeighbor>=degree){stack.pop_back();continue;}
        const auto next=original_nav_search_detail::load16(originalNavBytes,
            desc+4+2*frame.nextNeighbor++);
        if(next>=sourceGraph.records.size())
            throw std::out_of_range("Original navigation query neighbor out of range");
        if(!attempted[next])stack.push_back({next,0,false});
    }
    return result;
}
// Original 0x220994 counts the +0x30 predecessor chain and allocates
// one 32-bit spatial-record pointer per step, then writes pointers in
// forward route order. Native output is the corresponding SOURCE record
// IDs, rather than pretending to know runtime 32-bit PS2 addresses.
inline std::vector<std::uint32_t> originalNavExtractPath_220994(
    std::span<const OriginalNavSearchNode_220c00> nodes,
    std::int32_t destinationSearchIndex) {
    if(destinationSearchIndex==-1)return {};
    if(destinationSearchIndex<0 || std::size_t(destinationSearchIndex)>=nodes.size())
        throw std::out_of_range("Original navigation destination search index invalid");
    if(nodes[destinationSearchIndex].route.originalDistanceAt34==
       std::numeric_limits<float>::max())return {};
    std::vector<std::uint32_t> reverse;
    std::vector<unsigned char> seen(nodes.size(),0);
    auto index=static_cast<std::uint32_t>(destinationSearchIndex);
    for(;;){
        if(index>=nodes.size()||seen[index])
            throw std::invalid_argument("Original navigation predecessor chain invalid or cyclical");
        seen[index]=1;
        reverse.push_back(nodes[index].originalSpatialRecordIndex);
        if(index==0)break;
        index=nodes[index].route.originalPredecessorAt30;
    }
    return {reverse.rbegin(),reverse.rend()};
}
// Composes the original 0x2207D4 input selection, 0x220C00 search
// adjacency construction and 0x220D5C Dijkstra traversal. The original
// post-search modes at 0x220F60/0x221030 are intentionally separate.
struct OriginalNavQueryExecution_2207d4 {
    OriginalNavSelection_220aa4 selected;
    std::vector<OriginalNavSearchNode_220c00> nodes;
    OriginalNavSearchResult_220d5c traversal;
};
inline OriginalNavQueryExecution_2207d4 originalNavQueryAndSearch_2207d4(
    const OriginalStageSpatialLoaded& graph,
    std::span<const std::byte> raw,
    std::uint32_t startingPolygon,
    const OriginalVU0Vec4& point,
    float radius,
    bool requireAdditionalEligibility=false,
    const OriginalNavEligibility_21f89c& eligibility={}){
    auto selected=originalNavSelectByRadius_220aa4(graph,raw,startingPolygon,point,radius,
        requireAdditionalEligibility,eligibility);
    auto nodes=originalNavBuildSearchNodes_220c00(graph,raw,selected.sourceRecordIndices);
    auto outcome=originalNavExecuteSearch_220d5c(nodes,graph);
    return {std::move(selected),std::move(nodes),outcome};
}
} // namespace ae3
