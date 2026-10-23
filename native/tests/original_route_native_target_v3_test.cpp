#include "ae3/ae3_unreal_lua_bridge.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>

struct OriginalOwner {
    bool allowBegin=true,allowStage=true,allowCommit=true;
    bool inTransaction=false;
    unsigned begins=0,stages=0,commits=0,aborts=0;
    uint64_t pendingLow=0,pendingHigh=0,lastLow=0,lastHigh=0;
    uint32_t pendingR=0,lastR=0;
};
int begin(void* opaque) {
    auto& owner=*static_cast<OriginalOwner*>(opaque);
    assert(!owner.inTransaction);
    ++owner.begins;
    owner.inTransaction=true;
    return owner.allowBegin;
}
int stage(void* opaque,uint32_t polygon,uint64_t low,uint64_t high,uint32_t r) {
    auto& owner=*static_cast<OriginalOwner*>(opaque);
    assert(owner.inTransaction&&polygon==0);
    ++owner.stages;
    if(!owner.allowStage)return 0;
    owner.pendingLow=low;
    owner.pendingHigh=high;
    owner.pendingR=r;
    return 1;
}
int commit(void* opaque) {
    auto& owner=*static_cast<OriginalOwner*>(opaque);
    assert(owner.inTransaction);
    ++owner.commits;
    if(!owner.allowCommit)return 0;
    owner.lastLow=owner.pendingLow;
    owner.lastHigh=owner.pendingHigh;
    owner.lastR=owner.pendingR;
    owner.inTransaction=false;
    return 1;
}
void abort(void* opaque) {
    auto& owner=*static_cast<OriginalOwner*>(opaque);
    assert(owner.inTransaction);
    ++owner.aborts;
    owner.inTransaction=false;
    owner.pendingLow=owner.pendingHigh=0;
    owner.pendingR=0;
}
int main() {
    std::array<uint8_t,0x70> polygon{};
    const std::array<Ae3OriginalVec4,3> vertices{{
        {0,0,0,1},{0,0,10,1},{10,0,0,1}
    }};
    std::memcpy(polygon.data()+0x40,vertices.data(),sizeof(vertices));
    const std::array<uint8_t,1> degree{0};
    const std::array<uint32_t,3> neighbors{0,0,0};
    const std::array<uint16_t,3> actions{0,0,0};
    std::array<uint16_t,1> counts{3};
    Ae3OriginalRouteSourceV3 source{};
    source.original.original.original_records70=polygon.data();
    source.original.original.original_record_count=1;
    source.original.original.actor_polygon_index=0;
    source.original.original.active_target_polygon_index=UINT32_MAX;
    source.original.original.original_has_multiple_polygons1c=0;
    source.original.original_degree=degree.data();
    source.original.original_neighbor3=neighbors.data();
    source.original.original_action_kind3=actions.data();
    source.original.original_max_recursions=8;
    source.original.original_incoming_a1={2,0,2,1};
    source.original.original_controller40={3,0,2,1};
    source.original_polygon_point_counts=counts.data();
    source.original_controller20={2,0,2,1};
    source.original_controller_variant30=1;

    auto* a=ae3_original_manager_init_create(5489);
    auto* b=ae3_original_manager_init_create(5489);
    assert(a&&b);
    OriginalOwner owner{},other{};
    Ae3OriginalRouteCallsV3 callbacks{&owner,begin,stage,commit,abort};
    Ae3OriginalRouteCallsV3 control{&other,begin,stage,commit,abort};
    Ae3OriginalRouteResultV1 result{};
    result.active_target_polygon_index=UINT32_MAX;

    // Genuine original 0x57C7CC + 0x57C8EC fast path.
    assert(ae3_original_actor_route_step_v3(a,&source,&callbacks,&result)==1);
    assert(result.active_target_polygon_index==0 &&
           result.original_targets_applied==1 &&
           owner.commits==1 && !owner.inTransaction);
    const std::array<float,2> xy{2,0},zw{2,1};
    uint64_t expectedLow=0,expectedHigh=0;
    std::memcpy(&expectedLow,xy.data(),8);
    std::memcpy(&expectedHigh,zw.data(),8);
    assert(owner.lastLow==expectedLow&&owner.lastHigh==expectedHigh);

    // Original 0x57C98C triangle result lives in GPR HIGH64.
    source.original_controller_variant30=0;
    owner.allowCommit=false;
    const auto before=result;
    assert(ae3_original_actor_route_step_v3(a,&source,&callbacks,&result)==0);
    assert(result.active_target_polygon_index==before.active_target_polygon_index &&
           owner.lastHigh==expectedHigh && owner.aborts==1);
    owner.allowCommit=true;
    assert(ae3_original_actor_route_step_v3(a,&source,&callbacks,&result)==1);
    const auto firstHigh=owner.lastHigh;
    assert(owner.lastLow==0 && firstHigh!=0);
    assert(ae3_original_actor_route_step_v3(b,&source,&control,&result)==1);
    assert(other.lastHigh==firstHigh); // Failed commit did not advance RNG.
    assert(ae3_original_actor_route_step_v3(a,&source,&callbacks,&result)==1);
    assert(owner.lastHigh!=firstHigh);
    assert(ae3_original_actor_route_step_v3(b,&source,&control,&result)==1);
    assert(owner.lastHigh==other.lastHigh);

    // Failed VU0 fallback without REAL source actor owner/Q/R fails closed.
    source.original_controller_variant30=1;
    source.original_controller20={50,0,50,1};
    const auto lastCommit=owner.commits;
    assert(ae3_original_actor_route_step_v3(a,&source,&callbacks,&result)==0);
    assert(owner.commits==lastCommit && owner.aborts==2);
    assert(std::strstr(ae3_original_manager_init_last_error(a),
                       "0x57CA68")!=nullptr);
    // Parallel VU0 workstream recovered the fixed 0x57CA68 candidate
    // transforms. Authentic owner+0x14->+0x40 and Q/R make them executable.
    source.original_owner14_vector40={0,0,0,1};
    source.original_q_bits=0x3f800000u; // genuine input Q=1
    source.original_vu0_r_bits=0;
    source.original_fallback_operands_available=1;
    assert(ae3_original_actor_route_step_v3(a,&source,&callbacks,&result)==1);
    assert(owner.lastLow==0 && owner.lastHigh!=0);
    assert(owner.lastR==0x3f800000u);

    // An original Begin or staged target failure MUST invoke Abort.
    source.original_controller20={2,0,2,1};
    owner.allowBegin=false;
    assert(ae3_original_actor_route_step_v3(a,&source,&callbacks,&result)==0);
    assert(owner.aborts==3&&!owner.inTransaction);
    owner.allowBegin=true;
    owner.allowStage=false;
    assert(ae3_original_actor_route_step_v3(a,&source,&callbacks,&result)==0);
    assert(owner.aborts==4&&!owner.inTransaction);
    owner.allowStage=true;
    source.original_polygon_point_counts=nullptr;
    assert(ae3_original_actor_route_step_v3(a,&source,&callbacks,&result)==0);
    source.original_polygon_point_counts=counts.data();
    counts[0]=4;
    assert(ae3_original_actor_route_step_v3(a,&source,&callbacks,&result)==0);

    assert(ae3_original_manager_init_destroy(a)==1);
    assert(ae3_original_manager_init_destroy(b)==1);
    std::cout<<"ORIGINAL_ROUTE_57C7CC_FULL128_TARGET_AND_VU0_PASSED\n";
    return 0;
}
