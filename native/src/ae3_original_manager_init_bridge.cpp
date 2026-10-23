#include "ae3/ae3_unreal_lua_bridge.h"
#include "ae3/original_manager_initializer.hpp"
#include "ae3/original_actor_route_follower.hpp"
#include "ae3/original_polygon_raycast.hpp"
#include "ae3/original_actor_target_dispatch.hpp"
#include "ae3/original_actor_random_triangle.hpp"
#include "ae3/original_actor_fallback_sampler.hpp"
#include "ae3/original_actor_fallback_vu0_native.hpp"
#include "ae3/original_vu0_geometry.hpp"
#include "ae3/original_spatial_support.hpp"
#include "ae3/original_spatial_raw_bridge.hpp"
#include "ae3/original_stage_spatial_loader.hpp"
#include <cmath>

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
static_assert(sizeof(Ae3OriginalVec4) == sizeof(ae3::OriginalVU0Vec4));
static_assert(sizeof(ae3::OriginalSpatialRecord70) == 0x70);

ae3::OriginalVU0Vec4 native(const Ae3OriginalVec4& src) noexcept {
    return {src.x,src.y,src.z,src.w};
}
Ae3OriginalVec4 external(const ae3::OriginalVU0Vec4& src) noexcept {
    return {src.x,src.y,src.z,src.w};
}
class RealActorCalls final : public ae3::OriginalManagerInitCalls_57bcb8 {
public:
    explicit RealActorCalls(const Ae3OriginalManagerInitCallsV1* original)
        : callbacks_(original) {}
    ae3::OriginalVU0Vec4 call_original_21b854(
        const ae3::OriginalSpatialRecord70& polygon,
        const ae3::OriginalVU0Vec4& start,
        const ae3::OriginalVU0Vec4& end) override {
        if (!callbacks_ || !callbacks_->project_original_21b854)
            throw std::runtime_error(
                "unrecovered original 0x21B854 actor projection manager missing");
        const auto originalStart = external(start), originalEnd = external(end);
        Ae3OriginalVec4 projected{};
        if (callbacks_->project_original_21b854(
                callbacks_->user,polygon.originalFields.data(),
                &originalStart,&originalEnd,&projected) != 1)
            throw std::runtime_error(
                "original 0x21B854 actor projection manager rejected source polygon");
        return native(projected);
    }
    void call_original_57c7cc(
        ae3::OriginalManagerInitState_57bcb8&,
        const ae3::OriginalVU0Vec4& originalIncomingA1) override {
        if (!callbacks_ || !callbacks_->continue_original_57c7cc)
            throw std::runtime_error(
                "unrecovered original 0x57C7CC actor route-update manager missing");
        const auto midpoint = external(originalIncomingA1);
        if (callbacks_->continue_original_57c7cc(
                callbacks_->user,&midpoint) != 1)
            throw std::runtime_error(
                "original 0x57C7CC actor route-update manager rejected source midpoint");
    }
private:
    const Ae3OriginalManagerInitCallsV1* callbacks_;
};

/* Unlike the legacy init hook, this one executes the actually recovered
 * 0x57C7CC native controller traversal. Only original polygon projection
 * and target-update *effects* remain external owner dependencies.
 */
class RealActorCallsV2 final : public ae3::OriginalManagerInitCalls_57bcb8 {
public:
    RealActorCallsV2(const Ae3OriginalManagerInitCallsV2* source,
                     std::span<const ae3::OriginalSpatialRecord70> records,
                     std::span<const uint32_t> route,
                     const ae3::OriginalSpatialRecord70* actor)
        :source_(source),records_(records),route_(route),actor_(actor){}
    uint32_t originalIndex(const ae3::OriginalSpatialRecord70* polygon) const {
        if (!polygon) return UINT32_MAX;
        for (std::size_t i=0;i<records_.size();++i)
            if (polygon==&records_[i]) return static_cast<uint32_t>(i);
        throw std::invalid_argument("original controller polygon outside decoded source registry");
    }
    ae3::OriginalVU0Vec4 call_original_21b854(
        const ae3::OriginalSpatialRecord70& polygon,
        const ae3::OriginalVU0Vec4& begin,
        const ae3::OriginalVU0Vec4& end) override {
        if (!source_ || !source_->project_original_21b854)
            throw std::runtime_error("original 0x21B854 projection manager missing");
        const auto start=external(begin), stop=external(end);
        Ae3OriginalVec4 projected{};
        if (source_->project_original_21b854(
                source_->user,polygon.originalFields.data(),&start,&stop,&projected)!=1)
            throw std::runtime_error("original 0x21B854 projection rejected by manager");
        return native(projected);
    }
    void call_original_57c7cc(
        ae3::OriginalManagerInitState_57bcb8& init,
        const ae3::OriginalVU0Vec4&) override {
        if (!source_ || !source_->original_apply_target_57c8ec)
            throw std::runtime_error("original 0x57C8EC target owner missing");
        struct ForwardOriginalCalls final : ae3::OriginalActorRouteFollowerCalls_57c7cc {
            RealActorCallsV2& outer;
            explicit ForwardOriginalCalls(RealActorCallsV2& source):outer(source){}
            const ae3::OriginalSpatialRecord70* call_original_4c1b3c_project(
                const ae3::OriginalSpatialRecord70* actor) override {
                if (!outer.source_->original_project_4c1b3c)
                    throw std::runtime_error("original 0x4C1B3C projection manager missing");
                uint32_t polygon=UINT32_MAX;
                if (outer.source_->original_project_4c1b3c(
                        outer.source_->user,outer.originalIndex(actor),&polygon)!=1)
                    throw std::runtime_error("original 0x4C1B3C projection rejected");
                if (polygon==UINT32_MAX) return nullptr;
                if (polygon>=outer.records_.size())
                    throw std::out_of_range("original projected polygon outside decoded registry");
                return &outer.records_[polygon];
            }
            void call_original_57c8ec_update_target(
                const ae3::OriginalSpatialRecord70* target) override {
                if (outer.source_->original_apply_target_57c8ec(
                        outer.source_->user,outer.originalIndex(target))!=1)
                    throw std::runtime_error("original 0x57C8EC target update rejected");
            }
        } calls(*this);
        ae3::OriginalActorRouteFollowerState_57c7cc route{
            records_,route_,actor_,init.target50,init.lifecycle.flag1c!=0
        };
        (void)ae3::originalActorAdvanceRoute_57c7cc(route,calls);
        init.target50=route.activePolygonAtController50;
    }
private:
    const Ae3OriginalManagerInitCallsV2* source_;
    std::span<const ae3::OriginalSpatialRecord70> records_;
    std::span<const uint32_t> route_;
    const ae3::OriginalSpatialRecord70* actor_;
};
}
struct Ae3OriginalManagerInitSession {
    ae3::OriginalGameRandom_36f3c0 originalRandom;
    std::string error;
    bool busy = false;
    explicit Ae3OriginalManagerInitSession(uint32_t seed):originalRandom(seed){}
};

extern "C" Ae3OriginalManagerInitSession*
ae3_original_manager_init_create(uint32_t original_game_seed) {
    try {return new Ae3OriginalManagerInitSession(original_game_seed);}
    catch (...) {return nullptr;}
}
extern "C" int ae3_original_manager_init_destroy(
    Ae3OriginalManagerInitSession* session) {
    if (!session) return 1;
    if (session->busy) {
        session->error = "original actor initializer is still active";
        return 0;
    }
    delete session;
    return 1;
}
extern "C" const char* ae3_original_manager_init_last_error(
    const Ae3OriginalManagerInitSession* session) {
    return session ? session->error.c_str()
                   : "original actor initializer session unavailable";
}

/* Expose the actually recovered 0x21D898/0x21DA2C original selectors,
 * not an Unreal nearest-point substitute. 0x4E3C14 candidate generation
 * is separate and its genuine u16 output must be supplied by the owner.
 */
