#pragma once
#include "ae3/original_stage_spatial_loader.hpp"
#include "ae3/original_polygon_raycast.hpp"
#include <vector>
namespace ae3 {
// Original source 0x21B3F8/0x21B444/0x21B404 adjacency header
// directly from the same 0x1E-byte descriptors already consumed
// by originalStageBuildSourceSpatial_21d46c.
std::vector<OriginalPolygonLinks_21bd3c> originalStagePolygonLinks_21bd3c(
    const OriginalStageSpatialSource& originalSource,
    const OriginalStageSpatialLoaded& originalStage);

// End-to-end original game stage bytes and original 0x70-record
// registry into recovered native ray -> source polygon clipping.
// Caller retains originalStage, so returned record pointers remain valid.
OriginalPolygonRaycastResult_4c1b3c originalStagePolygonRaycast_4c1b3c(
    const OriginalStageSpatialSource& originalSource,
    const OriginalStageSpatialLoaded& originalStage,
    std::uint32_t originalStagePolygonIndex,
    const OriginalVU0Vec4& originalCallerRayStart,
    const OriginalVU0Vec4& originalCallerRayEnd,
    std::size_t nativeMaxRecursions=4096);
} // namespace ae3
