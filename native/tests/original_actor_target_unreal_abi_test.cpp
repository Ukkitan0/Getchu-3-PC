#include "ae3/ae3_unreal_lua_bridge.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>

static Ae3OriginalActorTargetRequestV2 genuineSource(
    const std::array<uint8_t,0x70>& record,int variant,float x,float z) {
    Ae3OriginalActorTargetRequestV2 result{};
    result.original.original_record70=record.data();
    result.original.original_point_count=3;
    result.original.original_variant30=variant;
    result.original.original_controller20={x,0,z,1};
    return result;
}
int main() {
    std::array<uint8_t,0x70> source{};
    const std::array<Ae3OriginalVec4,3> vertices{{
        {0,0,0,1},{0,0,10,1},{10,0,0,1}
    }};
    std::memcpy(source.data()+0x40,vertices.data(),sizeof(vertices));
    auto* a=ae3_original_manager_init_create(5489);
    auto* b=ae3_original_manager_init_create(5489);
    auto* c=ae3_original_manager_init_create(5489);
    assert(a&&b&&c);

    // Real 0x57C944 QMFC2 returns BOTH doublewords from controller+0x20.
    auto pass=genuineSource(source,1,2,2);
    Ae3OriginalActorTargetResultV2 target{};
    assert(ae3_original_actor_target_57c8ec_v2(a,&pass,&target)==1);
    assert(target.original_branch==1);
    std::array<float,4> vector{};
    std::memcpy(vector.data(),&target.original_low64,8);
    std::memcpy(vector.data()+2,&target.original_high64,8);
    assert(vector[0]==2&&vector[1]==0&&vector[2]==2&&vector[3]==1);

    // The old low64-only C ABI must not accept a truncated original target.
    Ae3OriginalActorTargetResultV1 narrow{};
    assert(ae3_original_actor_target_57c8ec(
        a,&pass.original,&narrow)==0);
    assert(std::strstr(ae3_original_manager_init_last_error(a),
                       "128-bit")!=nullptr);

    // Fully recovered original 0x57C98C: source PCPYUD zeroes X/Y and
    // uses original triangle's Z/W in the HIGH 64, with exact EE RNG.
    auto triangle=genuineSource(source,0,2,2);
    Ae3OriginalActorTargetResultV2 first{},seeded{},second{};
    assert(ae3_original_actor_target_57c8ec_v2(a,&triangle,&first)==1);
    assert(ae3_original_actor_target_57c8ec_v2(b,&triangle,&seeded)==1);
    assert(first.original_branch==2&&seeded.original_branch==2);
    assert(first.original_low64==0&&first.original_high64!=0);
    assert(first.original_high64==seeded.original_high64);
    assert(ae3_original_actor_target_57c8ec_v2(a,&triangle,&second)==1);
    assert(second.original_high64!=first.original_high64);
    assert(ae3_original_actor_target_57c8ec_v2(c,&pass,&target)==1);
    Ae3OriginalActorTargetResultV2 afterPass{};
    assert(ae3_original_actor_target_57c8ec_v2(c,&triangle,&afterPass)==1);
    assert(afterPass.original_high64==first.original_high64);

    // Source 0x57CA68: 6 native VU0 candidates, genuine EE RNG, original
    // COP2 Q and R, 128-bit result (LOW64 zero, HIGH64 nonzero).
    auto outside=genuineSource(source,1,30,30);
    target={UINT64_MAX,UINT64_MAX,77,55};
    assert(ae3_original_actor_target_57c8ec_v2(a,&outside,&target)==0);
    assert(target.original_branch==77&&target.original_high64==UINT64_MAX);
    assert(std::strstr(ae3_original_manager_init_last_error(a),
                       "owner vector/Q/R operands missing")!=nullptr);
    outside.original_owner14_vector40={0,0,0,1};
    outside.original_q_bits=0x3f800000u; // authentic caller-provided Q=1
    outside.original_vu0_r_bits=0;
    outside.original_fallback_operands_available=1;
    assert(ae3_original_actor_target_57c8ec_v2(a,&outside,&target)==1);
    assert(target.original_branch==3);
    assert(target.original_low64==0&&target.original_high64!=0);
    assert(target.original_vu0_r_bits==0x3f800000u);

    // Fail closed without consuming original EE RNG or changing output.
    triangle.original.original_point_count=0;
    target={UINT64_MAX,UINT64_MAX,77,55};
    assert(ae3_original_actor_target_57c8ec_v2(a,&triangle,&target)==0);
    assert(target.original_high64==UINT64_MAX&&target.original_branch==77);
    triangle.original.original_point_count=3;
    triangle.original.original_record70=nullptr;
    assert(ae3_original_actor_target_57c8ec_v2(a,&triangle,&target)==0);
    triangle.original.original_record70=source.data();
    triangle.original.original_controller20.x=
        std::numeric_limits<float>::quiet_NaN();
    assert(ae3_original_actor_target_57c8ec_v2(a,&triangle,&target)==0);

    assert(ae3_original_manager_init_destroy(a)==1);
    assert(ae3_original_manager_init_destroy(b)==1);
    assert(ae3_original_manager_init_destroy(c)==1);
    std::cout<<"ORIGINAL_TARGET_57C8EC_NATIVE_FULL128_FALLBACK_PASSED\n";
    return 0;
}
