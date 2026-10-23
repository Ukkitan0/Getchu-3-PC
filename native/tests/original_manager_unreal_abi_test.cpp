#include "ae3/ae3_unreal_lua_bridge.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace {
void w16(std::vector<uint8_t>& d, std::size_t at, uint16_t v) {
    d.at(at)=uint8_t(v);d.at(at+1)=uint8_t(v>>8);
}
void w32(std::vector<uint8_t>& d, std::size_t at, uint32_t v) {
    for(unsigned n=0;n<4;++n)d.at(at+n)=uint8_t(v>>(n*8));
}
struct RealManager {
    Ae3OriginalManagerInitSession* session = nullptr;
    int projections=0, continuations=0, reentrantDestroy=-1;
    bool allowProjection=true, allowContinuation=true;
};
int originalProjection(void* opaque,const void* polygon,
                       const Ae3OriginalVec4* start,
                       const Ae3OriginalVec4* end,
                       Ae3OriginalVec4* projected) {
    auto* manager=static_cast<RealManager*>(opaque);
    assert(manager && polygon && start && end && projected);
    assert(start->x==1 && start->y==2 && end->x==26 && end->y==52);
    ++manager->projections;
    if(!manager->allowProjection)return 0;
    *projected={30,40,50,60};
    return 1;
}
int originalRouteContinuation(void* opaque,const Ae3OriginalVec4* midpoint) {
    auto* manager=static_cast<RealManager*>(opaque);
    assert(manager && midpoint);
    assert(midpoint->x==15.5f && midpoint->y==21 &&
           midpoint->z==26.5f && midpoint->w==32);
    ++manager->continuations;
    manager->reentrantDestroy=ae3_original_manager_init_destroy(manager->session);
    assert(manager->reentrantDestroy==0); // No UAF during active callback.
    return manager->allowContinuation?1:0;
}
void verifyNearOwner() {
    auto* session=ae3_original_manager_init_create(5489);
    assert(session);
    Ae3OriginalManagerInitStateV1 state{};
    state.source_method08=0x1234;
    state.source_method0c=0x5678;
    state.source_alternative30=1;
    state.controller20={1,2,3,4};
    state.target40={7,8,9,10};
    state.target_polygon_index=UINT32_MAX;
    Ae3OriginalManagerInitSourceV1 source{};
    source.original_actor_polygon_index=UINT32_MAX;
    source.original_owner_position40={1.5f,2,3,4};
    assert(ae3_original_manager_init_step(session,&source,&state,nullptr)==1);
    uint32_t expectedX=0;
    const float delta=-0.5f;
    std::memcpy(&expectedX,&delta,sizeof delta);
    assert(state.source_method08==expectedX &&
           state.source_method0c==0 && state.target40.x==7 &&
           state.target_polygon_index==UINT32_MAX);
    assert(ae3_original_manager_init_destroy(session)==1);
}
void verifyGeneralRouteAndRequiredOwners() {
    auto* session=ae3_original_manager_init_create(5489);
    assert(session);
    std::array<uint8_t,0x70> sourcePolygon{};
    Ae3OriginalManagerInitSourceV1 original{};
    original.original_records70=sourcePolygon.data();
    original.original_record_count=1;
    original.original_actor_polygon_index=0;
    original.original_owner_direction50={1,2,3,4};
    Ae3OriginalManagerInitStateV1 state{};
    state.source_method08=0x1234;
    state.source_method0c=0x5678;
    state.controller20={1,2,3,4};
    state.target_polygon_index=UINT32_MAX;
    RealManager manager{};manager.session=session;
    Ae3OriginalManagerInitCallsV1 hooks{
        &manager,originalProjection,originalRouteContinuation
    };
    assert(ae3_original_manager_init_step(session,&original,&state,nullptr)==0);
    assert(std::strstr(ae3_original_manager_init_last_error(session),
                       "0x21B854")!=nullptr);
    assert(state.source_method08==0x1234 && manager.projections==0);
    manager.allowProjection=false;
    assert(ae3_original_manager_init_step(session,&original,&state,&hooks)==0);
    assert(state.source_method08==0x1234 && manager.continuations==0);
    manager.allowProjection=true;
    manager.allowContinuation=false;
    assert(ae3_original_manager_init_step(session,&original,&state,&hooks)==0);
    assert(state.source_method08==0x1234);
    manager.allowContinuation=true;
    assert(ae3_original_manager_init_step(session,&original,&state,&hooks)==3);
    assert(state.source_method08==0x57bf5c && state.source_method0c==0 &&
           state.midpoint60.x==15.5f && manager.projections==3 &&
           manager.continuations==2 && manager.reentrantDestroy==0);
    assert(ae3_original_manager_init_destroy(session)==1);
}
struct RecoveredRouteCalls {
    int targetUpdates=0;
    int projected=0;
    bool acceptTarget=true;
    uint32_t expectedTarget=0;
    uint32_t* mutableRoute=nullptr;
    uint8_t* mutableNavigation=nullptr;
    Ae3OriginalManagerInitCallsV2* mutableHooks=nullptr;
};
int recoveredProject21b854(void* opaque, const void* polygon,
                           const Ae3OriginalVec4* from,
                           const Ae3OriginalVec4* to,
                           Ae3OriginalVec4* projected) {
    assert(polygon && from && to && projected);
    assert(from->x==1 && from->y==2 && to->x==26 && to->y==52);
    if (opaque) {
        auto& owner=*static_cast<RecoveredRouteCalls*>(opaque);
        if (owner.mutableRoute) {
            assert(owner.mutableNavigation);
            owner.mutableRoute[1]=99;
            owner.mutableNavigation[0x80+4]=99;
        }
        if (owner.mutableHooks)
            owner.mutableHooks->original_apply_target_57c8ec=nullptr;
    }
    *projected={30,40,50,60};
    return 1;
}
int recoveredProjection4c1b3c(void* context,uint32_t actor,
                               uint32_t* out) {
    auto& callbacks=*static_cast<RecoveredRouteCalls*>(context);
    assert(actor==0 && out);
    callbacks.projected++;
    *out=UINT32_MAX;
    return 1;
}
int recoveredApply57c8ec(void* context,uint32_t target) {
    auto& callbacks=*static_cast<RecoveredRouteCalls*>(context);
    assert(target==callbacks.expectedTarget);
    callbacks.targetUpdates++;
    return callbacks.acceptTarget?1:0;
}
void verifyRecoveredRouteFollowerRunsInsideOriginalInitializer() {
    auto* session=ae3_original_manager_init_create(5489);
    assert(session);
    std::array<uint8_t,0x70> realPolygon{};
    Ae3OriginalManagerInitSourceV1 source{};
    source.original_records70=realPolygon.data();
    source.original_record_count=1;
    source.original_actor_polygon_index=0;
    source.original_owner_direction50={1,2,3,4};
    Ae3OriginalManagerInitStateV1 state{};
    state.source_method08=0x1234;
    state.source_method0c=0x5678;
    state.target_polygon_index=UINT32_MAX;
    state.controller20={1,2,3,4};
    RecoveredRouteCalls actualOwner{};
    Ae3OriginalManagerInitCallsV2 realHooks{
        &actualOwner,
        recoveredProject21b854,
        recoveredProjection4c1b3c,
        recoveredApply57c8ec
    };
    actualOwner.acceptTarget=false;
    // The 0x57BCB8 midpoint is calculated, then genuine 0x57C7CC
    // branches to the original one-polygon target update 0x57C8EC.
    // Owner rejection stops native init without publishing controller state.
    assert(ae3_original_manager_init_step_v2(
        session,&source,&state,&realHooks)==0);
    assert(state.source_method08==0x1234 &&
           state.target_polygon_index==UINT32_MAX &&
           actualOwner.targetUpdates==1);
    actualOwner.acceptTarget=true;
    assert(ae3_original_manager_init_step_v2(
        session,&source,&state,&realHooks)==3);
    assert(state.source_method08==0x57bf5c &&
           state.target_polygon_index==0 &&
           state.midpoint60.x==15.5f &&
           actualOwner.targetUpdates==2 &&
           actualOwner.projected==0);
    // The legacy opaque callback is no longer needed for the new path.
    assert(ae3_original_manager_init_destroy(session)==1);
}

