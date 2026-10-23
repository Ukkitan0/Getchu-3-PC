#include "ae3/ae3_unreal_lua_bridge.h"
#include "ae3/original_manager_initializer.hpp"
#include "ae3/original_actor_route_follower.hpp"

#include <array>
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

extern "C" int ae3_original_manager_init_step(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalManagerInitSourceV1* source,
    Ae3OriginalManagerInitStateV1* state,
    const Ae3OriginalManagerInitCallsV1* originalCallbacks) {
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
        RealActorCalls calls(originalCallbacks);
        const auto branch = ae3::originalManagerInitialize_57bcb8(
            sourceState,stage,session->originalRandom,calls);
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
