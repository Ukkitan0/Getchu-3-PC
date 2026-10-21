#pragma once
#include "ae3/original_spatial_raw_bridge.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace ae3 {
// Authentic 0x21D46C source stage header. This is the original loaded
// stage-data block, not an ISO/R5900 memory simulator.
struct OriginalStageSpatialSource {
    std::uint32_t runtimeBaseVA = 0;
    std::span<const std::byte> loadedBytes;
    std::uint32_t originalRegistryOwnerVA = 0; // original 0x21B254 +0 owner
};
struct OriginalStageSpatialOffsets {
    std::uint32_t countAt24=0;
    // Six original source-relative offsets at 0x38..0x4C.
    std::array<std::uint32_t,6> relativeOffsets{};
    std::array<std::uint32_t,6> resolvedVAs{};
};
// Preserve unfinished constructor branches without inventing behavior.
struct OriginalStageSpatialAction {
    std::uint32_t sourceRecordIndex=0;
    std::uint16_t actionIndex=0;
    std::uint16_t type=0;
    std::uint16_t argument=0;
};
struct OriginalStageSpatialLoaded {
    OriginalStageSpatialOffsets sourceHeader{};
    std::vector<OriginalSpatialRecord70> records;
    std::vector<OriginalSpatialHeaderBinding> polygonHeaders;
    std::vector<OriginalStageSpatialAction> pendingOriginalActions;
};

// Real 0x21D46C stage source pointers and source record creation.
// 0x36EDAC game-owned spatial allocator, original object constructor
// 0x21B254 and action types 1/2 still need further game-side recovery;
// no engine object or script callback is fabricated for those actions.
OriginalStageSpatialOffsets originalStageReadSpatialOffsets_21d46c(
    const OriginalStageSpatialSource&);
OriginalStageSpatialLoaded originalStageBuildSourceSpatial_21d46c(
    const OriginalStageSpatialSource&);
// End-to-end source-stage record loader -> native original VU0 math.
const OriginalSpatialRecord70* originalStageNearestGrid_21d898(
    const OriginalStageSpatialLoaded&,
    const OriginalSpatialGridFields&,float originalGridX,float originalGridY,
    const OriginalVU0Vec4& originalQuery);
const OriginalSpatialRecord70* originalStageNearestAll_21da2c(
    const OriginalStageSpatialLoaded&,const OriginalVU0Vec4& originalQuery);
} // namespace ae3
