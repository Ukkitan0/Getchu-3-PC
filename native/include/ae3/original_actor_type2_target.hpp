#pragma once
#include "ae3/original_spatial_index.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace ae3 {
// Original 0x57BDB8..0x57BDFC: if the action kind is 2, the actor
// takes the SECOND polygon from its route, stores its pointer at
// controller+0x50 and calls 0x21B458 for vector slots 0, 1 and 2.
// The subsequent COP2 state/motion calculation is not yet recovered.
struct OriginalActorType2Target_57bdb8 {
    const OriginalSpatialRecord70* targetPolygonAt50=nullptr;
    std::array<std::array<std::byte,16>,3> rawOriginalVertices{};
};
OriginalActorType2Target_57bdb8 originalActorType2Target_57bdb8(
    std::span<const OriginalSpatialRecord70> sourceOriginalPolygonRecords,
    std::span<const std::uint32_t> sourceRoutePolygonIds);
} // namespace ae3