static int originalNearestSourceImpl(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalNearestPolygonSourceV1* source,
    Ae3OriginalNearestPolygonResultV1* output,
    bool originalAllRecords,
    const Ae3OriginalGridNearestSourceV1* realGrid=nullptr) {
    if (!session) return 0;
    if (session->busy) {
        session->error="reentrant original actor nearest polygon query refused";
        return 0;
    }
    session->busy=true;
    session->error.clear();
    int success=0;
    try {
        if (!source || !output || !source->original_records70 ||
            !source->original_point_counts ||
            source->original_record_count==0 ||
            source->original_record_count>8192 ||
            (!originalAllRecords && !realGrid &&
             (source->original_hit_count>65535 ||
              (source->original_hit_count &&
               !source->original_hit_indices))))
            throw std::invalid_argument(
                "original spatial selector requires stage records, "
                "point descriptors and source grid hits");
        const auto& query=source->original_query;
        // W is not used by the original XZ plane test or XYZ distance,
        // and must not be reinterpreted as a new Unreal position axis.
        if (!std::isfinite(query.x) || !std::isfinite(query.y) ||
            !std::isfinite(query.z))
            throw std::invalid_argument(
                "original nearest polygon XYZ query is nonfinite");

        // Rehydrate aligned native geometry from genuine original records.
        // The original 0x21B458 reads the three source LQC2 vectors
        // in +0x40..+0x6F. Vertex counts come from 0x1E descriptors;
        // never infer counts from collision meshes or triangle fans.
        std::vector<ae3::OriginalSpatialRecord70> records(
            source->original_record_count);
        std::vector<std::array<ae3::OriginalVU0Vec4,3>> vertices(
            source->original_record_count);
        std::vector<ae3::OriginalSpatialSourcePolygon> polygons(
            source->original_record_count);
        const auto* bytes=
            static_cast<const std::byte*>(source->original_records70);
        for (std::size_t i=0;i<records.size();++i) {
            std::memcpy(records[i].originalFields.data(),
                        bytes+i*0x70,0x70);
            const auto count=source->original_point_counts[i];
            if (count==0 || count>3)
                throw std::out_of_range(
                    "original 0x1E polygon point count outside 1..3");
            for (std::size_t j=0;j<count;++j)
                std::memcpy(&vertices[i][j],
                    records[i].originalFields.data()+0x40+j*16,16);
            polygons[i].sourcePoints=
                std::span<const ae3::OriginalVU0Vec4>(
                    vertices[i].data(),count);
        }
        const auto queryVec=native(query);
        const ae3::OriginalSpatialRecord70* chosen=nullptr;
        if (originalAllRecords) {
            chosen=ae3::originalSpatialNearestAllWithVU0_21da2c(
                records,polygons,queryVec);
        } else {
            // Exact original 0x4E3C14 source grid metadata can now run
            // here, before the recovered source nearest selection.
            // No host-generated hit candidates are needed in this mode.
            ae3::OriginalSpatialGridHits generatedHits{};
            if (realGrid) {
                if (!realGrid->original_cell_offsets50 ||
                    !realGrid->original_hit_pool08 ||
                    !std::isfinite(realGrid->original_resolved_x) ||
                    !std::isfinite(realGrid->original_resolved_y) ||
                    !std::isfinite(realGrid->original_origin_x00) ||
                    !std::isfinite(realGrid->original_origin_y04) ||
                    !std::isfinite(realGrid->original_step_x10) ||
                    !std::isfinite(realGrid->original_step_y14) ||
                    realGrid->original_step_x10==0.0f ||
                    realGrid->original_step_y14==0.0f ||
                    realGrid->original_dimension_x18<=0 ||
                    realGrid->original_dimension_y1c<=0 ||
                    realGrid->original_dimension_x18>65535 ||
                    realGrid->original_dimension_y1c>65535 ||
                    realGrid->original_hit_pool_count>1048576 ||
                    realGrid->original_cell_offset_count>1048576)
                    throw std::invalid_argument(
                        "invalid original 0x4E3C14 source grid owner metadata");
                const auto cells=
                    static_cast<std::size_t>(realGrid->original_dimension_x18)*
                    static_cast<std::size_t>(realGrid->original_dimension_y1c);
                if (cells>1048576 ||
                    realGrid->original_cell_offset_count<cells)
                    throw std::out_of_range(
                        "original 0x4E3C14 grid cells exceed source registry");
                ae3::OriginalSpatialGridFields grid{};
                grid.originX00=realGrid->original_origin_x00;
                grid.originY04=realGrid->original_origin_y04;
                grid.stepX10=realGrid->original_step_x10;
                grid.stepY14=realGrid->original_step_y14;
                grid.dimensionX18=realGrid->original_dimension_x18;
                grid.dimensionY1c=realGrid->original_dimension_y1c;
                grid.cellOffsets50=std::span<const std::uint16_t>(
                    realGrid->original_cell_offsets50,
                    realGrid->original_cell_offset_count);
                grid.hitBuffer08=std::span<const std::uint16_t>(
                    realGrid->original_hit_pool08,
                    realGrid->original_hit_pool_count);
                generatedHits=ae3::originalSpatialGridLookup_4e3c14(
                    grid,realGrid->original_resolved_x,
                    realGrid->original_resolved_y);
            }
            const std::span<const std::uint16_t> hitIndices=realGrid
                ? generatedHits.originalRecordIndices
                : std::span<const std::uint16_t>(
                      source->original_hit_indices,source->original_hit_count);
            // Validate before calling 0x21D898 so malformed external
            // hit arrays cannot partially drive the source iterator.
            for (const auto hit:hitIndices)
                if (hit>=records.size())
                    throw std::out_of_range(
                        "original 0x4E3C14 grid hit outside spatial registry");
            chosen=ae3::originalSpatialNearestGeometry_21d898(
                records,polygons,ae3::OriginalSpatialGridHits{hitIndices},
                queryVec);
        }
        Ae3OriginalNearestPolygonResultV1 result{};
        result.original_polygon_index=UINT32_MAX;
        if (chosen) {
            // The iterator returns references only into local aligned
            // records. Never expose a transient pointer through the ABI.
            for (std::size_t i=0;i<records.size();++i) {
                if (chosen==&records[i]) {
                    result.original_polygon_index=
                        static_cast<std::uint32_t>(i);
                    break;
                }
            }
            if (result.original_polygon_index==UINT32_MAX)
                throw std::runtime_error(
                    "original spatial selector returned foreign record");
        }
        *output=result;
        session->error.clear();
        success=1;
    } catch (const std::exception& ex) {
        session->error=ex.what();
    } catch (...) {
        session->error="unknown original native polygon selection failure";
    }
    session->busy=false;
    return success;
}

extern "C" int ae3_original_polygon_nearest_hits_21d898(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalNearestPolygonSourceV1* original,
    Ae3OriginalNearestPolygonResultV1* output) {
    return originalNearestSourceImpl(session,original,output,false);
}
extern "C" int ae3_original_polygon_nearest_all_21da2c(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalNearestPolygonSourceV1* original,
    Ae3OriginalNearestPolygonResultV1* output) {
    return originalNearestSourceImpl(session,original,output,true);
}
extern "C" int ae3_original_polygon_nearest_grid_4e3c14_21d898(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalGridNearestSourceV1* original,
    Ae3OriginalNearestPolygonResultV1* output) {
    return originalNearestSourceImpl(
        session,original?&original->original:nullptr,output,false,original);
}

/* Source-backed actor spatial query: record+0x30 -> original header+0x16
 * -> record +0x40 VU0 vertices -> source grid (when requested) ->
 * original 0x21B788 eligibility / 0x21B468 mean / 0x21D898 or 0x21DA2C.
 * Do not decode counts from disconnected host-side polygon metadata.
 */