void verifyRecoveredRouteFollowerMultiPolygonBranchInInitializer() {
    auto* session=ae3_original_manager_init_create(5489);
    assert(session);
    std::array<uint8_t,0x70*2> polygons{};
    std::vector<uint8_t> navigation(0x80+2*0x1e);
    navigation[0x24]=2; // exactly two original stage polygon descriptors
    navigation[0x3c]=0x80; // original descriptor table offset
    navigation[0x80+2]=1;  // source polygon 0: one neighbor
    navigation[0x80+4]=1;  // neighbor polygon 1
    navigation[0x80+0x0a]=0; // original non-kind-2 action
    std::array<uint32_t,2> route{0,1};
    Ae3OriginalManagerInitSourceV1 source{};
    source.original_records70=polygons.data();
    source.original_record_count=2;
    source.original_actor_polygon_index=0;
    source.original_navigation_bytes=navigation.data();
    source.original_navigation_size=navigation.size();
    source.original_route_indices=route.data();
    source.original_route_count=route.size();
    source.original_owner_direction50={1,2,3,4};
    Ae3OriginalManagerInitStateV1 state{};
    state.source_flag1c=1;
    state.source_method08=0x1234;
    state.source_method0c=0x5678;
    state.target_polygon_index=UINT32_MAX;
    state.controller20={1,2,3,4};
    RecoveredRouteCalls owner{};
    owner.expectedTarget=1;
    Ae3OriginalManagerInitCallsV2 hooks{
        &owner,recoveredProject21b854,
        recoveredProjection4c1b3c,recoveredApply57c8ec
    };
    // Mutate the borrower's stage bytes inside 0x21B854. Genuine
    // 0x57C7CC must still consume the pre-callback route snapshot,
    // not the now-corrupted 99 index.
    owner.mutableRoute=route.data();
    owner.mutableNavigation=navigation.data();
    owner.mutableHooks=&hooks;
    // Control flow: 0x57BCB8 checks action kind 0, performs original
    // projection and calls source 0x57C7CC. No invented route selector:
    // original 0x57C7CC adopts source route[1] then invokes 0x57C8EC.
    assert(ae3_original_manager_init_step_v2(
        session,&source,&state,&hooks)==3);
    assert(state.target_polygon_index==1 &&
           state.source_method08==0x57bf5c &&
           owner.targetUpdates==1 && owner.projected==0 &&
           route[1]==99 && navigation[0x80+4]==99 &&
           hooks.original_apply_target_57c8ec==nullptr);
    owner.mutableRoute=nullptr;
    owner.mutableNavigation=nullptr;
    owner.mutableHooks=nullptr;
    hooks.original_apply_target_57c8ec=&recoveredApply57c8ec;
    // Missing original route[1] must abort without publishing any state.
    route[1]=99;
    const auto before=state;
    assert(ae3_original_manager_init_step_v2(
        session,&source,&state,&hooks)==0);
    assert(state.source_method08==before.source_method08 &&
           state.target_polygon_index==before.target_polygon_index);
    assert(ae3_original_manager_init_destroy(session)==1);
}

