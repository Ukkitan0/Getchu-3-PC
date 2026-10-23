#include "ae3/ae3_unreal_lua_bridge.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>

namespace {
using Record=std::array<std::uint8_t,0x70>;
constexpr std::uint32_t originalHeaderBase=0x00500000;
void write32(Record& record,std::size_t at,std::uint32_t value) {
    for (unsigned n=0;n<4;++n)
        record.at(at+n)=static_cast<std::uint8_t>(value>>(8u*n));
}
void installOriginalPolygon(Record& record,std::uint32_t headerVA,
                            float baseX) {
    write32(record,0x30,headerVA);
    const std::array<Ae3OriginalVec4,3> originalSourceVertices{{
        {baseX,0,0,1}, {baseX,0,2,1}, {baseX+2,0,0,1}
    }};
    std::memcpy(record.data()+0x40,originalSourceVertices.data(),
                sizeof(originalSourceVertices));
}
void expectedFailure(Ae3OriginalManagerInitSession* session,
                     const Ae3OriginalRawNearestSourceV1& source,
                     Ae3OriginalNearestPolygonResultV1& result) {
    result.original_polygon_index=1234;
    assert(ae3_original_polygon_nearest_original_data_all_21da2c(
               session,&source,&result)==0);
    assert(result.original_polygon_index==1234);
    assert(std::strlen(ae3_original_manager_init_last_error(session))>0);
}
} // namespace

int main() {
    std::array<Record,3> originalRecords{};
    std::array<std::uint8_t,3*0x20> originalHeaders{};
    for (std::size_t i=0;i<originalRecords.size();++i) {
        const auto originalVA=originalHeaderBase+
            static_cast<std::uint32_t>(i*0x20);
        installOriginalPolygon(originalRecords[i],originalVA,
                               i==1 ? 9.0f:0.0f);
        // Original 0x21B468: source header pointer record+0x30;
        // LHU +0x16; source allocations are real VA-tagged bytes.
        originalHeaders[i*0x20+0x16]=3;
        originalHeaders[i*0x20+0x17]=0;
    }
    Ae3OriginalSourceAllocationV1 sourceAllocation{
        originalHeaderBase,originalHeaders.data(),originalHeaders.size()};
    Ae3OriginalRawNearestSourceV1 source{
        originalRecords.data(),originalRecords.size(),
        &sourceAllocation,1,{0.5f,0,0.5f,4000.0f}};
    auto* session=ae3_original_manager_init_create(5489);
    assert(session);
    Ae3OriginalNearestPolygonResultV1 result{1234};

    // 0x21DA2C is the registry scan, independent of 0x21B788
    // eligibility and the grid. First source-order tie wins.
    assert(ae3_original_polygon_nearest_original_data_all_21da2c(
               session,&source,&result)==1);
    assert(result.original_polygon_index==0);

    const std::array<std::uint16_t,2> originalCellOffsets{0,4};
    const std::array<std::uint16_t,8> originalHitPool{
        2,1,2,0, 1,0,0,0};
    Ae3OriginalRawGridNearestSourceV1 grid{};
    grid.original=source;
    grid.original_origin_x00=0;grid.original_origin_y04=0;
    grid.original_step_x10=1;grid.original_step_y14=1;
    grid.original_dimension_x18=2;grid.original_dimension_y1c=1;
    grid.original_cell_offsets50=originalCellOffsets.data();
    grid.original_cell_offset_count=originalCellOffsets.size();
    grid.original_hit_pool08=originalHitPool.data();
    grid.original_hit_pool_count=originalHitPool.size();
    grid.original_resolved_x=0;grid.original_resolved_y=0;
    // First grid list is [1,2]; source original 0x21B788 rejects 1,
    // returns 2, using genuine +0x30 -> header +0x16 -> vertices.
    assert(ae3_original_polygon_nearest_original_data_grid_21d898(
               session,&grid,&result)==1);
    assert(result.original_polygon_index==2);
    grid.original_resolved_x=1;
    assert(ae3_original_polygon_nearest_original_data_grid_21d898(
               session,&grid,&result)==1);
    assert(result.original_polygon_index==0);

    // Unlike the older ABI, these calls have no caller-invented vertex
    // counts. A malformed ORIGINAL header must reject instead.
    originalHeaders[0x16]=0;
    expectedFailure(session,source,result);
    originalHeaders[0x16]=4;
    expectedFailure(session,source,result);
    originalHeaders[0x16]=3;

    // Detect missing source header ownership, never reinterpret a
    // guest address directly as a Windows or Unreal pointer.
    auto saved=originalRecords[0];
    write32(originalRecords[0],0x30,0xDEADBEEF);
    expectedFailure(session,source,result);
    originalRecords[0]=saved;

    // Two matching guest VA allocations are ambiguous and must reject,
    // not silently attach geometry to whichever the host found first.
    std::array<Ae3OriginalSourceAllocationV1,2> overlapping{
        sourceAllocation,sourceAllocation};
    source.original_allocations=overlapping.data();
    source.original_allocation_count=overlapping.size();
    expectedFailure(session,source,result);
    source.original_allocations=&sourceAllocation;
    source.original_allocation_count=1;

    // A malformed original 0x4E3C14 cell offset fails before
    // publishing any target. A no-hit original cell returns UINT32_MAX.
    std::array<std::uint16_t,2> invalidOffsets=originalCellOffsets;
    invalidOffsets[0]=65535;
    grid.original_cell_offsets50=invalidOffsets.data();
    grid.original_resolved_x=0;
    result.original_polygon_index=1234;
    assert(ae3_original_polygon_nearest_original_data_grid_21d898(
               session,&grid,&result)==0);
    assert(result.original_polygon_index==1234);
    grid.original_cell_offsets50=originalCellOffsets.data();
    const std::array<std::uint16_t,3> noHits{0,0,0};
    grid.original_hit_pool08=noHits.data();
    grid.original_hit_pool_count=noHits.size();
    grid.original_resolved_x=0;
    assert(ae3_original_polygon_nearest_original_data_grid_21d898(
               session,&grid,&result)==1);
    assert(result.original_polygon_index==UINT32_MAX);
    grid.original_hit_pool08=originalHitPool.data();
    grid.original_hit_pool_count=originalHitPool.size();

    // Genuine VU0 polygon distances ignore W; XYZ NaN is rejected.
    source.original_query.w=std::numeric_limits<float>::quiet_NaN();
    assert(ae3_original_polygon_nearest_original_data_all_21da2c(
               session,&source,&result)==1);
    assert(result.original_polygon_index==0);
    source.original_query.x=std::numeric_limits<float>::quiet_NaN();
    expectedFailure(session,source,result);

    assert(ae3_original_manager_init_destroy(session)==1);
    std::cout<<"ORIGINAL_SOURCE_HEADER_GRID_NEAREST_HOST_ABI_PASSED\n";
    return 0;
}
