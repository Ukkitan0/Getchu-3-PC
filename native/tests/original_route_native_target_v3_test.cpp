#include "ae3/ae3_unreal_lua_bridge.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>

struct ActualActorOwner {
    bool allowCommit=true;
    unsigned commits=0;
    uint64_t lastLow64=0;
};
int sourceTargetCommit(void* opaque,uint32_t polygon,uint64_t bits) {
    auto& owner=*static_cast<ActualActorOwner*>(opaque);
    assert(polygon==0);
    ++owner.commits;
    if (!owner.allowCommit) return 0;
    owner.lastLow64=bits;
    return 1;
}
int main() {
    // Synthetic structural source records with genuine decoded +0x40
    // LQC2 polygon fields; no substitute mesh, Player or actor is created.
    std::array<uint8_t,0x70> polygon{};
    const std::array<Ae3OriginalVec4,3> vertices{{
        {0,0,0,1},{0,0,10,1},{10,0,0,1}
    }};
    std::memcpy(polygon.data()+0x40,vertices.data(),sizeof(vertices));
    std::array<uint8_t,1> degree{0};
    std::array<uint32_t,3> edges{0,0,0};
    std::array<uint16_t,3> kind{0,0,0};
    std::array<uint16_t,1> actualPoints{3};
    Ae3OriginalRouteSourceV3 source{};
    source.original.original.original_records70=polygon.data();
    source.original.original.original_record_count=1;
    source.original.original.actor_polygon_index=0;
    source.original.original.active_target_polygon_index=UINT32_MAX;
    source.original.original.original_has_multiple_polygons1c=0;
    source.original.original_degree=degree.data();
    source.original.original_neighbor3=edges.data();
    source.original.original_action_kind3=kind.data();
    source.original.original_max_recursions=8;
    source.original_incoming_a1={2,0,2,1};
    source.original.original_controller40={3,0,2,1};
    source.original_polygon_point_counts=actualPoints.data();
    source.original_controller20={2,0,2,1};
    source.original_controller_variant30=1;

    auto* session=ae3_original_manager_init_create(5489);
    auto* control=ae3_original_manager_init_create(5489);
    assert(session && control);
    ActualActorOwner actual{};
    Ae3OriginalRouteCallsV3 callbacks{&actual,sourceTargetCommit};
    Ae3OriginalRouteResultV1 result{};
    result.active_target_polygon_index=UINT32_MAX;

    // Fast path: real 0x57C7CC single-route update ->
    // real 0x21B788 eligibility -> 0x57C8EC low64 controller output.
    assert(ae3_original_actor_route_step_v3(
        session,&source,&callbacks,&result)==1);
    assert(result.active_target_polygon_index==0 &&
           result.original_targets_applied==1 &&
           actual.commits==1);
    const std::array<float,2> originalControllerXY{2,0};
    uint64_t originalPassBits=0;
    std::memcpy(&originalPassBits,originalControllerXY.data(),8);
    assert(actual.lastLow64==originalPassBits);

    // Original variant-zero 0x57C98C draws genuine shared RNG.
    source.original_controller_variant30=0;
    actual.allowCommit=false;
    const auto before=result;
    assert(ae3_original_actor_route_step_v3(
        session,&source,&callbacks,&result)==0);
    assert(result.active_target_polygon_index==before.active_target_polygon_index &&
           result.original_targets_applied==before.original_targets_applied);
    assert(std::strstr(ae3_original_manager_init_last_error(session),
                       "refused")!=nullptr);
    actual.allowCommit=true;
    assert(ae3_original_actor_route_step_v3(
        session,&source,&callbacks,&result)==1);
    const auto afterFailedRandom=actual.lastLow64;
    // Failed original manager commit did not consume source game RNG.
    ActualActorOwner expected{};
    Ae3OriginalRouteCallsV3 controlCallbacks{&expected,sourceTargetCommit};
    Ae3OriginalRouteResultV1 controlResult{};
    assert(ae3_original_actor_route_step_v3(
        control,&source,&controlCallbacks,&controlResult)==1);
    assert(afterFailedRandom==expected.lastLow64);

    // The actual game RNG is persistent across original route updates.
    assert(ae3_original_actor_route_step_v3(
        session,&source,&callbacks,&result)==1);
    assert(actual.lastLow64!=afterFailedRandom);
    assert(ae3_original_actor_route_step_v3(
        control,&source,&controlCallbacks,&controlResult)==1);
    assert(actual.lastLow64==expected.lastLow64);

    // Original unrecovered fallback MUST NOT cause owner target commit.
    source.original_controller_variant30=1;
    source.original_controller20={50,0,50,1};
    const auto accepted=actual.commits;
    const auto last=result;
    assert(ae3_original_actor_route_step_v3(
        session,&source,&callbacks,&result)==0);
    assert(actual.commits==accepted &&
           result.active_target_polygon_index==last.active_target_polygon_index);
    assert(std::strstr(ae3_original_manager_init_last_error(session),
                       "0x57CA68")!=nullptr);
    // Source descriptor absent or inconsistent: refuse, never invent count.
    source.original_polygon_point_counts=nullptr;
    assert(ae3_original_actor_route_step_v3(
        session,&source,&callbacks,&result)==0);
    source.original_polygon_point_counts=actualPoints.data();
    actualPoints[0]=4;
    assert(ae3_original_actor_route_step_v3(
        session,&source,&callbacks,&result)==0);

    assert(ae3_original_manager_init_destroy(session)==1);
    assert(ae3_original_manager_init_destroy(control)==1);
    std::cout<<"ORIGINAL_ROUTE_57C7CC_TARGET_57C8EC_COMMIT_PASSED\n";
    return 0;
}
