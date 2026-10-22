#include "ae3/original_nav_stage_route.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

using namespace ae3;
static void check(bool yes,const char* why){if(!yes)throw std::runtime_error(why);}
static void w16(std::vector<std::uint8_t>& d,std::size_t p,std::uint16_t n){
    d.at(p)=static_cast<std::uint8_t>(n);d.at(p+1)=static_cast<std::uint8_t>(n>>8);
}
static void w32(std::vector<std::uint8_t>& d,std::size_t p,std::uint32_t n){
    for(unsigned j=0;j<4;++j)d.at(p+j)=static_cast<std::uint8_t>(n>>(8*j));
}
static void vertex(std::vector<std::uint8_t>& d,unsigned id,float x){
    w32(d,0x120+16*id,std::bit_cast<std::uint32_t>(x));
}
static OriginalNavSource graph(const char* name,std::size_t slot) {
    OriginalNavSource s{};s.originalFilename=name;s.originalSlot=slot;
    auto& d=s.originalNavBytes;d.resize(0x280);
    w32(d,0x24,4);w32(d,0x38,0x60);w32(d,0x3c,0x80);
    w32(d,0x40,0x120);w32(d,0x44,0x1a0);
    w32(d,0x48,0x1c0);w32(d,0x4c,0x1e0);
    for(unsigned i=0;i<4;++i){
        const auto p=0x80+0x1e*i;
        w16(d,p,0xffff);w16(d,p+0x16,1);w16(d,p+0x18,i);
        vertex(d,i,static_cast<float>(i));
    }
    vertex(d,4,0.4f);
    // 0 -> 1 is an original type-1 child (length 0.4);
    // 0 -> 2 is a centroid edge. The other route to 3 is longer.
    w16(d,0x80+2,2);w16(d,0x80+4,1);w16(d,0x80+6,2);
    w16(d,0x80+0x0a,1);w16(d,0x80+0x0c,0);
    w16(d,0x80+0x10,0);
    w16(d,0x80+0x1e+2,1);w16(d,0x80+0x1e+4,3);
    // 2 -> 3 has a genuine original kind-2 source-table action.
    w16(d,0x80+2*0x1e+2,1);w16(d,0x80+2*0x1e+4,3);
    w16(d,0x80+2*0x1e+0x0a,2);w16(d,0x80+2*0x1e+0x10,0);
    // Original type-1 list: count=2, source-vector indices 0 and 4.
    w16(d,0x1e0,2);w16(d,0x1e2,0);w16(d,0x1e4,4);
    return s;
}
int main(){
    OriginalNavStageOwners_21ed30 owners;
    owners.originalInitialize_21ed30();
    const auto a=graph("nav_monkey.nav",0);
    const auto b=graph("nav_enemy.nav",1);
    owners.originalAttach_4f0a5c(a,{"nav_monkey.nav",0x510000,0x620000});
    owners.originalAttach_4f0a5c(b,{"nav_enemy.nav",0x520000,0x620028});
    check(owners.originalAt(0).decodedGraph->originalType1Children.size()==1,
          "Original type-1 child not owned by stage graph");
    check(owners.originalAt(0).decodedGraph->pendingOriginalActions.empty(),
          "Original type-1/type-2 action materialization incomplete");
    OriginalNavDispatchRequest q{};q.sourceSeedIndex=0;
    q.originalReference={0,0,0,0};q.maximumDistance=100;q.routeMode=0;
    const auto route=originalNavStageDispatch_2207d0(owners,0,q);
    check(route.originalAdmittedSourcePolygons==
          std::vector<std::uint32_t>({0,1,3,2}),"Original DFS admission order changed");
    check(route.originalForwardSourcePolygons==
          std::vector<std::uint32_t>({0,1,3}),"Original type-1 cost was not used to select predecessor");
    check(route.originalSelectedSearchIndex==2,"Original route endpoint must be DFS workspace index");
    check(originalNavStageRouteCurrent(owners,route),"Fresh original graph route invalid");
    check(originalNavStageRoutePolygon(owners,route,1)==
          &owners.originalAt(0).decodedGraph->records[1],"Stage pointer projection");
    const auto edge=originalNavStageRouteEdge_57bd88(owners,route,0);
    check(edge.sourceEdgeSlot==0&&edge.originalActionKind==1 &&
          !edge.originalKindTwoBranch,"Original type-1 edge identity lost");
    auto follow=originalNavStageFollowerState_57c7cc(owners,route,0);
    check(follow.sourceHasMultiplePolygonsAtController1c &&
          follow.originalRoutePolygonIndices.size()==3 &&
          follow.originalStagePolygonAtActor30==&owners.originalAt(0).decodedGraph->records[0],
          "Route did not connect to original actor follower");
    // Independent kind-2 decision, using source seed 2.
    q.sourceSeedIndex=2;
    const auto kind2=originalNavStageDispatch_2207d0(owners,0,q);
    check(kind2.originalForwardSourcePolygons==
          std::vector<std::uint32_t>({2,3}),"Original type-2 route changed");
    check(originalNavStageRouteEdge_57bd88(owners,kind2,0).originalKindTwoBranch,
          "Original kind-2 actor branch missing");
    // Source list fallback still uses original link ordering and
    // 0x4E3DEC strict nearest member rather than a substitute search.
    OriginalNavListOwner group;group.originalCreate_21f604();
    group.originalList()->originalInsertUnique_21f82c(
        &owners.originalAt(0).decodedGraph->records[3]);
    group.originalList()->originalInsertUnique_21f82c(
        &owners.originalAt(0).decodedGraph->records[2]);
    q.sourceSeedIndex=0;q.routeMode=0;
    const auto fallback=originalNavStageDispatch_2207d0(owners,0,q,&group);
    check(fallback.originalFallbackUsed &&
          fallback.originalFallbackSourcePolygon==2 &&
          fallback.originalForwardSourcePolygons==
          std::vector<std::uint32_t>({0,2}),"Original group fallback route mismatch");
    // Detect a group belonging to a different stage before passing its
    // member pointers through the original source membership tests.
    OriginalNavListOwner foreign;foreign.originalCreate_21f604();
    foreign.originalList()->originalInsertUnique_21f82c(
        &owners.originalAt(1).decodedGraph->records[2]);
    bool rejected=false;
    try{(void)originalNavStageDispatch_2207d0(owners,0,q,&foreign);}
    catch(const std::invalid_argument&){rejected=true;}
    check(rejected,"Cross-stage membership list accepted");
    const auto other=originalNavStageDispatch_2207d0(owners,1,q);
    owners.originalReleaseSlot_21d7d8(0);
    check(!originalNavStageRouteCurrent(owners,route) &&
          originalNavStageRouteCurrent(owners,other),
          "Releasing one original owner invalidated wrong stage route");
    rejected=false;
    try{(void)originalNavStageRoutePolygon(owners,route,0);}
    catch(const std::logic_error&){rejected=true;}
    check(rejected,"Released original graph returned a stale polygon pointer");
    owners.originalAttach_4f0a5c(a,{"nav_monkey.nav",0x510000,0x620000});
    check(!originalNavStageRouteCurrent(owners,route),"Graph replacement revived old route");
    owners.originalTeardown_21ed94();
    owners.originalInitialize_21ed30();
    owners.originalAttach_4f0a5c(a,{"nav_monkey.nav",0x510000,0x620000});
    check(!originalNavStageRouteCurrent(owners,route),"Stage reinitialization revived old route");
}
