#include "ae3/original_nav_actor_binding.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace ae3;
static void ck(bool v,const char* why){if(!v)throw std::runtime_error(why);}
static void w16(std::vector<std::uint8_t>& b,std::size_t at,std::uint16_t n){
    b.at(at)=static_cast<std::uint8_t>(n);
    b.at(at+1)=static_cast<std::uint8_t>(n>>8);
}
static void w32(std::vector<std::uint8_t>& b,std::size_t at,std::uint32_t n){
    for(unsigned j=0;j<4;++j)b.at(at+j)=static_cast<std::uint8_t>(n>>(8*j));
}
static void w32(std::array<std::byte,0x64>& b,std::size_t at,std::uint32_t n){
    for(unsigned j=0;j<4;++j)b.at(at+j]=std::byte((n>>(8*j))&255);
}
static OriginalNavSource sourceGraph() {
    OriginalNavSource input{};
    input.originalFilename="nav_monkey.nav";
    input.originalSlot=0;
    auto& b=input.originalNavBytes;
    b.resize(0x260);
    w32(b,0x24,4);w32(b,0x38,0x60);w32(b,0x3c,0x80);
    w32(b,0x40,0x120);w32(b,0x44,0x180);
    w32(b,0x48,0x1a0);w32(b,0x4c,0x1c0);
    for(unsigned i=0;i<4;++i){
        const auto at=0x80+0x1e*i;
        w16(b,at,0xffff);w16(b,at+0x16,1);w16(b,at+0x18,i);
        w32(b,0x120+16*i,std::bit_cast<std::uint32_t>(float(i)));
    }
    w16(b,0x80+2,2);
    w16(b,0x80+4,1);w16(b,0x80+6,2);
    w16(b,0x80+0x1e+2,1);w16(b,0x80+0x1e+4,3);
    w16(b,0x80+2*0x1e+2,1);w16(b,0x80+2*0x1e+4,3);
    w16(b,0x80+2*0x1e+0x0a,2);
    w16(b,0x80+2*0x1e+0x10,0);
    return input;
}
static std::array<std::byte,0x64> navrange(float halfWidth) {
    std::array<std::byte,0x64> h{};
    constexpr char name[]="navrange";
    for(unsigned i=0;i<8;++i)h[i]=std::byte(static_cast<unsigned char>(name[i]));
    w32(h,0x3c,std::bit_cast<std::uint32_t>(1.f));
    w32(h,0x40,std::bit_cast<std::uint32_t>(halfWidth));
    w32(h,0x44,std::bit_cast<std::uint32_t>(3.f));
    w32(h,0x48,std::bit_cast<std::uint32_t>(4.f));
    w32(h,0x54,1);w32(h,0x5c,0x650000);
    return h;
}
struct SourceCallbacks final : OriginalActorRouteFollowerCalls_57c7cc {
    const OriginalSpatialRecord70* expectedActor=nullptr;
    const OriginalSpatialRecord70* projection=nullptr;
    const OriginalSpatialRecord70* lastApplied=nullptr;
    unsigned projected=0,applied=0;
    const OriginalSpatialRecord70* call_original_4c1b3c_project(
        const OriginalSpatialRecord70* from) override {
        ck(from==expectedActor,"Original actor stage polygon pointer changed");
        ++projected;
        return projection;
    }
    void call_original_57c8ec_update_target(
        const OriginalSpatialRecord70* next) override {
        lastApplied=next;
        ++applied;
    }
};
int main() {
    OriginalNavStageOwners_21ed30 stage;
    stage.originalInitialize_21ed30();
    const auto source=sourceGraph();
    stage.originalAttach_4f0a5c(
        source,{"nav_monkey.nav",0x510000,0x620000});
    const auto& records=stage.originalAt(0).decodedGraph->records;
    OriginalNavActorRouteBinding actor(stage);
    OriginalNavDispatchRequest query{};
    query.sourceSeedIndex=0;
    query.routeMode=0;
    query.maximumDistance=100;
    query.originalReference={0,0,0,1};
    const auto plain=originalNavStageDispatch_2207d0(stage,0,query);
    ck(plain.originalForwardSourcePolygons==
       std::vector<std::uint32_t>({0,1,3}),"Original source least-cost predecessor incorrect");
    // Source-derived route snapshots are host-owned and copyable.
    // A deliberately altered path must NOT be able to make original
    // 0x221104's no-match slot-0 result look like a genuine edge.
    auto corruptPath=plain;
    corruptPath.originalForwardSourcePolygons={0,3};
    bool invalidSourceRoute=false;
    try{actor.originalUseStageRoute_2207d0(corruptPath);}
    catch(const std::invalid_argument&){invalidSourceRoute=true;}
    ck(invalidSourceRoute&&!actor.originalRouteCurrent(),
       "Original actor accepted a disconnected or altered path");
    const auto directed=plain.originalForwardSourcePolygons;
    ck(directed==std::vector<std::uint32_t>({0,1,3}),
       "Original recovered route source ordering changed");
    actor.originalUseStageRoute_2207d0(plain);
    ck(actor.originalRouteCurrent()&&actor.originalPathLength()==3 &&
       actor.originalHasMultiplePolygonsAtController1c(),
       "Original 0x57BA98 count/actor path binding lost");
    auto zero=actor.originalCurrentEdge_57bd88(0);
    ck(zero.sourcePolygonIndex==0&&zero.destinationPolygonIndex==1&&
       zero.originalActionKind==0,"Original kind-0 source edge not bound");
    ck(!actor.originalInitialType2Target_57bdb8(),
       "Original type-0 first edge falsely selected class-2 action");
    SourceCallbacks calls{};
    calls.expectedActor=&records[0];
    auto advanced=actor.originalAdvance_57c7cc(0,calls);
    ck(advanced.sourceTargetsApplied==2&&advanced.sourceProjectionsPerformed==1 &&
       actor.originalCurrentTargetAt50()==&records[1]&&
       calls.lastApplied==&records[3],
       "Original follower target update must precede projection test");
    calls.projection=&records[1];
    const auto applied=calls.applied;
    advanced=actor.originalAdvance_57c7cc(0,calls);
    ck(advanced.returnedEarlyBecauseCurrentTargetMatched &&
       advanced.sourceTargetsApplied==0 && calls.applied==applied,
       "Original actor projection match must skip target update");

    query.sourceSeedIndex=2;
    const auto kind2=originalNavStageDispatch_2207d0(stage,0,query);
    actor.originalUseStageRoute_2207d0(kind2);
    ck(actor.originalCurrentTargetAt50()==nullptr,
       "Replacing original actor route kept previous stage polygon target");
    const auto edge=actor.originalCurrentEdge_57bd88(0);
    ck(edge.originalKindTwoBranch && edge.sourcePolygonIndex==2 &&
       edge.destinationPolygonIndex==3,"Original class-2 edge decision absent");
    const auto target=actor.originalInitialType2Target_57bdb8();
    ck(target&&target->targetPolygonAt50==&records[3],
       "Original 0x57BDB8 target polygon identity lost");
    ck(target->rawOriginalVertices[0]==originalSpatialVector_21b458(records[3],0),
       "Original type-2 destination vector zero mismatched");
    calls.expectedActor=&records[2];
    const auto before=calls.applied;
    advanced=actor.originalAdvance_57c7cc(2,calls);
    ck(actor.originalCurrentTargetAt50()==&records[3]&&
       advanced.sourceTargetsApplied==1&&calls.applied==before+1,
       "Original two-polygon class-2 actor follower did not advance");

    // A target-change request uses the original dispatcher and swaps the
    // actor's active route only after that original search succeeds.
    OriginalNavDispatchRequest requery=query;
    requery.sourceSeedIndex=0;
    requery.routeMode=1;
    ck(actor.originalRetargetStage_2207d0(0,requery) &&
       actor.originalPathLength()==1 &&
       !actor.originalHasMultiplePolygonsAtController1c() &&
       actor.originalCurrentTargetAt50()==nullptr,
       "Original nearest-route target change did not replace actor state");
    requery.routeMode=0;
    ck(actor.originalRetargetStage_2207d0(0,requery) &&
       actor.originalPathLength()==3,
       "Original farthest destination was not regenerated");
    requery.sourceSeedIndex=999;
    bool failedRetarget=false;
    try{(void)actor.originalRetargetStage_2207d0(0,requery);}
    catch(const std::out_of_range&){failedRetarget=true;}
    ck(failedRetarget&&actor.originalRouteCurrent()&&actor.originalPathLength()==3,
       "Failed original route request overwrote previous valid actor route");
    requery.sourceSeedIndex=0;
    requery.routeMode=2;
    requery.explicitDestinationSearchIndex=-1;
    ck(!actor.originalRetargetStage_2207d0(0,requery) &&
       !actor.originalRouteCurrent() && actor.originalPathLength()==0,
       "Original explicit destination -1 must clear actor route");
    requery.routeMode=0;
    ck(actor.originalRetargetStage_2207d0(0,requery),
       "Original actor failed to regenerate route after no-route response");
    auto hdr=navrange(2.f);
    std::array<std::byte,0x20> groupKeyBytes{};
    groupKeyBytes[0]=std::byte{0x35};
    std::array<OriginalNavGroupAllocation,1> allocations{{
        {0x650000,groupKeyBytes}
    }};
    OriginalNavGroupSource navrangeResource{hdr,allocations};
    OriginalNavStageGroupSession group(stage,0);
    ck(group.originalRegisterRange_4e4060(navrangeResource)
       .sourcePolygonsInserted==3,"Source navrange admission failed");
    const auto key=originalNavGroupKeys_4e4060(navrangeResource).at(0);
    query.sourceSeedIndex=0;
    const auto grouped=group.originalBoundRouteByKey_2207d0(key,query);
    ck(grouped.sourceRoute.originalForwardSourcePolygons==
       std::vector<std::uint32_t>({0,2}),"Original group filter route changed");
    actor.originalUseGroupRoute_2207d0(group,grouped);
    ck(actor.originalRouteCurrent()&&actor.originalPathLength()==2 &&
       !actor.originalInitialType2Target_57bdb8(),
       "Original range-based actor route binding failed");
    ck(actor.originalRetargetGroup_2207d0(group,key,query) &&
       actor.originalPathLength()==2,
       "Original keyed navrange retarget failed");
    OriginalNavGroupKey20 badKey{};
    badKey.originalBytes[0]=std::byte{0xff};
    failedRetarget=false;
    try{(void)actor.originalRetargetGroup_2207d0(group,badKey,query);}
    catch(const std::out_of_range&){failedRetarget=true;}
    ck(failedRetarget&&actor.originalRouteCurrent()&&actor.originalPathLength()==2,
       "Unknown original group key discarded a valid actor route");
    auto wider=navrange(4.f);
    const OriginalNavGroupSource grown{wider,allocations};
    ck(group.originalRegisterRange_4e4060(grown).sourcePolygonsInserted==1,
       "Original navrange polygon registration did not grow");
    ck(!actor.originalRouteCurrent()&&actor.originalCurrentTargetAt50()==nullptr,
       "Actor retained route after group membership changed");
    bool threw=false;
    try{(void)actor.originalAdvance_57c7cc(0,calls);}
    catch(const std::logic_error&){threw=true;}
    ck(threw,"Actor advanced invalid navrange route");
    ck(actor.originalDropInvalidRoute()&&!actor.originalDropInvalidRoute() &&
       actor.originalPathLength()==0,"Actor route invalidation was not idempotent");

    // The group registry may be destroyed before its borrowed actor.
    // Weak lifetime guard prevents dereference of a dead group owner.
    auto temporary=std::make_unique<OriginalNavStageGroupSession>(stage,0);
    ck(temporary->originalRegisterRange_4e4060(navrangeResource)
       .sourcePolygonsInserted==3,"Secondary navrange session registration failed");
    actor.originalUseGroupRoute_2207d0(
        *temporary,temporary->originalBoundRouteByKey_2207d0(key,query));
    temporary.reset();
    ck(!actor.originalRouteCurrent()&&actor.originalDropInvalidRoute(),
       "Destroyed navrange group still considered live by actor");

    // Original routeMode>=2 with $8=-1 is an explicit NO ROUTE,
    // not permission to invent a path from actor+0x30.
    query.routeMode=2;query.explicitDestinationSearchIndex=-1;
    const auto none=originalNavStageDispatch_2207d0(stage,0,query);
    ck(none.originalForwardSourcePolygons.empty(),
       "Original explicit search-index -1 did not return no route");
    actor.originalUseStageRoute_2207d0(plain);
    actor.originalUseStageRoute_2207d0(none);
    ck(!actor.originalRouteCurrent()&&actor.originalPathLength()==0,
       "Original no-route path must clear actor binding");
    actor.originalUseGroupRoute_2207d0(
        group,group.originalBoundRouteByKey_2207d0(key,query));
    ck(!actor.originalRouteCurrent(),
       "Original group with explicit -1 must also unbind actor route");
    query.routeMode=0;
    // Malformed original navrange input must not leave existing actor
    // bound to a possibly partially-mutated source registry.
    actor.originalUseGroupRoute_2207d0(
        group,group.originalBoundRouteByKey_2207d0(key,query));
    ck(actor.originalRouteCurrent(),"Original grouped route failed to rebind");
    auto corrupt=navrange(2.f);
    w32(corrupt,0x5c,0x660000);
    bool badSource=false;
    try {
        (void)group.originalRegisterRange_4e4060({corrupt,allocations});
    } catch(const std::out_of_range&) {badSource=true;}
    ck(badSource&&!actor.originalRouteCurrent(),
       "Failed navrange registration did not conservatively invalidate routes");
    ck(group.originalRegistrationNeedsReset(),
       "Malformed navrange should require explicit clear");
    bool blockedRoute=false;
    try{(void)group.originalBoundRouteByKey_2207d0(key,query);}
    catch(const std::logic_error&){blockedRoute=true;}
    ck(blockedRoute,"Original actor could reacquire a partially registered group");
    group.originalClear_4e3fec();
    ck(!group.originalRegistrationNeedsReset() &&
       group.originalRegisterRange_4e4060(navrangeResource).sourcePolygonsInserted==3,
       "Explicit navrange group reset failed");
    actor.originalUseStageRoute_2207d0(plain);
    stage.originalReleaseSlot_21d7d8(0);
    ck(!actor.originalRouteCurrent()&&actor.originalCurrentTargetAt50()==nullptr,
       "Stage unload did not clear actor-visible polygon target");
    threw=false;
    try{(void)actor.originalCurrentEdge_57bd88(0);}
    catch(const std::logic_error&){threw=true;}
    ck(threw,"Stale original stage source edge was used after unload");
    stage.originalAttach_4f0a5c(
        source,{"nav_monkey.nav",0x510000,0x620000});
    ck(!actor.originalRouteCurrent(),"Reattached stage revived an old actor route");
    actor.originalClearForStageTransition();
    ck(actor.originalPathLength()==0&&!actor.originalHasMultiplePolygonsAtController1c(),
       "Original stage transition failed to clear controller route state");
}
