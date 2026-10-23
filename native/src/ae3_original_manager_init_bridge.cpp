#include "ae3/ae3_unreal_lua_bridge.h"
#include "ae3/original_manager_initializer.hpp"

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
        ae3::OriginalManagerChild70 child{};
        child.word08 = state->source_method08;
        child.word0c = state->source_method0c;
        child.flag1c = state->source_flag1c;
        child.alternative30 = state->source_alternative30;
        ae3::OriginalManagerInitState_57bcb8 sourceState{
            child, native(state->controller20), native(state->target40),
            nullptr, native(state->midpoint60)
        };
        if (state->target_polygon_index != UINT32_MAX) {
            if (state->target_polygon_index >= records.size())
                throw std::out_of_range("original target polygon outside source stage");
            sourceState.target50 = &records[state->target_polygon_index];
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
        Ae3OriginalManagerInitStateV1 completed = *state;
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