static int originalRawNearestImpl(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalRawNearestSourceV1* source,
    const Ae3OriginalRawGridNearestSourceV1* originalGrid,
    Ae3OriginalNearestPolygonResultV1* output) {
    if (!session) return 0;
    if (session->busy) {
        session->error="reentrant original resolved-header spatial query refused";
        return 0;
    }
    session->busy=true;
    session->error.clear();
    int success=0;
    try {
        if (!source || !output || !source->original_records70 ||
            !source->original_allocations ||
            source->original_record_count==0 ||
            source->original_record_count>8192 ||
            source->original_allocation_count==0 ||
            source->original_allocation_count>8192)
            throw std::invalid_argument(
                "original resolved-header selector requires genuine stage records and allocations");
        const auto q=source->original_query;
        if (!std::isfinite(q.x) || !std::isfinite(q.y) ||
            !std::isfinite(q.z))
            throw std::invalid_argument(
                "original resolved-header query has nonfinite XYZ");

        // Preserve 0x70 original record identities without type punning or
        // reading host pointer fields from the guest's 32-bit addresses.
        std::vector<ae3::OriginalSpatialRecord70> records(
            source->original_record_count);
        const auto* raw=static_cast<const std::byte*>(source->original_records70);
        for (std::size_t i=0;i<records.size();++i)
            std::memcpy(records[i].originalFields.data(),
                        raw+i*0x70,0x70);

        std::vector<ae3::OriginalSpatialSourceAllocation> allocations;
        allocations.reserve(source->original_allocation_count);
        for (std::size_t i=0;i<source->original_allocation_count;++i) {
            const auto& allocation=source->original_allocations[i];
            if (!allocation.original_bytes ||
                allocation.original_byte_count<0x18 ||
                allocation.original_byte_count>16*1024*1024)
                throw std::invalid_argument(
                    "invalid original header allocation bytes or span");
            allocations.push_back({
                allocation.original_base_va,
                std::span<const std::byte>(
                    static_cast<const std::byte*>(allocation.original_bytes),
                    allocation.original_byte_count)});
        }

        const ae3::OriginalSpatialRecord70* chosen=nullptr;
        if (originalGrid) {
            if (!originalGrid->original_cell_offsets50 ||
                !originalGrid->original_hit_pool08 ||
                !std::isfinite(originalGrid->original_resolved_x) ||
                !std::isfinite(originalGrid->original_resolved_y) ||
                !std::isfinite(originalGrid->original_origin_x00) ||
                !std::isfinite(originalGrid->original_origin_y04) ||
                !std::isfinite(originalGrid->original_step_x10) ||
                !std::isfinite(originalGrid->original_step_y14) ||
                originalGrid->original_step_x10==0.0f ||
                originalGrid->original_step_y14==0.0f ||
                originalGrid->original_dimension_x18<=0 ||
                originalGrid->original_dimension_y1c<=0 ||
                originalGrid->original_dimension_x18>65535 ||
                originalGrid->original_dimension_y1c>65535 ||
                originalGrid->original_hit_pool_count>1048576 ||
                originalGrid->original_cell_offset_count>1048576)
                throw std::invalid_argument(
                    "invalid original 0x4E3C14 resolved-header grid metadata");
            const auto requiredCells=
                static_cast<std::size_t>(originalGrid->original_dimension_x18)*
                static_cast<std::size_t>(originalGrid->original_dimension_y1c);
            if (requiredCells>1048576 ||
                originalGrid->original_cell_offset_count<requiredCells)
                throw std::out_of_range(
                    "original resolved-header grid cell map truncated");
            ae3::OriginalSpatialGridFields grid{};
            grid.originX00=originalGrid->original_origin_x00;
            grid.originY04=originalGrid->original_origin_y04;
            grid.stepX10=originalGrid->original_step_x10;
            grid.stepY14=originalGrid->original_step_y14;
            grid.dimensionX18=originalGrid->original_dimension_x18;
            grid.dimensionY1c=originalGrid->original_dimension_y1c;
            grid.cellOffsets50=std::span<const std::uint16_t>(
                originalGrid->original_cell_offsets50,
                originalGrid->original_cell_offset_count);
            grid.hitBuffer08=std::span<const std::uint16_t>(
                originalGrid->original_hit_pool08,
                originalGrid->original_hit_pool_count);
            // The existing raw bridge resolves EVERY record+0x30 pointer
            // to an original allocated header, reading LHU +0x16.
            // Candidate generation then executes genuine 0x4E3C14.
            chosen=ae3::originalSpatialNearestOriginalDataGrid_21d898(
                records,allocations,grid,
                originalGrid->original_resolved_x,
                originalGrid->original_resolved_y,native(q));
        } else {
            // Original 0x21DA2C scans the full registry without a
            // spatial hit list or the 0x21B788 plane filter.
            chosen=ae3::originalSpatialNearestOriginalDataAll_21da2c(
                records,allocations,native(q));
        }
        Ae3OriginalNearestPolygonResultV1 value{};
        value.original_polygon_index=UINT32_MAX;
        if (chosen) {
            for (std::size_t i=0;i<records.size();++i) {
                if (chosen==&records[i]) {
                    value.original_polygon_index=
                        static_cast<std::uint32_t>(i);
                    break;
                }
            }
            if (value.original_polygon_index==UINT32_MAX)
                throw std::runtime_error(
                    "original resolved-header selector returned foreign record");
        }
        *output=value;
        session->error.clear();
        success=1;
    } catch (const std::exception& error) {
        session->error=error.what();
    } catch (...) {
        session->error="unknown original header-resolved actor nearest failure";
    }
    session->busy=false;
    return success;
}
extern "C" int ae3_original_polygon_nearest_original_data_all_21da2c(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalRawNearestSourceV1* original,
    Ae3OriginalNearestPolygonResultV1* output) {
    return originalRawNearestImpl(session,original,nullptr,output);
}
extern "C" int ae3_original_polygon_nearest_original_data_grid_21d898(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalRawGridNearestSourceV1* original,
    Ae3OriginalNearestPolygonResultV1* output) {
    return originalRawNearestImpl(
        session,original?&original->original:nullptr,original,output);
}

/* The complete recovered source stage spatial lookup in ONE native call:
 * 0x21D46C loaded source -> original header pointer+0x30 and vertex table
 * -> 0x4E3C14 grid hits (if requested) -> source VU0 0x21D898/0x21DA2C.
 * Actor/gameplay creation is intentionally NOT manufactured here.
 */
static int originalStageNearestImpl(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalStageNearestSourceV1* source,
    const Ae3OriginalStageGridNearestSourceV1* originalGrid,
    Ae3OriginalNearestPolygonResultV1* output) {
    if (!session) return 0;
    if (session->busy) {
        session->error="reentrant original 0x21D46C stage spatial query refused";
        return 0;
    }
    session->busy=true;
    session->error.clear();
    int success=0;
    try {
        if (!source || !output ||
            !source->original_loaded_stage_bytes ||
            source->original_loaded_stage_size<0x50 ||
            source->original_loaded_stage_size>64*1024*1024 ||
            source->original_runtime_base_va==0)
            throw std::invalid_argument(
                "original 0x21D46C requires genuine bounded loaded stage bytes and source VA");
        const auto q=source->original_query;
        if (!std::isfinite(q.x) || !std::isfinite(q.y) ||
            !std::isfinite(q.z))
            throw std::invalid_argument(
                "original stage nearest source XYZ query is nonfinite");
        const ae3::OriginalStageSpatialSource genuineStage{
            source->original_runtime_base_va,
            std::span<const std::byte>(
                static_cast<const std::byte*>(
                    source->original_loaded_stage_bytes),
                source->original_loaded_stage_size),
            source->original_registry_owner_va};
        const auto bounds=ae3::originalStageReadSpatialOffsets_21d46c(
            genuineStage);
        if (bounds.countAt24==0 || bounds.countAt24>8192)
            throw std::out_of_range(
                "original stage spatial registry count outside supported bounds");
        const auto decoded=ae3::originalStageBuildSourceSpatial_21d46c(
            genuineStage);
        const ae3::OriginalSpatialRecord70* selected=nullptr;
        if (originalGrid) {
            if (!originalGrid->original_cell_offsets50 ||
                !originalGrid->original_hit_pool08 ||
                !std::isfinite(originalGrid->original_origin_x00) ||
                !std::isfinite(originalGrid->original_origin_y04) ||
                !std::isfinite(originalGrid->original_step_x10) ||
                !std::isfinite(originalGrid->original_step_y14) ||
                !std::isfinite(originalGrid->original_resolved_x) ||
                !std::isfinite(originalGrid->original_resolved_y) ||
                originalGrid->original_step_x10==0.0f ||
                originalGrid->original_step_y14==0.0f ||
                originalGrid->original_dimension_x18<=0 ||
                originalGrid->original_dimension_y1c<=0 ||
                originalGrid->original_dimension_x18>65535 ||
                originalGrid->original_dimension_y1c>65535 ||
                originalGrid->original_hit_pool_count>1048576 ||
                originalGrid->original_cell_offset_count>1048576)
                throw std::invalid_argument(
                    "original stage 0x4E3C14 grid metadata invalid");
            const auto cells=
                static_cast<std::size_t>(originalGrid->original_dimension_x18)*
                static_cast<std::size_t>(originalGrid->original_dimension_y1c);
            if (cells>1048576 ||
                originalGrid->original_cell_offset_count<cells)
                throw std::out_of_range(
                    "original stage grid source offset map truncated");
            ae3::OriginalSpatialGridFields grid{};
            grid.originX00=originalGrid->original_origin_x00;
            grid.originY04=originalGrid->original_origin_y04;
            grid.stepX10=originalGrid->original_step_x10;
            grid.stepY14=originalGrid->original_step_y14;
            grid.dimensionX18=originalGrid->original_dimension_x18;
            grid.dimensionY1c=originalGrid->original_dimension_y1c;
            grid.cellOffsets50=std::span<const std::uint16_t>(
                originalGrid->original_cell_offsets50,
                originalGrid->original_cell_offset_count);
            grid.hitBuffer08=std::span<const std::uint16_t>(
                originalGrid->original_hit_pool08,
                originalGrid->original_hit_pool_count);
            selected=ae3::originalStageNearestGrid_21d898(
                decoded,grid,originalGrid->original_resolved_x,
                originalGrid->original_resolved_y,native(q));
        } else {
            selected=ae3::originalStageNearestAll_21da2c(
                decoded,native(q));
        }
        Ae3OriginalNearestPolygonResultV1 value{};
        value.original_polygon_index=UINT32_MAX;
        if (selected) {
            for (std::size_t i=0;i<decoded.records.size();++i) {
                if (selected==&decoded.records[i]) {
                    value.original_polygon_index=
                        static_cast<std::uint32_t>(i);
                    break;
                }
            }
            if (value.original_polygon_index==UINT32_MAX)
                throw std::runtime_error(
                    "original 0x21D46C spatial selection returned foreign stage record");
        }
        *output=value;
        session->error.clear();
        success=1;
    } catch (const std::exception& error) {
        session->error=error.what();
    } catch (...) {
        session->error="unknown original 0x21D46C source spatial query error";
    }
    session->busy=false;
    return success;
}
extern "C" int ae3_original_stage_nearest_all_21d46c_21da2c(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalStageNearestSourceV1* source,
    Ae3OriginalNearestPolygonResultV1* output) {
    return originalStageNearestImpl(session,source,nullptr,output);
}
extern "C" int ae3_original_stage_nearest_grid_21d46c_21d898(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalStageGridNearestSourceV1* source,
    Ae3OriginalNearestPolygonResultV1* output) {
    return originalStageNearestImpl(
        session,source?&source->original:nullptr,source,output);
}

