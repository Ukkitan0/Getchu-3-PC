#pragma once
#include "ae3/original_spatial_raw_bridge.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace ae3 {
// Original 0x220C00/0x220D5C search nodes occupy 0x38 guest bytes.
// These native fields preserve known original offsets and control flow
// without pretending host pointers are 32-bit original-game pointers.
struct OriginalNavRouteNode38 {
    const OriginalSpatialRecord70* polygon04=nullptr;
    bool visited00=false;
    std::uint32_t linkCount08=0;
    std::array<std::int32_t,3> destinationIndices0c{{-1,-1,-1}};
    std::array<std::uint32_t,3> linkTypes18{};
    std::array<std::uint32_t,3> sourceLinkIndices24{};
    std::int32_t predecessor30=-1;
    float cost34=3.4028234663852886e38f;
    OriginalVU0Vec4 centroid{};
};
struct OriginalNavRouteWorkspace {
    std::vector<OriginalNavRouteNode38> nodes;
};
class OriginalNavRouteSplineCalls {
public:
    virtual ~OriginalNavRouteSplineCalls()=default;
    // Original type-1 edge: 0x21B4E0 loads source path by slot,
    // and original 0x21B194 computes its actual VU0 polyline length.
    virtual float call_original_21b4e0_21b194(
        const OriginalSpatialRecord70& from,std::uint32_t originalSlot)=0;
};
// Original source descriptor adjacency -> original 0x38-node link fields.
// Populates every original polygon; the original recursive 0x220AA4
// workspace admission/filtering behavior is still unresolved.
OriginalNavRouteWorkspace originalNavRouteBuildSourceWorkspace_220c00(
    std::span<const OriginalSpatialRecord70> originalRecords,
    std::span<const std::byte> originalNavBytes);
std::int32_t originalNavRouteFindRecord_220d14(
    const OriginalNavRouteWorkspace&,
    const OriginalSpatialRecord70* originalPolygon) noexcept;
// Translate original lowest-cost unvisited selection and strict
// relaxation, including original root initialization and edge types.
void originalNavRouteRelax_220d5c(
    OriginalNavRouteWorkspace&,
    OriginalNavRouteSplineCalls* unresolvedType1=nullptr);
// Original 0x220F60 / 0x221030 nearest/farthest original XYZ centroid.
std::int32_t originalNavRouteNearest_220f60(
    const OriginalNavRouteWorkspace&,const OriginalVU0Vec4&) noexcept;
std::int32_t originalNavRouteFarthest_221030(
    const OriginalNavRouteWorkspace&,const OriginalVU0Vec4&) noexcept;
// Original 0x220994 reconstructs original pointer path from
// predecessor+0x30 back to record zero and writes forward order.
std::vector<const OriginalSpatialRecord70*> originalNavRouteExtract_220994(
    const OriginalNavRouteWorkspace&,std::int32_t destination);
} // namespace ae3
