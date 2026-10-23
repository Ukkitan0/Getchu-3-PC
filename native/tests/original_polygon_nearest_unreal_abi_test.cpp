#include "ae3/ae3_unreal_lua_bridge.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>

namespace {
using OriginalPolygon=std::array<std::uint8_t,0x70>;
void installTriangle(OriginalPolygon& polygon,float baseX) {
    // Source 0x21B458: three 16-byte records at +0x40.
    // XZ ring orientation is identical to the VU0 selector regression.
    const std::array<Ae3OriginalVec4,3> originalVertices{{
        {baseX,0,0,1},{baseX,0,2,1},{baseX+2,0,0,1}
    }};
    std::memcpy(polygon.data()+0x40,originalVertices.data(),
                sizeof(originalVertices));
}
} // namespace

int main() {
    std::array<OriginalPolygon,3> records{};
    installTriangle(records[0],0);
    installTriangle(records[1],9);
    installTriangle(records[2],0);
    std::array<std::uint16_t,3> sourcePointCounts{3,3,3};
    std::array<std::uint16_t,3> originalGridHits{1,2,0};
    Ae3OriginalNearestPolygonSourceV1 request{};
    request.original_records70=records.data();
    request.original_record_count=records.size();
    request.original_point_counts=sourcePointCounts.data();
    request.original_hit_indices=originalGridHits.data();
    request.original_hit_count=originalGridHits.size();
    request.original_query={0.5f,0,0.5f,1000.0f};
    auto* session=ae3_original_manager_init_create(5489);
    assert(session);

    Ae3OriginalNearestPolygonResultV1 result{123};
    // Retail 0x21D898 checks original grid hit ORDER and the source
    // 0x21B788 plane predicate. The first eligible equal-distance
    // candidate (source polygon 2) wins over later source polygon 0.
    assert(ae3_original_polygon_nearest_hits_21d898(
               session,&request,&result)==1);
    assert(result.original_polygon_index==2);

    // Retail 0x21DA2C walks the entire original registry, has no
    // source plane predicate, and keeps first equal-distance result.
    result.original_polygon_index=123;
    assert(ae3_original_polygon_nearest_all_21da2c(
               session,&request,&result)==1);
    assert(result.original_polygon_index==0);

    // The complete original 0x4E3C14 grid lookup is recovered and
    // must drive 0x21D898 with original source cell offsets/pool.
    // No caller-generated hit list is allowed to influence this path.
    const std::array<std::uint16_t,2> sourceCellOffsets{0,4};
    const std::array<std::uint16_t,8> sourceHitPool{
        2,1,2,0, 1,0,0,0
    };
    Ae3OriginalGridNearestSourceV1 grid{};
    grid.original=request;
    grid.original.original_hit_indices=nullptr;
    grid.original.original_hit_count=0;
    grid.original_origin_x00=0;
    grid.original_origin_y04=0;
    grid.original_step_x10=1;
    grid.original_step_y14=1;
    grid.original_dimension_x18=2;
    grid.original_dimension_y1c=1;
    grid.original_cell_offsets50=sourceCellOffsets.data();
    grid.original_cell_offset_count=sourceCellOffsets.size();
    grid.original_hit_pool08=sourceHitPool.data();
    grid.original_hit_pool_count=sourceHitPool.size();
    grid.original_resolved_x=0;
    grid.original_resolved_y=0;
    result.original_polygon_index=123;
    assert(ae3_original_polygon_nearest_grid_4e3c14_21d898(
               session,&grid,&result)==1);
    assert(result.original_polygon_index==2);
    grid.original_resolved_x=1;
    assert(ae3_original_polygon_nearest_grid_4e3c14_21d898(
               session,&grid,&result)==1);
    assert(result.original_polygon_index==0);

    // The grid adapter uses genuine +0x50 cell offsets. A bad offset,
    // absent cell metadata, malformed cell count or nonfinite source
    // scalar must not publish a false nearest actor polygon.
    std::array<std::uint16_t,2> invalidOffsets=sourceCellOffsets;
    invalidOffsets[0]=65535;
    grid.original_cell_offsets50=invalidOffsets.data();
    grid.original_resolved_x=0;
    result.original_polygon_index=123;
    assert(ae3_original_polygon_nearest_grid_4e3c14_21d898(
               session,&grid,&result)==0);
    assert(result.original_polygon_index==123);
    grid.original_cell_offsets50=sourceCellOffsets.data();
    grid.original_cell_offset_count=1;
    assert(ae3_original_polygon_nearest_grid_4e3c14_21d898(
               session,&grid,&result)==0);
    assert(result.original_polygon_index==123);
    grid.original_cell_offset_count=sourceCellOffsets.size();
    grid.original_resolved_x=std::numeric_limits<float>::quiet_NaN();
    assert(ae3_original_polygon_nearest_grid_4e3c14_21d898(
               session,&grid,&result)==0);
    assert(result.original_polygon_index==123);

    // Original eligibility rejects the distant polygon, even when it
    // is the ONLY returned candidate (unlike the full-registry path).
    request.original_hit_count=1;
    result.original_polygon_index=123;
    assert(ae3_original_polygon_nearest_hits_21d898(
               session,&request,&result)==1);
    assert(result.original_polygon_index==UINT32_MAX);
    request.original_hit_count=0;
    result.original_polygon_index=123;
    assert(ae3_original_polygon_nearest_hits_21d898(
               session,&request,&result)==1);
    assert(result.original_polygon_index==UINT32_MAX);

    // A bad source grid index fails BEFORE any result publication.
    originalGridHits[0]=65535;
    request.original_hit_count=1;
    result.original_polygon_index=123;
    assert(ae3_original_polygon_nearest_hits_21d898(
               session,&request,&result)==0);
    assert(result.original_polygon_index==123);
    assert(std::strstr(ae3_original_manager_init_last_error(session),
                       "0x4E3C14")!=nullptr);
    originalGridHits[0]=1;

    // Invalid original stage descriptors and nonfinite XYZ fail closed,
    // do not leave a misleading successful target index behind.
    sourcePointCounts[1]=0;
    result.original_polygon_index=123;
    assert(ae3_original_polygon_nearest_all_21da2c(
               session,&request,&result)==0);
    assert(result.original_polygon_index==123);
    sourcePointCounts[1]=4;
    assert(ae3_original_polygon_nearest_all_21da2c(
               session,&request,&result)==0);
    assert(result.original_polygon_index==123);
    sourcePointCounts[1]=3;

    request.original_hit_indices=nullptr;
    request.original_hit_count=1;
    assert(ae3_original_polygon_nearest_hits_21d898(
               session,&request,&result)==0);
    assert(result.original_polygon_index==123);
    request.original_hit_indices=originalGridHits.data();

    request.original_hit_count=65536;
    assert(ae3_original_polygon_nearest_hits_21d898(
               session,&request,&result)==0);
    assert(result.original_polygon_index==123);
    request.original_hit_count=1;

    const auto previousQuery=request.original_query;
    request.original_query.x=std::numeric_limits<float>::quiet_NaN();
    assert(ae3_original_polygon_nearest_all_21da2c(
               session,&request,&result)==0);
    assert(result.original_polygon_index==123);
    request.original_query=previousQuery;

    // Source XYZ distance/plane checks never use a host-invented W axis.
    request.original_query.w=std::numeric_limits<float>::quiet_NaN();
    request.original_hit_count=originalGridHits.size();
    assert(ae3_original_polygon_nearest_hits_21d898(
               session,&request,&result)==1);
    assert(result.original_polygon_index==2);
    assert(ae3_original_manager_init_destroy(session)==1);
    std::cout<<"ORIGINAL_NEAREST_21D898_21DA2C_HOST_ABI_PASSED\n";
    return 0;
}