static int originalManagerInitImpl(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalManagerInitSourceV1* source,
    Ae3OriginalManagerInitStateV1* state,
    const Ae3OriginalManagerInitCallsV1* oldCallbacks,
    const Ae3OriginalManagerInitCallsV2* sourceCallbacks) {
    if (!session) return 0;
    if (session->busy) {
        session->error = "reentrant original actor initializer call forbidden";
        return 0;
    }
    session->error.clear();
    session->busy = true;
    int result = 0;
    try {
        if (!source || !state || source->original_record_count > 8192 ||
            source->original_route_count > 8192 ||
            source->original_navigation_size > 16*1024*1024 ||
            (source->original_record_count && !source->original_records70) ||
            (source->original_route_count && !source->original_route_indices) ||
            (source->original_navigation_size && !source->original_navigation_bytes))
            throw std::invalid_argument("missing or oversized original actor spatial source");
        // The owner decoded original stage resource supplies packed
        // 0x70-byte polygon records; no synthetic spatial reconstruction.
        // Copy to aligned, invocation-local host records; never return
        // pointers into temporary actor/stage buffers.
        std::vector<ae3::OriginalSpatialRecord70> records(
            source->original_record_count);
        for (std::size_t i = 0; i != records.size(); ++i)
            std::memcpy(records[i].originalFields.data(),
                static_cast<const std::byte*>(source->original_records70) +
                i*0x70,0x70);
        // Freeze every externally owned stage input before calling
        // 0x21B854 or entering the genuine 0x57C7CC route. Those source
        // manager callbacks may pump Unreal events and mutate original
        // route/nav buffers, but must not change this invocation halfway.
        std::vector<std::uint32_t> routeSnapshot;
        if (source->original_route_count)
            routeSnapshot.assign(source->original_route_indices,
                                 source->original_route_indices+
                                     source->original_route_count);
        std::vector<std::byte> navigationSnapshot;
        if (source->original_navigation_size) {
            const auto* navigationBytes=
                static_cast<const std::byte*>(source->original_navigation_bytes);
            navigationSnapshot.assign(navigationBytes,
                navigationBytes+source->original_navigation_size);
        }
        auto* originalPolygon = static_cast<const ae3::OriginalSpatialRecord70*>(nullptr);
        if (source->original_actor_polygon_index != UINT32_MAX) {
            if (source->original_actor_polygon_index >= records.size())
                throw std::out_of_range("original actor polygon index outside source stage");
            originalPolygon = &records[source->original_actor_polygon_index];
        }
        // Callbacks may re-enter the owner's Unreal code, so the input
        // state must be one snapshot. Do not mix pre- and post-callback
        // fields, and publish a complete result only on source success.
        const Ae3OriginalManagerInitStateV1 initial = *state;
        ae3::OriginalManagerChild70 child{};
        child.word08 = initial.source_method08;
        child.word0c = initial.source_method0c;
        child.flag1c = initial.source_flag1c;
        child.alternative30 = initial.source_alternative30;
        // When the genuine variant-one 0x57BAF8 constructor preceded
        // 0x57BCB8, source SQ wrote the COMPLETE controller +0x20 value.
        // Preserve this exact owner-provided vector in the typed child.
        // Variant-zero 0x57BA98 must not claim any such source write.
        if (child.alternative30) {
            std::array<std::byte,16> originalController20{};
            static_assert(sizeof(initial.controller20)==16);
            std::memcpy(originalController20.data(),&initial.controller20,16);
            child.controller20=originalController20;
        }
        ae3::OriginalManagerInitState_57bcb8 sourceState{
            child, native(initial.controller20), native(initial.target40),
            nullptr, native(initial.midpoint60)
        };
        if (initial.target_polygon_index != UINT32_MAX) {
            if (initial.target_polygon_index >= records.size())
                throw std::out_of_range("original target polygon outside source stage");
            sourceState.target50 = &records[initial.target_polygon_index];
        }
        const auto stage = ae3::OriginalManagerInitSource_57bcb8{
            native(source->original_owner_position40),
            native(source->original_owner_direction50),
            originalPolygon,records,
            std::span<const std::uint32_t>(routeSnapshot),
            std::span<const std::byte>(navigationSnapshot)
        };
        // Freeze the complete registered callback table as well as actor
        // resources. A host rebind during 0x21B854 must not redirect the
        // remaining source call sequence within this initialization.
        Ae3OriginalManagerInitCallsV1 stableOldCallbacks{};
        Ae3OriginalManagerInitCallsV2 stableSourceCallbacks{};
        if (oldCallbacks) stableOldCallbacks=*oldCallbacks;
        if (sourceCallbacks) stableSourceCallbacks=*sourceCallbacks;
        RealActorCalls oldCalls(oldCallbacks?&stableOldCallbacks:nullptr);
        RealActorCallsV2 recoveredRouteCalls(
            sourceCallbacks?&stableSourceCallbacks:nullptr,records,
            std::span<const std::uint32_t>(routeSnapshot),originalPolygon);
        ae3::OriginalManagerInitCalls_57bcb8& calls = sourceCallbacks
            ? static_cast<ae3::OriginalManagerInitCalls_57bcb8&>(recoveredRouteCalls)
            : static_cast<ae3::OriginalManagerInitCalls_57bcb8&>(oldCalls);
        // Preserve original global RNG behavior on successful calls, but
        // do not consume any original random numbers if owner input or a
        // recovered branch throws before the controller state commits.
        // The native stage transaction owns this atomicity boundary.
        auto tentativeRng = session->originalRandom;
        const auto branch = ae3::originalManagerInitialize_57bcb8(
            sourceState,stage,tentativeRng,calls);
        // No manager state is published until original computation succeeded.
        Ae3OriginalManagerInitStateV1 completed = initial;
        completed.source_method08 = child.word08;
        completed.source_method0c = child.word0c;
        completed.target40 = external(sourceState.target40);
        completed.midpoint60 = external(sourceState.midpoint60);
        completed.target_polygon_index = UINT32_MAX;
        if (sourceState.target50) {
            const auto* first = records.data();
            if (sourceState.target50 < first ||
                sourceState.target50 >= first+records.size())
                throw std::runtime_error(
                    "original actor target pointer outside source spatial registry");
            completed.target_polygon_index =
                static_cast<uint32_t>(sourceState.target50-first);
        }
        session->originalRandom = std::move(tentativeRng);
        *state = completed;
        // A callback can correctly try (and fail) to destroy its live
        // session. That rejection must not leave a stale last_error on
        // a successfully completed original branch.
        session->error.clear();
        result = branch == ae3::OriginalManagerInitBranch_57bcb8::nearOwnerReturn ? 1
               : branch == ae3::OriginalManagerInitBranch_57bcb8::typeTwoTarget ? 2
               : 3;
    } catch (const std::exception& ex) {
        session->error = ex.what();
    } catch (...) {
        session->error = "unknown original actor initializer error";
    }
    session->busy = false;
    return result;
}
extern "C" int ae3_original_manager_init_step(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalManagerInitSourceV1* source,
    Ae3OriginalManagerInitStateV1* state,
    const Ae3OriginalManagerInitCallsV1* callbacks) {
    return originalManagerInitImpl(session,source,state,callbacks,nullptr);
}
extern "C" int ae3_original_manager_init_step_v2(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalManagerInitSourceV1* source,
    Ae3OriginalManagerInitStateV1* state,
    const Ae3OriginalManagerInitCallsV2* callbacks) {
    if (!callbacks) {
        if(session) session->error="original initializer v2 requires recovered route manager";
        return 0;
    }
    return originalManagerInitImpl(session,source,state,nullptr,callbacks);
}

