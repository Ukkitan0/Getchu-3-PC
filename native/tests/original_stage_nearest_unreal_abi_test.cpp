#include "ae3/ae3_unreal_lua_bridge.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <vector>

namespace {
void put16(std::vector<std::uint8_t>& data,std::size_t off,std::uint16_t v) {
    assert(off+2<=data.size());
    data[off]=static_cast<std::uint8_t>(v);
    data[off+1]=static_cast<std::uint8_t>(v>>8);
}
void put32(std::vector<std::uint8_t>& data,std::size_t off,std::uint32_t v) {
    assert(off+4<=data.size());
    for (unsigned i=0;i<4;++i)
        data[off+i]=static_cast<std::uint8_t>(v>>(8*i));
}
void vertex(std::vector<std::uint8_t>& data,std::size_t off,
            float x,float y,float z) {
    put32(data,off,std::bit_cast<std::uint32_t>(x));
    put32(data,off+4,std::bit_cast<std::uint32_t>(y));
    put32(data,off+8,std::bit_cast<std::uint32_t>(z));
    put32(data,off+12,0xBADCAFE); // native source loader installs W=1
}
std::vector<std::uint8_t> originalLoadedStageFixture() {
    std::vector<std::uint8_t> source(0x220);
    put32(source,0x24,2); // actual 0x21D46C source count
    put32(source,0x38,0x60);put32(source,0x3c,0x100);
    put32(source,0x40,0x160);put32(source,0x44,0x1a0);
    put32(source,0x48,0x1c0);put32(source,0x4c,0x1e0);
    const std::size_t a=0x100,b=a+0x1e;
    put16(source,a,0xffff);put16(source,b,0xffff);
    put16(source,a+0x16,3);put16(source,b+0x16,1);
    put16(source,a+0x18,0);put16(source,a+0x1a,1);
    put16(source,a+0x1c,2);put16(source,b+0x18,3);
    vertex(source,0x160,1,0,0);
    vertex(source,0x170,4,1,0);
    vertex(source,0x180,8,0,1);
    vertex(source,0x190,5,0,0);
    return source;
}
}