void verifyOriginalKindTwoTarget() {
    auto* session=ae3_original_manager_init_create(5489);
    assert(session);
    std::array<uint8_t,0x70*2> records{};
    const std::array<Ae3OriginalVec4,3> vertices{{
        {1,2,3,4},{11,12,13,14},{21,22,23,24}
    }};
    std::memcpy(records.data()+0x70+0x40,vertices.data(),
                vertices.size()*sizeof(vertices[0]));
    std::vector<uint8_t> navigation(0x80+2*0x1e);
    w32(navigation,0x24,2);w32(navigation,0x3c,0x80);
    w16(navigation,0x80+2,1);w16(navigation,0x80+4,1);
    w16(navigation,0x80+0x0a,2);
    std::array<uint32_t,2> route{0,1};
    Ae3OriginalManagerInitSourceV1 original{};
    original.original_records70=records.data();
    original.original_record_count=2;
    original.original_actor_polygon_index=0;
    original.original_navigation_bytes=navigation.data();
    original.original_navigation_size=navigation.size();
    original.original_route_indices=route.data();
    original.original_route_count=route.size();
    Ae3OriginalManagerInitStateV1 state{};
    state.source_flag1c=1;
    state.target_polygon_index=UINT32_MAX;
    assert(ae3_original_manager_init_step(session,&original,&state,nullptr)==2);
    assert(state.target_polygon_index==1 && state.source_method08==0x57c554);
    assert(state.target40.x>=1 && state.target40.x<=21 &&
           state.target40.y>=2 && state.target40.y<=22);
    // Corrupt route input is rejected without publishing a fake target.
    route[1]=22;
    const auto previous=state;
    assert(ae3_original_manager_init_step(session,&original,&state,nullptr)==0);
    assert(state.target_polygon_index==previous.target_polygon_index &&
           state.source_method08==previous.source_method08);
    assert(std::strlen(ae3_original_manager_init_last_error(session))>0);
    assert(ae3_original_manager_init_destroy(session)==1);
}
void verifyUnsafeInputs() {
    auto* session=ae3_original_manager_init_create(5489);
    assert(session);
    Ae3OriginalManagerInitStateV1 state{};
    state.target_polygon_index=UINT32_MAX;
    Ae3OriginalManagerInitSourceV1 original{};
    original.original_record_count=1;
    original.original_actor_polygon_index=0;
    assert(ae3_original_manager_init_step(session,&original,&state,nullptr)==0);
    assert(std::strstr(ae3_original_manager_init_last_error(session),"spatial")!=nullptr);
    assert(ae3_original_manager_init_step(session,nullptr,&state,nullptr)==0);
    assert(ae3_original_manager_init_step(session,&original,nullptr,nullptr)==0);
    assert(ae3_original_manager_init_destroy(session)==1);
}
}
int main() {
    verifyNearOwner();
    verifyGeneralRouteAndRequiredOwners();
    verifyRecoveredRouteFollowerRunsInsideOriginalInitializer();
    verifyRecoveredRouteFollowerMultiPolygonBranchInInitializer();
    verifyOriginalKindTwoTarget();
    verifyUnsafeInputs();
    std::cout<<"ORIGINAL_MANAGER_57BCB8_HOST_ABI_PASSED\n";
    return 0;
}
