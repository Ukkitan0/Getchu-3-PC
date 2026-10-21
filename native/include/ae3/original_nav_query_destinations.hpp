#pragma once
#include "ae3/original_nav_query_selection.hpp"
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

namespace ae3 {
// Original 0x220F60: minimum centroid XYZ distance to the query
// point; initializes best to FLT_MAX; strict less-than retains the
// earliest search slot in a tie. Returns -1 when no candidate exists.
inline std::int32_t originalNavNearestQueryNode_220f60(
    std::span<const OriginalNavSearchNode_220c00> nodes,
    const OriginalVU0Vec4& queryPoint) noexcept {
    std::int32_t chosen=-1;
    float best=std::numeric_limits<float>::max();
    for(std::size_t i=0;i<nodes.size();++i){
        const float d=originalVU0DistanceXYZ_21b234(nodes[i].originalCentroid,queryPoint);
        if(d<best){best=d;chosen=static_cast<std::int32_t>(i);}
    }
    return chosen;
}
// Original 0x221030: maximum centroid XYZ distance to the query.
// Initializes best to -FLT_MAX and updates for strictly greater.
inline std::int32_t originalNavFarthestQueryNode_221030(
    std::span<const OriginalNavSearchNode_220c00> nodes,
    const OriginalVU0Vec4& queryPoint) noexcept {
    std::int32_t chosen=-1;
    float best=-std::numeric_limits<float>::max();
    for(std::size_t i=0;i<nodes.size();++i){
        const float d=originalVU0DistanceXYZ_21b234(nodes[i].originalCentroid,queryPoint);
        if(best<d){best=d;chosen=static_cast<std::int32_t>(i);}
    }
    return chosen;
}
// The original 0x2208A8 and 0x2208B4 switch on output mode:
//   0 -> 0x221030 farthest; 1 -> 0x220F60 nearest;
//   other -> separate caller-provided original $8 search index to 0x220994.
inline std::int32_t originalNavDestinationForMode_2208a8(
    std::span<const OriginalNavSearchNode_220c00> nodes,
    const OriginalVU0Vec4& point,std::int32_t originalMode,
    std::int32_t explicitSearchIndex=-1) {
    if(originalMode==0)return originalNavFarthestQueryNode_221030(nodes,point);
    if(originalMode==1)return originalNavNearestQueryNode_220f60(nodes,point);
    // Source 0x2208B8 passes independent $8, not mode $9.
    if(explicitSearchIndex==-1)return -1;
    if(explicitSearchIndex<0 || std::size_t(explicitSearchIndex)>=nodes.size())
        throw std::out_of_range("Original navigation explicit search index invalid");
    return explicitSearchIndex;
}
inline std::vector<std::uint32_t> originalNavDestinationPath_2208a8(
    std::span<const OriginalNavSearchNode_220c00> nodes,
    const OriginalVU0Vec4& point,std::int32_t outputMode,
    std::int32_t explicitSearchIndex=-1) {
    return originalNavExtractPath_220994(nodes,
        originalNavDestinationForMode_2208a8(nodes,point,outputMode,explicitSearchIndex));
}
} // namespace ae3