/* Original 0x57C7CC route follower is already source-recovered. Unlike
 * opaque callback delegation, this function executes that implementation.
 * Only truly unrecovered original 0x4C1B3C and 0x57C8EC game manager
 * operations remain externally owned and fail if unavailable.
 */
extern "C" int ae3_original_actor_route_step(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalRouteSourceV1* source,
    const Ae3OriginalRouteCallsV1* owner,
    Ae3OriginalRouteResultV1* output) {
    if (!session) return 0;
    if (session->busy) {
        session->error = "reentrant original 0x57C7CC route follow refused";
        return 0;
    }
    session->error.clear();
    session->busy = true;
    int succeeded = 0;
    try {
        if (!source || !output || !owner || !owner->original_apply_target_57c8ec ||
            source->original_record_count == 0 ||
            source->original_record_count > 8192 ||
            source->original_route_count > 8192 ||
            !source->original_records70 ||
            (source->original_route_count && !source->original_route_indices) ||
            source->actor_polygon_index >= source->original_record_count ||
            (source->active_target_polygon_index != UINT32_MAX &&
             source->active_target_polygon_index >= source->original_record_count))
            throw std::invalid_argument(
                "original 0x57C7CC requires actual source polygons and target owner");
        std::vector<ae3::OriginalSpatialRecord70> records(
            source->original_record_count);
        const auto* bytes = static_cast<const std::byte*>(source->original_records70);
        for (std::size_t i = 0; i < records.size(); ++i)
            std::memcpy(records[i].originalFields.data(),
                bytes + i*0x70,0x70);
        // Source records are local and aligned, and all callback source
        // identities remain valid throughout this synchronous call.
        struct OriginalOwnerAdapter final : ae3::OriginalActorRouteFollowerCalls_57c7cc {
            const Ae3OriginalRouteCallsV1* owner;
            std::span<const ae3::OriginalSpatialRecord70> records;
            explicit OriginalOwnerAdapter(
                const Ae3OriginalRouteCallsV1* calls,
                std::span<const ae3::OriginalSpatialRecord70> source)
                :owner(calls),records(source){}
            uint32_t index(const ae3::OriginalSpatialRecord70* polygon) const {
                if (!polygon) return UINT32_MAX;
                for (uint32_t i=0; i < records.size(); ++i)
                    if (&records[i] == polygon) return i;
                throw std::invalid_argument(
                    "original route returned polygon outside decoded stage");
            }
            const ae3::OriginalSpatialRecord70* call_original_4c1b3c_project(
                const ae3::OriginalSpatialRecord70* actor) override {
                if (!owner->original_project_4c1b3c)
                    throw std::runtime_error(
                        "original 0x4C1B3C ray projection manager unavailable");
                uint32_t projected=UINT32_MAX;
                if (owner->original_project_4c1b3c(
                    owner->user,index(actor),&projected)!=1)
                    throw std::runtime_error(
                        "original 0x4C1B3C ray projection manager rejected source");
                if (projected == UINT32_MAX) return nullptr;
                if (projected >= records.size())
                    throw std::out_of_range(
                        "original 0x4C1B3C projected polygon outside source stage");
                return &records[projected];
            }
            void call_original_57c8ec_update_target(
                const ae3::OriginalSpatialRecord70* target) override {
                if (owner->original_apply_target_57c8ec(
                        owner->user,index(target))!=1)
                    throw std::runtime_error(
                        "original 0x57C8EC target manager rejected source route");
            }
        } adapter(owner,records);
        ae3::OriginalActorRouteFollowerState_57c7cc state{
            records,
            std::span<const uint32_t>(
                source->original_route_indices,source->original_route_count),
            &records[source->actor_polygon_index],
            source->active_target_polygon_index==UINT32_MAX
                ? nullptr:&records[source->active_target_polygon_index],
            source->original_has_multiple_polygons1c!=0
        };
        const auto followed=ae3::originalActorAdvanceRoute_57c7cc(state,adapter);
        // Publish nothing on failure. Owner callbacks must separately use
        // original stage transactions for their own side-effect rollback.
        const Ae3OriginalRouteResultV1 result{
            adapter.index(state.activePolygonAtController50),
            static_cast<uint32_t>(followed.sourceTargetsApplied),
            static_cast<uint32_t>(followed.sourceProjectionsPerformed),
            static_cast<uint32_t>(followed.followingRouteIndicesTested),
            followed.returnedEarlyBecauseCurrentTargetMatched ? 1 : 0
        };
        *output=result;
        session->error.clear();
        succeeded=1;
    } catch (const std::exception& error) {
        session->error=error.what();
    } catch (...) {
        session->error="unknown original 0x57C7CC route follower failure";
    }
    session->busy=false;
    return succeeded;
}

/* This runs the original complete source polygon raycast chain, not an
 * Unreal-engine ray or an imported collision-mesh substitute. The owner
 * supplies decoded 0x70 original records and source graph-edge metadata.
 */
extern "C" int ae3_original_polygon_project_4c1b3c(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalPolygonRaySourceV1* source,
    Ae3OriginalPolygonRayResultV1* out) {
    if (!session) return 0;
    if (session->busy) {
        session->error = "original polygon raycast cannot reenter live actor manager";
        return 0;
    }
    session->error.clear();
    session->busy = true;
    int success = 0;
    try {
        if (!source || !out ||
            source->original_record_count == 0 ||
            source->original_record_count > 8192 ||
            !source->original_records70 || !source->original_degree ||
            !source->original_neighbor3 || !source->original_action_kind3 ||
            source->original_start_polygon >= source->original_record_count ||
            source->original_max_recursions == 0 ||
            source->original_max_recursions > 4096)
            throw std::invalid_argument(
                "actual original polygon records, adjacency, and recursion budget required");
        const auto& a=source->original_ray_start;
        const auto& b=source->original_ray_end;
        if (!(std::isfinite(a.x) && std::isfinite(a.y) &&
              std::isfinite(a.z) && std::isfinite(a.w) &&
              std::isfinite(b.x) && std::isfinite(b.y) &&
              std::isfinite(b.z) && std::isfinite(b.w)))
            throw std::invalid_argument("original polygon projection operands are not finite");
        std::vector<ae3::OriginalSpatialRecord70> records(
            source->original_record_count);
        std::vector<ae3::OriginalPolygonLinks_21bd3c> adjacency(
            source->original_record_count);
        const auto* polygonBytes =
            static_cast<const std::byte*>(source->original_records70);
        for (std::size_t polygon = 0; polygon < records.size(); ++polygon) {
            std::memcpy(records[polygon].originalFields.data(),
                        polygonBytes + polygon*0x70,0x70);
            const auto degree=source->original_degree[polygon];
            if(degree>3)
                throw std::out_of_range(
                    "original polygon navigation degree exceeds 3");
            adjacency[polygon].edgeCount=degree;
            for (std::size_t edge=0; edge<degree; ++edge) {
                const auto i=3*polygon+edge;
                const auto neighbor=source->original_neighbor3[i];
                const auto kind=source->original_action_kind3[i];
                if (neighbor>=records.size() || kind>2)
                    throw std::out_of_range(
                        "original source graph neighbor/action kind invalid");
                adjacency[polygon].edges[edge]={neighbor,kind};
            }
        }
        const auto result=ae3::originalPolygonRaycast_4c1b3c(
            records,adjacency,source->original_start_polygon,
            native(a),native(b),source->original_max_recursions);
        Ae3OriginalPolygonRayResultV1 value{};
        value.last_original_polygon=UINT32_MAX;
        if (result.originalTraversal.originalLastAcceptedPolygon) {
            bool identified=false;
            for (std::uint32_t i=0;i<records.size();++i) {
                if (result.originalTraversal.originalLastAcceptedPolygon==
                    &records[i]) {
                    value.last_original_polygon=i;
                    identified=true;
                    break;
                }
            }
            if (!identified)
                throw std::runtime_error(
                    "original 0x4C1B3C traversal returned foreign polygon");
        }
        const auto& traversal=result.originalTraversal;
        value.original_polygons_tested=
            static_cast<std::uint32_t>(traversal.sourcePolygonsTested);
        value.original_edge_tests=
            static_cast<std::uint32_t>(traversal.sourceEdgeTests);
        value.original_zero_kind_recursions=
            static_cast<std::uint32_t>(traversal.sourceZeroKindRecursions);
        value.original_hit_point_available=traversal.sourceHitPoint?1:0;
        if (traversal.sourceHitPoint)
            value.original_hit_point=external(*traversal.sourceHitPoint);
        *out=value;
        session->error.clear();
        success=1;
    } catch (const std::exception& ex) {
        session->error=ex.what();
    } catch (...) {
        session->error="unknown original 0x4C1B3C polygon raycast failure";
    }
    session->busy=false;
    return success;
}

