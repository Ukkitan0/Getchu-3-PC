#include "ae3/ae3_unreal_lua_bridge.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

struct GenuineRouteOwner {
    std::vector<uint32_t> applied;
    std::vector<uint32_t> projected;
    size_t cursor=0;
    int projectionCalls=0;
    int destroyDuringCallback=-1;
    bool rejectUpdate=false;
    Ae3OriginalManagerInitSession* activeSession=nullptr;
};
static int project(void* opaque,uint32_t actor,uint32_t* result) {
    auto& owner=*static_cast<GenuineRouteOwner*>(opaque);
    assert(actor==0 && result);
    ++owner.projectionCalls;
    assert(owner.cursor<owner.projected.size());
    *result=owner.projected[owner.cursor++];
    return 1;
}
static int apply(void* opaque,uint32_t target) {
    auto& owner=*static_cast<GenuineRouteOwner*>(opaque);
    owner.applied.push_back(target);
    if (owner.activeSession)
        owner.destroyDuringCallback =
            ae3_original_manager_init_destroy(owner.activeSession);
    return !owner.rejectUpdate;
}

int main() {
    auto* session=ae3_original_manager_init_create(5489);
    assert(session);
    std::array<uint8_t,0x70*5> originalRecords{};
    const std::array<uint32_t,5> route{0,1,2,3,4};
    Ae3OriginalRouteSourceV1 src{};
    src.original_records70=originalRecords.data();
    src.original_record_count=5;
    src.actor_polygon_index=0;
    src.active_target_polygon_index=UINT32_MAX;
    src.original_route_indices=route.data();
    src.original_route_count=route.size();
    GenuineRouteOwner owner{};
    owner.activeSession=session;
    Ae3OriginalRouteCallsV1 calls{&owner,project,apply};
    Ae3OriginalRouteResultV1 result{};
    result.active_target_polygon_index=99;
    // Source 0x57C8D8 single-polygon path applies original actor+0x30.
    assert(ae3_original_actor_route_step(session,&src,&calls,&result)==1);
    assert(result.active_target_polygon_index==0 &&
           result.original_targets_applied==1 &&
           result.original_projections_performed==0 &&
           owner.applied==std::vector<uint32_t>({0}) &&
           owner.destroyDuringCallback==0);
    owner.applied.clear();
    src.original_has_multiple_polygons1c=1;
    src.active_target_polygon_index=0;
    owner.projected={0};
    owner.cursor=0;
    // Original matched active target exits BEFORE any target update.
    assert(ae3_original_actor_route_step(session,&src,&calls,&result)==1);
    assert(result.original_early_current_target_match==1 &&
           result.original_targets_applied==0 &&
           result.original_projections_performed==1 &&
           result.active_target_polygon_index==0 &&
           owner.applied.empty());
    // Current target projection misses; route[1] is applied, then
    // route[2] matches, route[3] differs, and its update is retained.
    owner.projected={UINT32_MAX,2,4};
    owner.cursor=0;
    assert(ae3_original_actor_route_step(session,&src,&calls,&result)==1);
    assert(result.active_target_polygon_index==2 &&
           result.original_targets_applied==3 &&
           result.original_projections_performed==3 &&
           result.original_following_indices==2 &&
           owner.applied==std::vector<uint32_t>({1,2,3}));

    // Failure must not publish a new route result; the real owning
    // manager is responsible for undoing any earlier callback side effects.
    owner.applied.clear();
    owner.rejectUpdate=true;
    const auto previous=result;
    assert(ae3_original_actor_route_step(session,&src,&calls,&result)==0);
    assert(result.active_target_polygon_index==previous.active_target_polygon_index &&
           result.original_targets_applied==previous.original_targets_applied);
    assert(std::strstr(ae3_original_manager_init_last_error(session),
                       "0x57C8EC")!=nullptr);
    owner.rejectUpdate=false;
    // A missing true original projection must reject rather than substitute
    // another geometric ray calculation.
    calls.original_project_4c1b3c=nullptr;
    assert(ae3_original_actor_route_step(session,&src,&calls,&result)==0);
    assert(std::strstr(ae3_original_manager_init_last_error(session),
                       "0x4C1B3C")!=nullptr);
    calls.original_project_4c1b3c=project;
    // Original route destination beyond the decoded retail registry rejects.
    std::array<uint32_t,2> invalidRoute{0,99};
    src.original_route_indices=invalidRoute.data();
    src.original_route_count=2;
    src.active_target_polygon_index=UINT32_MAX;
    assert(ae3_original_actor_route_step(session,&src,&calls,&result)==0);
    assert(std::strstr(ae3_original_manager_init_last_error(session),
                       "route")!=nullptr);
    src.original_record_count=0;
    assert(ae3_original_actor_route_step(session,&src,&calls,&result)==0);
    assert(ae3_original_manager_init_destroy(session)==1);
    std::cout<<"ORIGINAL_ACTOR_ROUTE_57C7CC_SOURCE_ABI_PASSED\n";
    return 0;
}
