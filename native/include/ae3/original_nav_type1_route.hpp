#pragma once
#include "ae3/original_stage_spatial_loader.hpp"
#include "ae3/original_vu0_geometry.hpp"
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace ae3 {
// The original source 0x21B194 VU0 polyline math was already recovered
// in native/src/original_vu0_geometry.cpp. This adapter brings the
// *real source-derived type-1 child bytes* from 0x21D70C into that
// implementation; do not duplicate/reinterpret the VU0 arithmetic.
inline float originalNavType1ChildDistance_21b194(
    const OriginalNavType1Child_21b0b0& child) {
    static_assert(sizeof(OriginalVU0Vec4)==16);
    static_assert(std::is_trivially_copyable_v<OriginalVU0Vec4>);
    static_assert(std::endian::native==std::endian::little,
        "Original USA R5900 vectors store little-endian float32 lanes");
    std::vector<OriginalVU0Vec4> sourceVectors;
    sourceVectors.reserve(child.originalVectors.size());
    for(const auto& original16:child.originalVectors){
        OriginalVU0Vec4 v{};
        std::memcpy(&v,original16.data(),sizeof(v));
        sourceVectors.push_back(v);
    }
    return originalVU0PolylineLength_21b194(sourceVectors);
}
// Original route traversal 0x220F18 uses record+0x04+4*actionIndex
// to obtain its type-1 child via 0x21B4E0, and then calls 0x21B194.
// Native child ownership is held by OriginalStageSpatialLoaded. Avoid
// original 32-bit guest pointer reinterpretation when selecting it.
inline float originalNavType1ActionDistance_220f28(
    const OriginalStageSpatialLoaded& graph,
    std::uint32_t originalRecordIndex,std::uint16_t originalActionIndex) {
    if(originalRecordIndex>=graph.records.size() || originalActionIndex>=3)
        throw std::out_of_range("Original NavGraph action record out of range");
    for(const auto& child:graph.originalType1Children)
        if(child.sourceRecordIndex==originalRecordIndex &&
           child.sourceActionIndex==originalActionIndex)
            return originalNavType1ChildDistance_21b194(child);
    throw std::out_of_range("Original NavGraph type-1 child absent");
}
// Native semantic representation of the two route fields that the
// original 0x220E8C..0x220EAC path relaxation writes into its 0x38-byte
// search record: +0x30 predecessor, +0x34 best distance.
struct OriginalNavRouteCost_220e8c {
    std::uint32_t originalPredecessorAt30=0;
    float originalDistanceAt34=0.f;
};
inline bool originalNavRelaxCandidate_220e8c(
    OriginalNavRouteCost_220e8c& destination,float originalCurrentDistance,
    float originalEdgeDistance,std::uint32_t originalPredecessor) noexcept {
    const float candidate=originalCurrentDistance+originalEdgeDistance;
    if(!(candidate<destination.originalDistanceAt34))return false;
    destination.originalDistanceAt34=candidate;
    destination.originalPredecessorAt30=originalPredecessor;
    return true;
}
// Original 0x220F18..0x220F34 type-1 cost branch; source 0x220E8C
// performs the shared candidate test and conditional predecessor write.
inline bool originalNavType1Relax_220f18(
    const OriginalStageSpatialLoaded& graph,
    std::uint32_t originalRecordIndex,std::uint16_t originalActionIndex,
    float sourceDistance,OriginalNavRouteCost_220e8c& destination,
    std::uint32_t originalPredecessor) {
    return originalNavRelaxCandidate_220e8c(destination,sourceDistance,
        originalNavType1ActionDistance_220f28(graph,originalRecordIndex,originalActionIndex),
        originalPredecessor);
}
// Original 0x220EC8 direct-cost branch reuses the *existing* native
// original 0x21B468 VU0 centroid and 0x21B234 distance functions.
// Point coordinates come from the already decoded retail 0x70 record.
inline bool originalNavCentroidRelax_220ec8(
    std::span<const OriginalVU0Vec4> originalPolygonPoints,
    const OriginalVU0Vec4& originalCurrentPoint,float sourceDistance,
    OriginalNavRouteCost_220e8c& destination,std::uint32_t predecessor) {
    const auto center=originalVU0Mean_21b468(originalPolygonPoints);
    const float edge=originalVU0DistanceXYZ_21b234(center,originalCurrentPoint);
    return originalNavRelaxCandidate_220e8c(destination,sourceDistance,edge,predecessor);
}
} // namespace ae3
