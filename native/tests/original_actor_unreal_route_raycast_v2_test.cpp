#include "ae3/ae3_unreal_lua_bridge.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>

struct GenuineOriginalTarget {
    unsigned updates=0;
    bool accepted=true;
};
static int update57c8ec(void* opaque,uint32_t polygon,uint64_t* packed) {
    auto& owner=*static_cast<GenuineOriginalTarget*>(opaque);
    assert(polygon==0 && packed);
    owner.updates++;
    if (!owner.accepted)return 0;
    // Source DMFC2 low64 of two original float32 target lanes.
    const std::array<float,2> originalTarget{3.0f,2.0f};
    std::memcpy(packed,originalTarget.data(),sizeof(uint64_t));
    return 1;
}
int main() {
    auto* manager=ae3_original_manager_init_create(5489);
    assert(manager);
    std::array<uint8_t,0x70> sourcePolygon{};
    const std::array<Ae3OriginalVec4,3> originalVertices{{
        {0,0,0,1},{0,0,10,1},{10,0,0,1}
    }};
    std::memcpy(sourcePolygon.data()+0x40,originalVertices.data(),
                sizeof(originalVertices));
    const std::array<uint8_t,1> degree{0};
    const std::array<uint32_t,3> neighbor{0,0,0};
    const std::array<uint16_t,3> kinds{0,0,0};
    const std::array<uint32_t,2> route{0,0};
    Ae3OriginalRouteSourceV2 input{};
    input.original.original_records70=sourcePolygon.data();
    input.original.original_record_count=1;
    input.original.actor_polygon_index=0;
    input.original.active_target_polygon_index=UINT32_MAX;
    input.original.original_route_indices=route.data();
    input.original.original_route_count=2;
    input.original.original_has_multiple_polygons1c=1;
    input.original_degree=degree.data();
    input.original_neighbor3=neighbor.data();
    input.original_action_kind3=kinds.data();
    input.original_incoming_a1={2,0,2,1};
    input.original_controller40={3,0,2,1};
    input.original_max_recursions=16;
    GenuineOriginalTarget actual{};
    Ae3OriginalRouteCallsV2 calls{&actual,update57c8ec};
    Ae3OriginalRouteResultV1 output{};
    output.active_target_polygon_index=99;

    // No active original target: recovered 0x57C7CC chooses route[1]
    // and original 0x57C8EC returns packed source target vectors.
    assert(ae3_original_actor_route_step_v2(
        manager,&input,&calls,&output)==1);
    assert(output.active_target_polygon_index==0 &&
           output.original_targets_applied==1 &&
           output.original_projections_performed==0 &&
           actual.updates==1);

    // A live original target triggers real 0x4C1B3C polygon raycast.
    // The source ray is inside this triangle: early return, no update.
    input.original.active_target_polygon_index=0;
    assert(ae3_original_actor_route_step_v2(
        manager,&input,&calls,&output)==1);
    assert(output.original_early_current_target_match==1 &&
           output.original_projections_performed==1 &&
           output.original_targets_applied==0 &&
           actual.updates==1);

    // An outward ray misses; the recovered 0x57C7CC now reapplies route[1].
    input.original_incoming_a1={-2,0,2,1};
    input.original_controller40={-3,0,2,1};
    assert(ae3_original_actor_route_step_v2(
        manager,&input,&calls,&output)==1);
    assert(output.original_early_current_target_match==0 &&
           output.original_targets_applied==1 &&
           output.active_target_polygon_index==0 &&
           actual.updates==2);

    // A route[2] follow-up would require the original QMFC2 UPPER
    // doubleword after 0x57C8EC. This legacy low64 callback cannot
    // supply it; reject BEFORE touching owner targets/output. V3 uses
    // the full 128-bit target and is the supported continuation path.
    const std::array<uint32_t,3> extendedRoute{0,0,0};
    input.original.original_route_indices=extendedRoute.data();
    input.original.original_route_count=extendedRoute.size();
    const auto beforeLegacy=output;
    const auto beforeUpdates=actual.updates;
    assert(ae3_original_actor_route_step_v2(
        manager,&input,&calls,&output)==0);
    assert(actual.updates==beforeUpdates &&
           output.active_target_polygon_index==beforeLegacy.active_target_polygon_index &&
           output.original_targets_applied==beforeLegacy.original_targets_applied);
    assert(std::strstr(ae3_original_manager_init_last_error(manager),
                       "full 128-bit")!=nullptr);
    input.original.original_route_indices=route.data();
    input.original.original_route_count=route.size();

    // Missing real 0x57C8EC owner must not publish route results.
    actual.accepted=false;
    const auto before=output;
    assert(ae3_original_actor_route_step_v2(
        manager,&input,&calls,&output)==0);
    assert(actual.updates==3 &&
           output.active_target_polygon_index==before.active_target_polygon_index &&
           output.original_targets_applied==before.original_targets_applied);
    assert(std::strstr(ae3_original_manager_init_last_error(manager),
                       "0x57C8EC")!=nullptr);

    // Malformed source graph cannot be quietly replaced by Unreal collision.
    input.original_degree=nullptr;
    assert(ae3_original_actor_route_step_v2(
        manager,&input,&calls,&output)==0);
    assert(ae3_original_manager_init_destroy(manager)==1);
    std::cout<<"ORIGINAL_ACTOR_57C7CC_PLUS_4C1B3C_RAYCAST_SOURCE_PASSED\n";
    return 0;
}