/* Real original 0x57C7CC -> 0x4C1B3C -> 0x21BD3C -> 0x21EE00.
 * It uses no arbitrary geometry callback. The only externally owned
 * routine is the original 0x57C8EC update/low64 result.
 */
extern "C" int ae3_original_actor_route_step_v2(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalRouteSourceV2* source,
    const Ae3OriginalRouteCallsV2* owner,
    Ae3OriginalRouteResultV1* out) {
    if (!session) return 0;
    if (session->busy) {
        session->error="cannot reenter source original 0x57C7CC route v2";
        return 0;
    }
    session->error.clear();
    session->busy=true;
    int completed=0;
    try {
        if (!source || !out || !owner || !owner->original_apply_target_57c8ec)
            throw std::invalid_argument(
                "original route target manager and source must exist");
        const auto& info=source->original;
        if (info.original_record_count==0 ||
            info.original_record_count>8192 ||
            info.original_route_count>8192 ||
            !info.original_records70 ||
            (info.original_route_count && !info.original_route_indices) ||
            info.actor_polygon_index>=info.original_record_count ||
            (info.active_target_polygon_index!=UINT32_MAX &&
             info.active_target_polygon_index>=info.original_record_count) ||
            !source->original_degree || !source->original_neighbor3 ||
            !source->original_action_kind3 ||
            source->original_max_recursions==0 ||
            source->original_max_recursions>4096)
            throw std::invalid_argument(
                "original 0x57C7CC stage registry, adjacency or target invalid");
        // This legacy v2 callback only returns QMFC2 LOW64. Original
        // 0x57C894 uses QMFC2 HIGH64 for follow-up route projections.
        // Never produce a plausible but wrong projection after route[1].
        // Use the full-width, transactional route v3 for three+ entries.
        if (info.original_has_multiple_polygons1c &&
            info.original_route_count > 2)
            throw std::runtime_error(
                "original route v2 cannot represent full 128-bit target; "
                "use original route v3 before route[2] projection");
        const auto& v1=source->original_incoming_a1;
        const auto& v2=source->original_controller40;
        if (!std::isfinite(v1.x)||!std::isfinite(v1.y)||
            !std::isfinite(v1.z)||!std::isfinite(v1.w)||
            !std::isfinite(v2.x)||!std::isfinite(v2.y)||
            !std::isfinite(v2.z)||!std::isfinite(v2.w))
            throw std::invalid_argument(
                "nonfinite original route incoming ray or controller+0x40");
        std::vector<ae3::OriginalSpatialRecord70> records(
            info.original_record_count);
        std::vector<ae3::OriginalPolygonLinks_21bd3c> links(
            info.original_record_count);
        const auto* raw=static_cast<const std::byte*>(info.original_records70);
        for (std::size_t polygon=0;polygon<records.size();++polygon) {
            std::memcpy(records[polygon].originalFields.data(),
                        raw+polygon*0x70,0x70);
            const auto count=source->original_degree[polygon];
            if (count>3)throw std::out_of_range(
                "original route polygon degree invalid");
            links[polygon].edgeCount=count;
            for (std::size_t j=0;j<count;++j) {
                const auto i=polygon*3+j;
                const auto next=source->original_neighbor3[i];
                const auto kind=source->original_action_kind3[i];
                if (next>=records.size() || kind>2)
                    throw std::out_of_range(
                        "original route adjacency references invalid polygon");
                links[polygon].edges[j]={next,kind};
            }
        }
        struct ActualSourceMath final:ae3::OriginalActorRouteFollowerCalls_57c7cc {
            std::span<const ae3::OriginalSpatialRecord70> records;
            std::span<const ae3::OriginalPolygonLinks_21bd3c> links;
            const Ae3OriginalRouteCallsV2& owner;
            ae3::OriginalVU0Vec4 originalRayA1{};
            ae3::OriginalVU0Vec4 controller40{};
            std::optional<ae3::OriginalVU0Vec4> targetRayA2{};
            uint32_t recursions;
            ActualSourceMath(
                std::span<const ae3::OriginalSpatialRecord70> r,
                std::span<const ae3::OriginalPolygonLinks_21bd3c> links,
                const Ae3OriginalRouteCallsV2& owner,
                ae3::OriginalVU0Vec4 a1,
                ae3::OriginalVU0Vec4 controller,
                uint32_t recursionLimit)
                :records(r),links(links),owner(owner),
                 originalRayA1(a1),controller40(controller),
                 recursions(recursionLimit){}
            uint32_t at(const ae3::OriginalSpatialRecord70* polygon) const {
                for (std::size_t i=0;i<records.size();++i)
                    if (polygon==&records[i])return static_cast<uint32_t>(i);
                throw std::invalid_argument("original route polygon owner absent");
            }
            const ae3::OriginalSpatialRecord70* call_original_4c1b3c_project(
                const ae3::OriginalSpatialRecord70* actor) override {
                // A subsequent projection is not representable using
                // v2's low64-only target ABI. The source uses the high64.
                if (targetRayA2)
                    throw std::runtime_error(
                        "original route v2 missing QMFC2 high64 target; use v3");
                const auto projected=ae3::originalPolygonRaycast_4c1b3c(
                    records,links,at(actor),originalRayA1,controller40,recursions);
                return projected.originalTraversal.originalLastAcceptedPolygon;
            }
            void call_original_57c8ec_update_target(
                const ae3::OriginalSpatialRecord70* target) override {
                std::uint64_t bits=0;
                if (owner.original_apply_target_57c8ec(
                        owner.user,at(target),&bits)!=1)
                    throw std::runtime_error(
                        "original source 0x57C8EC manager failed to produce target vector");
                // Deliberately never use LOW64 to construct original
                // 0x57C894 projection lanes. The genuine next ray requires
                // the HIGH64 half, available only in the route v3 contract.
                targetRayA2=ae3::OriginalVU0Vec4{};
            }
        } math(records,links,*owner,native(v1),native(v2),
               source->original_max_recursions);
        ae3::OriginalActorRouteFollowerState_57c7cc actor{
            records,
            std::span<const uint32_t>(info.original_route_indices,
                                      info.original_route_count),
            &records[info.actor_polygon_index],
            info.active_target_polygon_index==UINT32_MAX
                ?nullptr:&records[info.active_target_polygon_index],
            info.original_has_multiple_polygons1c!=0
        };
        const auto followed=ae3::originalActorAdvanceRoute_57c7cc(actor,math);
        Ae3OriginalRouteResultV1 value{
            actor.activePolygonAtController50==nullptr
                ?UINT32_MAX:math.at(actor.activePolygonAtController50),
            static_cast<uint32_t>(followed.sourceTargetsApplied),
            static_cast<uint32_t>(followed.sourceProjectionsPerformed),
            static_cast<uint32_t>(followed.followingRouteIndicesTested),
            followed.returnedEarlyBecauseCurrentTargetMatched?1:0
        };
        *out=value;
        session->error.clear();
        completed=1;
    } catch (const std::exception& ex) {
        session->error=ex.what();
    } catch (...) {
        session->error="unknown original 0x57C7CC native ray route error";
    }
    session->busy=false;
    return completed;
}


/* Fully recovered portions of the actual 0x57C8EC dispatch: use the
 * genuine 0x21B788 predicate and real 0x57C98C RNG/triangle blending.
 * The original 0x57CA68 fallback VU0 math is not recovered and MUST
 * reject rather than pretending to generate an accurate target.
 */
/* Full original 128-bit QMFC2 target return including the native
 * translated 0x57CA68 six-candidate VU0 fallback. The caller provides
 * authentic actor owner vector, COP2 Q and VU0 R only when that fallback
 * is needed. RNG state is published only after successful source logic.
 */
