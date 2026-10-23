#include "ae3/ae3_unreal_lua_bridge.h"
#include "ae3/original_manager_initializer.hpp"
#include "ae3/original_actor_route_follower.hpp"
#include "ae3/original_polygon_raycast.hpp"
#include <cmath>

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
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
            std::span<const uint32_t>(
                source->original_route_indices,source->original_route_count),
            std::span<const std::byte>(
                static_cast<const std::byte*>(source->original_navigation_bytes),
                source->original_navigation_size)
        };
        RealActorCalls oldCalls(oldCallbacks);
        RealActorCallsV2 recoveredRouteCalls(sourceCallbacks,records,
            std::span<const uint32_t>(source->original_route_indices,
                source->original_route_count),originalPolygon);
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
                // Original 0x57C814 uses controller+0x40 until 0x57C8EC
                // supplies the packed low64 update; 0x57C894 zeroes high64.
                const auto& end=targetRayA2?*targetRayA2:controller40;
                const auto projected=ae3::originalPolygonRaycast_4c1b3c(
                    records,links,at(actor),originalRayA1,end,recursions);
                return projected.originalTraversal.originalLastAcceptedPolygon;
            }
            void call_original_57c8ec_update_target(
                const ae3::OriginalSpatialRecord70* target) override {
                std::uint64_t bits=0;
                if (owner.original_apply_target_57c8ec(
                        owner.user,at(target),&bits)!=1)
                    throw std::runtime_error(
                        "original source 0x57C8EC manager failed to produce target vector");
                targetRayA2=ae3::OriginalVU0Vec4{
                    std::bit_cast<float>(static_cast<uint32_t>(bits)),
                    std::bit_cast<float>(static_cast<uint32_t>(bits>>32)),
                    0,0
                };
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
