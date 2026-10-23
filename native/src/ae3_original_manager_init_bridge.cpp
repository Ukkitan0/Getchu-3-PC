#include "ae3/ae3_unreal_lua_bridge.h"
#include "ae3/original_manager_initializer.hpp"
#include "ae3/original_actor_route_follower.hpp"
#include "ae3/original_polygon_raycast.hpp"
#include "ae3/original_actor_target_dispatch.hpp"
#include "ae3/original_actor_random_triangle.hpp"
#include "ae3/original_actor_fallback_sampler.hpp"
#include "ae3/original_actor_fallback_vu0_native.hpp"
#include "ae3/original_vu0_geometry.hpp"
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
        RealActorCalls oldCalls(oldCallbacks);
        RealActorCallsV2 recoveredRouteCalls(sourceCallbacks,records,
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
    // The entire native route, including the gap BETWEEN Begin and
    // Commit, has live pointers into this session. In particular, Stage
    // callbacks must not be allowed to destroy it mid-stack.
    session->busy=true;
    auto abortSafely=[&]() {
        try {manager->abort_original_actor_update(manager->user);}
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

        OriginalRouteTargetOwnerV3 proposed(*session,fixedSource,*manager);
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
        if (manager->begin_original_actor_update(manager->user)!=1)
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
        if (manager->commit_original_actor_update(manager->user)!=1)
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