extern "C" int ae3_original_actor_target_57c8ec_v2(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalActorTargetRequestV2* source,
    Ae3OriginalActorTargetResultV2* out) {
    if (!session) return 0;
    if (session->busy) {
        session->error="reentrant original 0x57C8EC target calculation refused";
        return 0;
    }
    session->error.clear();
    session->busy=true;
    int success=0;
    try {
        if (!source || !out || !source->original.original_record70 ||
            source->original.original_point_count==0 ||
            source->original.original_point_count>3)
            throw std::invalid_argument(
                "original target requires source record and verified 0x1E descriptor count");
        const auto& control=source->original.original_controller20;
        if (!std::isfinite(control.x) || !std::isfinite(control.y) ||
            !std::isfinite(control.z) || !std::isfinite(control.w))
            throw std::invalid_argument("source original target controller20 nonfinite");
        ae3::OriginalSpatialRecord70 record{};
        std::memcpy(record.originalFields.data(),
                    source->original.original_record70,0x70);
        std::array<ae3::OriginalVU0Vec4,3> vertices{};
        for (std::size_t i=0;i<vertices.size();++i)
            std::memcpy(&vertices[i],record.originalFields.data()+0x40+i*16,16);
        std::array<std::byte,16> controller{};
        std::memcpy(controller.data(),&control,16);
        std::array<std::byte,16> ownerVector{};
        std::memcpy(ownerVector.data(),&source->original_owner14_vector40,16);
        auto tentativeRandom=session->originalRandom;
        ae3::OriginalActorFallbackVu0R_57ca68 vu0R{};
        vu0R.rawR=source->original_vu0_r_bits;
        ae3::OriginalActorFallbackNativeVU0_57ca68 nativeVU0{};
        struct SourceTargetMath final:ae3::OriginalActorTargetGeometryCalls_57c8ec {
            std::span<const ae3::OriginalVU0Vec4> vertices;
            ae3::OriginalVU0Vec4 controller;
            const std::array<std::byte,16>& ownerVector;
            const std::array<std::byte,16>& controllerVector;
            ae3::OriginalGameRandom_36f3c0& originalRandom;
            ae3::OriginalActorFallbackVu0R_57ca68& vu0R;
            ae3::OriginalActorFallbackNativeVU0_57ca68& nativeVU0;
            const Ae3OriginalActorTargetRequestV2& source;
            std::uint64_t lastFallbackHigh=0,lastTriangleHigh=0;
            SourceTargetMath(
                std::span<const ae3::OriginalVU0Vec4> originalPoints,
                ae3::OriginalVU0Vec4 originalController,
                const std::array<std::byte,16>& owner,
                const std::array<std::byte,16>& input,
                ae3::OriginalGameRandom_36f3c0& random,
                ae3::OriginalActorFallbackVu0R_57ca68& r,
                ae3::OriginalActorFallbackNativeVU0_57ca68& vu0,
                const Ae3OriginalActorTargetRequestV2& request)
                :vertices(originalPoints),controller(originalController),
                 ownerVector(owner),controllerVector(input),originalRandom(random),
                 vu0R(r),nativeVU0(vu0),source(request){}
            bool call_original_21b788_eligibility(
                const ae3::OriginalSpatialRecord70&,float threshold) override {
                return ae3::originalVU0PolygonEligible_21b788(
                    vertices,controller,threshold);
            }
            std::uint64_t call_original_57ca68_geometry(
                const ae3::OriginalSpatialRecord70& polygon) override {
                if (source.original_fallback_operands_available!=1)
                    throw std::runtime_error(
                        "original 0x57CA68 owner vector/Q/R operands missing");
                const auto result=ae3::originalActorFallbackSelect_57ca68(
                    polygon,ownerVector,controllerVector,
                    source.original_q_bits,originalRandom,nativeVU0,&vu0R);
                lastFallbackHigh=result.originalWinningPackedTargetHigh64;
                return result.originalWinningPackedTargetLow64;
            }
            std::uint64_t call_original_57ca68_geometry_high64()
                const override {return lastFallbackHigh;}
            std::uint64_t call_original_57c98c_three_vertex_transform(
                const ae3::OriginalSpatialRecord70&,
                const std::array<std::array<std::byte,16>,3>& points) override {
                const auto result=ae3::originalActorRandomTrianglePacked_57c98c(
                    points,originalRandom);
                lastTriangleHigh=result.originalHigh64;
                return result.originalLow64;
            }
            std::uint64_t call_original_57c98c_geometry_high64()
                const override {return lastTriangleHigh;}
        } math(std::span<const ae3::OriginalVU0Vec4>(
                    vertices.data(),source->original.original_point_count),
               native(control),ownerVector,controller,tentativeRandom,
               vu0R,nativeVU0,*source);
        const auto original=ae3::originalActorTargetVector_57c8ec(
            record,source->original.original_variant30!=0,controller,math);
        int branch=original.branch==
            ae3::OriginalActorTargetBranch_57c8ec::
                OriginalVariantVectorPassThrough_57c944 ? 1 :
            original.branch==ae3::OriginalActorTargetBranch_57c8ec::
                OriginalThreeVertexTransform_57c98c ? 2 : 3;
        const Ae3OriginalActorTargetResultV2 result{
            original.originalVectorLow64,original.originalVectorHigh64,
            branch,vu0R.rawR
        };
        session->originalRandom=std::move(tentativeRandom);
        *out=result;
        session->error.clear();
        success=1;
    } catch (const std::exception& e) {
        session->error=e.what();
    } catch (...) {
        session->error="unknown original 0x57C8EC native target failure";
    }
    session->busy=false;
    return success;
}
/* Low64-only ABI is deliberately fail-closed when original QMFC2
 * contains meaningful high64. Never tell Unreal that a truncated target
 * is the genuine controller result.
 */
extern "C" int ae3_original_actor_target_57c8ec(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalActorTargetRequestV1* source,
    Ae3OriginalActorTargetResultV1* out) {
    if (!session) return 0;
    if (session->busy) {
        session->error="reentrant original low64 target API refused";
        return 0;
    }
    if (!source || !out) {
        session->error="missing original low64 target request/output";
        return 0;
    }
    Ae3OriginalManagerInitSession scratch(0);
    scratch.originalRandom=session->originalRandom;
    Ae3OriginalActorTargetRequestV2 request{};
    request.original=*source;
    Ae3OriginalActorTargetResultV2 full{};
    if (ae3_original_actor_target_57c8ec_v2(&scratch,&request,&full)!=1) {
        session->error=scratch.error;
        return 0;
    }
    if (full.original_high64!=0) {
        session->error="source QMFC2 target is 128-bit; use original target ABI v2";
        return 0;
    }
    session->originalRandom=std::move(scratch.originalRandom);
    *out={full.original_low64,full.original_branch};
    session->error.clear();
    return 1;
}

/* Fully recovered target/route path:
 * original 0x57C7CC → 0x4C1B3C → 0x21BD3C → 0x21EE00
 * + real 0x57C8EC 128-bit target (0x57C98C or 0x57CA68).
 * Original controller update remains a real owner transaction.
 */
struct OriginalRouteTargetOwnerV3 {
    Ae3OriginalManagerInitSession proposed;
    const Ae3OriginalRouteSourceV3& source;
    const Ae3OriginalRouteCallsV3& manager;
    std::uint32_t latestR;
    OriginalRouteTargetOwnerV3(
        const Ae3OriginalManagerInitSession& live,
        const Ae3OriginalRouteSourceV3& input,
        const Ae3OriginalRouteCallsV3& originalOwner)
        :proposed(0),source(input),manager(originalOwner),
         latestR(input.original_vu0_r_bits) {
        proposed.originalRandom=live.originalRandom;
    }
    std::pair<std::uint64_t,std::uint64_t> stage(uint32_t polygon) {
        if (polygon>=source.original.original.original_record_count)
            throw std::out_of_range("original actor target outside owner stage");
        const auto* originalBytes=static_cast<const std::byte*>(
            source.original.original.original_records70);
        Ae3OriginalActorTargetRequestV2 request{};
        request.original.original_record70=originalBytes+polygon*0x70;
        request.original.original_point_count=
            source.original_polygon_point_counts[polygon];
        request.original.original_variant30=source.original_controller_variant30;
        request.original.original_controller20=source.original_controller20;
        request.original_owner14_vector40=source.original_owner14_vector40;
        request.original_q_bits=source.original_q_bits;
        request.original_vu0_r_bits=latestR;
        request.original_fallback_operands_available=
            source.original_fallback_operands_available;
        Ae3OriginalActorTargetResultV2 calculated{};
        if (ae3_original_actor_target_57c8ec_v2(
                &proposed,&request,&calculated)!=1)
            throw std::runtime_error(proposed.error.empty()
                ? "original 0x57C8EC target unavailable" : proposed.error);
        if (manager.stage_original_target_57c8ec(
                manager.user,polygon,calculated.original_low64,
                calculated.original_high64,
                calculated.original_vu0_r_bits)!=1)
            throw std::runtime_error(
                "original 0x57C8EC actor owner refused staged full-width target");
        latestR=calculated.original_vu0_r_bits;
        return {calculated.original_low64,calculated.original_high64};
    }
};

