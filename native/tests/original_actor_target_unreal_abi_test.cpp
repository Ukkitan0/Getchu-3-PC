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

static Ae3OriginalActorTargetRequestV1 genuineSource(
    const std::array<uint8_t,0x70>& bytes,int variant,float x,float z) {
    Ae3OriginalActorTargetRequestV1 result{};
    result.original_record70=bytes.data();
    result.original_point_count=3;
    result.original_variant30=variant;
    result.original_controller20={x,0,z,1};
    return result;
}
int main() {
    // Only source-compatible, synthetic 0x70 records are used in test.
    // No actor, collision geometry or target is fabricated at runtime.
    std::array<uint8_t,0x70> source{};
    const std::array<Ae3OriginalVec4,3> vertices{{
        {0,0,0,1},{0,0,10,1},{10,0,0,1}
    }};
    std::memcpy(source.data()+0x40,vertices.data(),sizeof(vertices));

    auto* a=ae3_original_manager_init_create(5489);
    auto* b=ae3_original_manager_init_create(5489);
    auto* c=ae3_original_manager_init_create(5489);
    assert(a&&b&&c);

    // Variant 1: fully recovered original polygon eligibility + source
    // DMFC2 low64 passthrough. It MUST NOT draw from shared game RNG.
    auto pass=genuineSource(source,1,2,2);
    Ae3OriginalActorTargetResultV1 target{};
    assert(ae3_original_actor_target_57c8ec(a,&pass,&target)==1);
    assert(target.original_branch==1);
    std::array<float,2> passthrough{};
    std::memcpy(passthrough.data(),&target.original_low64,8);
    assert(passthrough[0]==2.0f&&passthrough[1]==0.0f);

    // The real 0x57CA68 fallback remains untranslated. It must NOT
    // silently treat an outside polygon as a successful native target.
    auto outside=genuineSource(source,1,30,30);
    target={UINT64_MAX,77};
    assert(ae3_original_actor_target_57c8ec(a,&outside,&target)==0);
    assert(target.original_low64==UINT64_MAX&&target.original_branch==77);
    assert(std::strstr(ae3_original_manager_init_last_error(a),
                       "0x57CA68")!=nullptr);

    // Variant 0: source 0x57C98C consumes genuine game RNG and blends
    // the THREE original LQC2 vertex vectors. Compare seeded runtimes,
    // including the fast/failing operations on the first runtime.
    auto triangle=genuineSource(source,0,2,2);
    Ae3OriginalActorTargetResultV1 first{},seeded{},second{};
    assert(ae3_original_actor_target_57c8ec(a,&triangle,&first)==1);
    assert(ae3_original_actor_target_57c8ec(b,&triangle,&seeded)==1);
    assert(first.original_branch==2&&seeded.original_branch==2);
    assert(first.original_low64==seeded.original_low64);
    assert(ae3_original_actor_target_57c8ec(a,&triangle,&second)==1);
    assert(second.original_low64!=first.original_low64);

    // A different seeded owner with no previous random draws yields
    // the exact same initial source triangle result after pass-through.
    assert(ae3_original_actor_target_57c8ec(c,&pass,&target)==1);
    Ae3OriginalActorTargetResultV1 afterPass{};
    assert(ae3_original_actor_target_57c8ec(c,&triangle,&afterPass)==1);
    assert(afterPass.original_low64==first.original_low64);

    // Original inputs are validated before mutating controller/RNG state.
    triangle.original_point_count=0;
    target={UINT64_MAX,77};
    assert(ae3_original_actor_target_57c8ec(a,&triangle,&target)==0);
    assert(target.original_branch==77&&target.original_low64==UINT64_MAX);
    triangle.original_point_count=3;
    triangle.original_record70=nullptr;
    assert(ae3_original_actor_target_57c8ec(a,&triangle,&target)==0);
    triangle.original_record70=source.data();
    triangle.original_controller20.x=std::numeric_limits<float>::quiet_NaN();
    assert(ae3_original_actor_target_57c8ec(a,&triangle,&target)==0);

    assert(ae3_original_manager_init_destroy(a)==1);
    assert(ae3_original_manager_init_destroy(b)==1);
    assert(ae3_original_manager_init_destroy(c)==1);
    std::cout<<"ORIGINAL_TARGET_57C8EC_NATIVE_FAST_AND_RANDOM_PASSED\n";
    return 0;
}