int main() {
    auto originalBytes=originalLoadedStageFixture();
    Ae3OriginalStageNearestSourceV1 source{
        originalBytes.data(),originalBytes.size(),0x510000,0x6A1000,
        {5,0,0,0}};
    Ae3OriginalNearestPolygonResultV1 result{555};
    auto* session=ae3_original_manager_init_create(5489);
    assert(session);

    // Direct original 0x21D46C -> stage 0x70 records -> original
    // vertex table -> 0x21DA2C whole-registry VU0 nearest.
    assert(ae3_original_stage_nearest_all_21d46c_21da2c(
               session,&source,&result)==1);
    assert(result.original_polygon_index==1);
    assert(std::strlen(ae3_original_manager_init_last_error(session))==0);

    const std::array<std::uint16_t,1> cellOffsets{0};
    const std::array<std::uint16_t,3> hits{2,0,1};
    Ae3OriginalStageGridNearestSourceV1 grid{};
    grid.original=source;
    grid.original_origin_x00=0;
    grid.original_origin_y04=0;
    grid.original_step_x10=1;
    grid.original_step_y14=1;
    grid.original_dimension_x18=1;
    grid.original_dimension_y1c=1;
    grid.original_cell_offsets50=cellOffsets.data();
    grid.original_cell_offset_count=cellOffsets.size();
    grid.original_hit_pool08=hits.data();
    grid.original_hit_pool_count=hits.size();
    grid.original_resolved_x=0;
    grid.original_resolved_y=0;
    assert(ae3_original_stage_nearest_grid_21d46c_21d898(
               session,&grid,&result)==1);
    assert(result.original_polygon_index==1);

    // An original empty grid cell returns a legitimate no-hit result.
    const std::array<std::uint16_t,1> emptyCell{0};
    grid.original_hit_pool08=emptyCell.data();
    grid.original_hit_pool_count=emptyCell.size();
    assert(ae3_original_stage_nearest_grid_21d46c_21d898(
               session,&grid,&result)==1);
    assert(result.original_polygon_index==UINT32_MAX);
    grid.original_hit_pool08=hits.data();
    grid.original_hit_pool_count=hits.size();

    // Failure must not change a previously committed game-manager result.
    auto reject=[&] {
        result.original_polygon_index=555;
        assert(ae3_original_stage_nearest_all_21d46c_21da2c(
                   session,&source,&result)==0);
        assert(result.original_polygon_index==555);
        assert(std::strlen(ae3_original_manager_init_last_error(session))>0);
    };
    put16(originalBytes,0x100+0x16,4);
    reject(); // source descriptor count exceeds three 0x70 vertices
    put16(originalBytes,0x100+0x16,3);
    put16(originalBytes,0x100+2,4);
    reject(); // original 0x1e descriptor has only three action pairs
    put16(originalBytes,0x100+2,0);
    put32(originalBytes,0x40,0x21f);
    reject(); // original vertex table is incomplete
    put32(originalBytes,0x40,0x160);
    put32(originalBytes,0x24,0x7fffffff);
    reject(); // original stage spatial count cannot create huge allocations
    put32(originalBytes,0x24,2);
    source.original_runtime_base_va=0;
    reject(); // original 32-bit source pointer identity is required
    source.original_runtime_base_va=0x510000;

    // A bad original grid offset and query coordinate also fail closed.
    grid.original.original_loaded_stage_bytes=originalBytes.data();
    grid.original.original_loaded_stage_size=originalBytes.size();
    const std::array<std::uint16_t,1> badCell{65535};
    grid.original_cell_offsets50=badCell.data();
    result.original_polygon_index=555;
    assert(ae3_original_stage_nearest_grid_21d46c_21d898(
               session,&grid,&result)==0);
    assert(result.original_polygon_index==555);
    grid.original_cell_offsets50=cellOffsets.data();
    grid.original_resolved_x=std::numeric_limits<float>::quiet_NaN();
    assert(ae3_original_stage_nearest_grid_21d46c_21d898(
               session,&grid,&result)==0);
    assert(result.original_polygon_index==555);

    // Original 0x21B788 and XYZ distance do not consume vector W.
    source.original_query.w=std::numeric_limits<float>::quiet_NaN();
    assert(ae3_original_stage_nearest_all_21d46c_21da2c(
               session,&source,&result)==1);
    assert(result.original_polygon_index==1);
    // Install two genuine descriptor action requests. These are NOT
    // gameplay actors: the native stage preserves original type/slot/arg
    // until the source Player/Monkey manager can perform the real action.
    put16(originalBytes,0x100+2,2);
    put16(originalBytes,0x100+0x0A,1);
    put16(originalBytes,0x100+0x0C,2);
    put16(originalBytes,0x100+0x10,7);
    put16(originalBytes,0x100+0x12,8);
    // Stage owner keeps ORIGINAL decoded records across per-frame
    // queries without re-running 0x21D46C for every actor update.
    const Ae3OriginalStageSpatialMountSourceV1 mountSource{
        originalBytes.data(),originalBytes.size(),0x510000,0x6A1000};
    assert(ae3_original_stage_spatial_mount_21d46c(
               session,&mountSource)==1);
    size_t pendingCount=99;
    assert(ae3_original_stage_spatial_pending_action_count(
               session,&pendingCount)==1);
    assert(pendingCount==2);
    // Too-small read must publish neither source action nor count.
    std::array<Ae3OriginalStagePendingActionV1,2> pending{};
    pending[0].original_argument=0xABCD;
    pendingCount=99;
    assert(ae3_original_stage_spatial_pending_actions(
               session,pending.data(),1,&pendingCount)==0);
    assert(pendingCount==99 && pending[0].original_argument==0xABCD);
    assert(ae3_original_stage_spatial_pending_actions(
               session,pending.data(),pending.size(),&pendingCount)==1);
    assert(pendingCount==2 &&
           pending[0].original_record_index==0 &&
           pending[0].original_action_index==0 &&
           pending[0].original_action_type==1 &&
           pending[0].original_argument==7 &&
           pending[1].original_record_index==0 &&
           pending[1].original_action_index==1 &&
           pending[1].original_action_type==2 &&
           pending[1].original_argument==8 &&
           pending[0].reserved==0 && pending[1].reserved==0);

    Ae3OriginalVec4 frameQuery{5,0,0,0};
    assert(ae3_original_stage_spatial_nearest_all_21da2c(
               session,&frameQuery,&result)==1);
    assert(result.original_polygon_index==1);
    Ae3OriginalStageSpatialGridSourceV1 mountedGrid{};
    mountedGrid.original_origin_x00=0;
    mountedGrid.original_origin_y04=0;
    mountedGrid.original_step_x10=1;
    mountedGrid.original_step_y14=1;
    mountedGrid.original_dimension_x18=1;
    mountedGrid.original_dimension_y1c=1;
    mountedGrid.original_cell_offsets50=cellOffsets.data();
    mountedGrid.original_cell_offset_count=cellOffsets.size();
    mountedGrid.original_hit_pool08=hits.data();
    mountedGrid.original_hit_pool_count=hits.size();
    mountedGrid.original_resolved_x=0;
    mountedGrid.original_resolved_y=0;
    assert(ae3_original_stage_spatial_nearest_grid_21d898(
               session,&mountedGrid,&frameQuery,&result)==1);
    assert(result.original_polygon_index==1);

    // Ownership: original loaded source bytes can be modified or freed
    // after mounting; native records must NOT reference borrowed bytes.
    put16(originalBytes,0x100+0x16,4);
    put32(originalBytes,0x24,0x7fffffff);
    assert(ae3_original_stage_spatial_mount_21d46c(
               session,&mountSource)==0);
    result.original_polygon_index=555;
    assert(ae3_original_stage_spatial_nearest_all_21da2c(
               session,&frameQuery,&result)==1);
    assert(result.original_polygon_index==1);
    // Restore the test fixture and prove another good source mount
    // replaces the old stage only after successful validation.
    put32(originalBytes,0x24,2);
    put16(originalBytes,0x100+0x16,3);
    assert(ae3_original_stage_spatial_mount_21d46c(
               session,&mountSource)==1);
    result.original_polygon_index=555;
    assert(ae3_original_stage_spatial_nearest_grid_21d898(
               session,&mountedGrid,&frameQuery,&result)==1);
    assert(result.original_polygon_index==1);

    // Explicit unmount invalidates queries, but must not consume or
    // corrupt original RNG session ownership; same session may remount.
    assert(ae3_original_stage_spatial_unmount(session)==1);
    pendingCount=99;
    assert(ae3_original_stage_spatial_pending_action_count(
               session,&pendingCount)==0);
    assert(pendingCount==99);
    result.original_polygon_index=555;
    assert(ae3_original_stage_spatial_nearest_all_21da2c(
               session,&frameQuery,&result)==0);
    assert(result.original_polygon_index==555);
    assert(ae3_original_stage_spatial_mount_21d46c(
               session,&mountSource)==1);
    assert(ae3_original_stage_spatial_nearest_all_21da2c(
               session,&frameQuery,&result)==1);
    assert(result.original_polygon_index==1);
    assert(ae3_original_stage_spatial_unmount(session)==1);

    assert(ae3_original_manager_init_destroy(session)==1);
    std::cout<<"ORIGINAL_LOADED_STAGE_21D46C_NEAREST_HOST_ABI_PASSED\n";
    return 0;
}