extern "C" int ae3_original_actor_route_step_v3(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalRouteSourceV3* source,
    const Ae3OriginalRouteCallsV3* manager,
    Ae3OriginalRouteResultV1* out) {
    if (!session) return 0;
    if (session->busy) {
        session->error="reentrant original actor route v3 rejected";
        return 0;
    }
    if (!source || !manager || !out ||
        !manager->begin_original_actor_update ||
        !manager->stage_original_target_57c8ec ||
        !manager->commit_original_actor_update ||
        !manager->abort_original_actor_update ||
        !source->original_polygon_point_counts) {
        session->error="original actor route requires real atomic game manager and source counts";
        return 0;
    }
    const auto& s=source->original.original;
    if (s.original_record_count==0 || s.original_record_count>8192 ||
        s.original_route_count>8192 || !s.original_records70 ||
        (s.original_route_count&&!s.original_route_indices) ||
        s.actor_polygon_index>=s.original_record_count ||
        (s.active_target_polygon_index!=UINT32_MAX &&
         s.active_target_polygon_index>=s.original_record_count) ||
        !source->original.original_degree ||
        !source->original.original_neighbor3 ||
        !source->original.original_action_kind3 ||
        source->original.original_max_recursions==0 ||
        source->original.original_max_recursions>4096) {
        session->error="original actor route has invalid owner polygon records/adjacency";
        return 0;
    }
    // A manager may rebind its callback table during Begin; this
    // transaction must execute the same original owner contract throughout.
    const Ae3OriginalRouteCallsV3 stableManager=*manager;
    // The entire native route, including the gap BETWEEN Begin and
    // Commit, has live pointers into this session. In particular, Stage
    // callbacks must not be allowed to destroy it mid-stack.
    session->busy=true;
    auto abortSafely=[&]() {
        try {stableManager.abort_original_actor_update(stableManager.user);}
        catch (...) {} // preserve original error at the C ABI boundary
    };
    bool began=false;
    try {
        std::vector<ae3::OriginalSpatialRecord70> records(
            s.original_record_count);
        std::vector<ae3::OriginalPolygonLinks_21bd3c> links(
            s.original_record_count);
        const auto* originalBytes=
            static_cast<const std::byte*>(s.original_records70);
        for (std::size_t polygon=0;polygon<records.size();++polygon) {
            std::memcpy(records[polygon].originalFields.data(),
                originalBytes+polygon*0x70,0x70);
            if (source->original_polygon_point_counts[polygon]<1 ||
                source->original_polygon_point_counts[polygon]>3)
                throw std::out_of_range(
                    "original polygon 0x1E descriptor has invalid vertex count");
            const auto degree=source->original.original_degree[polygon];
            if (degree>3)
                throw std::out_of_range("original polygon adjacency degree >3");
            links[polygon].edgeCount=degree;
            for (std::size_t edge=0;edge<degree;++edge) {
                const auto i=polygon*3+edge;
                const auto neighbor=source->original.original_neighbor3[i];
                const auto kind=source->original.original_action_kind3[i];
                if (neighbor>=records.size()||kind>2)
                    throw std::out_of_range(
                        "original actor adjacency has invalid neighbor/type");
                links[polygon].edges[edge]={neighbor,kind};
            }
        }
        // A manager Begin callback can trigger engine events. All input used
        // by a route transaction must therefore be fixed BEFORE that call.
        // Spatial records were already copied above; snapshot the original
        // point-count descriptors and route indices as well. Target staging
        // must never reread mutable caller buffers after Begin.
        const std::vector<std::uint16_t> fixedPointCounts(
            source->original_polygon_point_counts,
            source->original_polygon_point_counts+s.original_record_count);
        std::vector<std::uint32_t> fixedRouteIndices;
        if (s.original_route_count)
            fixedRouteIndices.assign(
                s.original_route_indices,
                s.original_route_indices+s.original_route_count);
        Ae3OriginalRouteSourceV3 fixedSource=*source;
        fixedSource.original.original.original_records70=records.data();
        fixedSource.original.original.original_route_indices=
            fixedRouteIndices.data();
        fixedSource.original_polygon_point_counts=fixedPointCounts.data();
        const auto& fixedRoute=fixedSource.original.original;

        const auto& a=fixedSource.original.original_incoming_a1;
        const auto& b=fixedSource.original.original_controller40;
        if (!std::isfinite(a.x)||!std::isfinite(a.y)||
            !std::isfinite(a.z)||!std::isfinite(a.w)||
            !std::isfinite(b.x)||!std::isfinite(b.y)||
            !std::isfinite(b.z)||!std::isfinite(b.w))
            throw std::invalid_argument("original route projection vectors invalid");

        OriginalRouteTargetOwnerV3 proposed(*session,fixedSource,stableManager);
        struct OriginalFullWidthRoute final :
            ae3::OriginalActorRouteFollowerCalls_57c7cc {
            std::span<const ae3::OriginalSpatialRecord70> records;
            std::span<const ae3::OriginalPolygonLinks_21bd3c> links;
            OriginalRouteTargetOwnerV3& owner;
            ae3::OriginalVU0Vec4 rayStart,controller40;
            std::optional<ae3::OriginalVU0Vec4> updatedProjectionA2;
            std::uint32_t recursionLimit;
            OriginalFullWidthRoute(
                std::span<const ae3::OriginalSpatialRecord70> sourceRecords,
                std::span<const ae3::OriginalPolygonLinks_21bd3c> sourceLinks,
                OriginalRouteTargetOwnerV3& owner,
                ae3::OriginalVU0Vec4 start,
                ae3::OriginalVU0Vec4 originalController40,
                std::uint32_t maximum)
                :records(sourceRecords),links(sourceLinks),owner(owner),
                 rayStart(start),controller40(originalController40),
                 recursionLimit(maximum){}
            std::uint32_t index(
                const ae3::OriginalSpatialRecord70* sourcePolygon) const {
                for (std::size_t i=0;i<records.size();++i)
                    if (sourcePolygon==&records[i])
                        return static_cast<std::uint32_t>(i);
                throw std::invalid_argument(
                    "original actor returned polygon outside its stage");
            }
            const ae3::OriginalSpatialRecord70* call_original_4c1b3c_project(
                const ae3::OriginalSpatialRecord70* actor) override {
                const auto& rayEnd=updatedProjectionA2
                    ?*updatedProjectionA2:controller40;
                const auto rayStartFromSource=updatedProjectionA2
                    ?ae3::originalActorPackedSubsequentRayStart_57c88c(rayStart)
                    :rayStart;
                const auto ray=ae3::originalPolygonRaycast_4c1b3c(
                    records,links,index(actor),rayStartFromSource,rayEnd,
                    recursionLimit);
                return ray.originalTraversal.originalLastAcceptedPolygon;
            }
            void call_original_57c8ec_update_target(
                const ae3::OriginalSpatialRecord70* target) override {
                const auto [low,high]=owner.stage(index(target));
                // Source 0x57C894 PCPYUD a2,zero,v0:
                // the original QMFC2 UPPER 64 bits become VF.z/w.
                // Do not use v0 low64 as a fictitious XY target.
                updatedProjectionA2=ae3::OriginalVU0Vec4{
                    0.0f,0.0f,
                    std::bit_cast<float>(static_cast<std::uint32_t>(high)),
                    std::bit_cast<float>(static_cast<std::uint32_t>(high>>32))
                };
                (void)low; // owner receives both genuine doublewords.
            }
        } math(records,links,proposed,
               native(a),native(b),source->original.original_max_recursions);

        began=true; // Even rejected Begin may have allocated owner state.
        if (stableManager.begin_original_actor_update(stableManager.user)!=1)
            throw std::runtime_error("original game actor owner refused Begin");

        ae3::OriginalActorRouteFollowerState_57c7cc originalRoute{
            records,
            std::span<const std::uint32_t>(fixedRouteIndices),
            &records[fixedRoute.actor_polygon_index],
            fixedRoute.active_target_polygon_index==UINT32_MAX
                ?nullptr:&records[fixedRoute.active_target_polygon_index],
            fixedRoute.original_has_multiple_polygons1c!=0
        };
        // This executes the ACTUAL recovered original branch and callback
        // order. The host's output is not touched until Commit completes.
        const auto followed=ae3::originalActorAdvanceRoute_57c7cc(
            originalRoute,math);
        const Ae3OriginalRouteResultV1 result{
            originalRoute.activePolygonAtController50
                ?math.index(originalRoute.activePolygonAtController50)
                :UINT32_MAX,
            static_cast<std::uint32_t>(followed.sourceTargetsApplied),
            static_cast<std::uint32_t>(followed.sourceProjectionsPerformed),
            static_cast<std::uint32_t>(followed.followingRouteIndicesTested),
            followed.returnedEarlyBecauseCurrentTargetMatched?1:0
        };
        if (stableManager.commit_original_actor_update(stableManager.user)!=1)
            throw std::runtime_error("original game actor owner refused Commit");
        began=false;
        session->originalRandom=std::move(proposed.proposed.originalRandom);
        *out=result;
        session->error.clear();
        session->busy=false;
        return 1;
    } catch (const std::exception& e) {
        const std::string failure=e.what();
        if (began) abortSafely();
        session->error=failure;
    } catch (...) {
        if (began) abortSafely();
        session->error="original native full-width actor route failed";
    }
    session->busy=false;
    return 0;
}
