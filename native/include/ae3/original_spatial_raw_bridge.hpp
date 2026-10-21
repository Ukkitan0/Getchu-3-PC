#pragma once
#include "ae3/original_spatial_support.hpp"
#include "ae3/original_vu0_geometry.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace ae3 {
// Original 0x21B468 and 0x21B788 obtain vertex count through
// *(record+0x30)+0x16. The guest pointer is a source reference, NOT a
// host pointer or a simulated CPU address. The owner's native asset
// loader resolves it to an actual original header and count.
struct OriginalSpatialHeaderBinding {
    std::uint32_t originalHeaderVA = 0;
    std::uint16_t sourcePointCountAt16 = 0;
};
struct OriginalRawSpatialRecordInfo {
    std::uint32_t originalHeaderPointer30 = 0;
    std::uint16_t originalPointCount = 0;
};

// Extract source pointer and the three 16-byte vector slots directly
// from the original 0x70-byte record. No substitute geometry is created.
std::uint32_t originalSpatialHeaderPointer_21b468(
    const OriginalSpatialRecord70&) noexcept;
OriginalRawSpatialRecordInfo originalSpatialValidateHeader_21b468(
    const OriginalSpatialRecord70&, const OriginalSpatialHeaderBinding&);
std::array<OriginalVU0Vec4,3> originalSpatialVertices_21b468(
    const OriginalSpatialRecord70&);

// The source asset loader supplies actual original allocated data, each
// tagged with its source runtime VA. We resolve the original record's LW
// pointer at +0x30 to its header's LHU at +0x16. This does not emulate
// the PS2 heap; it only validates and reads original serialized bytes.
struct OriginalSpatialSourceAllocation {
    std::uint32_t originalBaseVA=0;
    std::span<const std::byte> originalBytes;
};
std::vector<OriginalSpatialHeaderBinding> originalSpatialResolveHeaders_21b468(
    std::span<const OriginalSpatialRecord70> sourceRecords,
    std::span<const OriginalSpatialSourceAllocation> originalAllocations);

// Owns VU0 vertex values decoded from authentic record bytes and views
// over them. Noncopyable so source span pointers cannot become dangling.
class OriginalResolvedSpatialGeometry {
public:
    OriginalResolvedSpatialGeometry(
        std::span<const OriginalSpatialRecord70> sourceRecords,
        std::span<const OriginalSpatialHeaderBinding> sourceHeaders);
    OriginalResolvedSpatialGeometry(const OriginalResolvedSpatialGeometry&)=delete;
    OriginalResolvedSpatialGeometry& operator=(const OriginalResolvedSpatialGeometry&)=delete;
    OriginalResolvedSpatialGeometry(OriginalResolvedSpatialGeometry&&)=delete;
    OriginalResolvedSpatialGeometry& operator=(OriginalResolvedSpatialGeometry&&)=delete;
    std::span<const OriginalSpatialSourcePolygon> polygons() const noexcept {return polygons_;}
    std::span<const OriginalRawSpatialRecordInfo> headers() const noexcept {return headers_;}
private:
    std::vector<std::array<OriginalVU0Vec4,3>> points_;
    std::vector<OriginalRawSpatialRecordInfo> headers_;
    std::vector<OriginalSpatialSourcePolygon> polygons_;
};

// No manually assembled per-record polygon or manually supplied counts;
// only original source record bytes and owner-loaded original allocations.
const OriginalSpatialRecord70* originalSpatialNearestOriginalDataGrid_21d898(
    std::span<const OriginalSpatialRecord70> sourceRecords,
    std::span<const OriginalSpatialSourceAllocation> originalAllocations,
    const OriginalSpatialGridFields& sourceGrid,float resolvedGridX,float resolvedGridY,
    const OriginalVU0Vec4& sourceQuery);
const OriginalSpatialRecord70* originalSpatialNearestOriginalDataAll_21da2c(
    std::span<const OriginalSpatialRecord70> sourceRecords,
    std::span<const OriginalSpatialSourceAllocation> originalAllocations,
    const OriginalVU0Vec4& sourceQuery);

// Connected actual flow: original record+0x30 resolved header count ->
// original record+0x40 VF lanes -> source plane/centroid/distance ->
// original grid-index or contiguous nearest-selector control flow.
const OriginalSpatialRecord70* originalSpatialNearestRawGeometry_21d898(
    std::span<const OriginalSpatialRecord70> sourceRecords,
    std::span<const OriginalSpatialHeaderBinding> sourceHeaders,
    OriginalSpatialGridHits sourceHits,
    const OriginalVU0Vec4& sourceQuery);
const OriginalSpatialRecord70* originalSpatialNearestRawGrid_21d898(
    std::span<const OriginalSpatialRecord70> sourceRecords,
    std::span<const OriginalSpatialHeaderBinding> sourceHeaders,
    const OriginalSpatialGridFields& originalGrid,
    float originalGridX,float originalGridY,
    const OriginalVU0Vec4& originalQuery);
const OriginalSpatialRecord70* originalSpatialNearestRawAll_21da2c(
    std::span<const OriginalSpatialRecord70> sourceRecords,
    std::span<const OriginalSpatialHeaderBinding> sourceHeaders,
    const OriginalVU0Vec4& originalQuery);
} // namespace ae3
