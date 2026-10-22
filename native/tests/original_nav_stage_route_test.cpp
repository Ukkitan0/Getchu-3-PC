#include "ae3/original_nav_stage_route.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <limits>
#include <span>
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
int main(int argc,char**argv){
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
    // Native safety only: reject a user-mutated route which points to
    // source polygon 3 without a genuine directed polygon 0->3 link.
    auto injected=route;
    injected.originalForwardSourcePolygons={0,3};
    bool disconnectedRejected=false;
    try{(void)originalNavStageRouteEdge_57bd88(owners,injected,0);}
    catch(const std::invalid_argument&){disconnectedRejected=true;}
    check(disconnectedRejected,"Foreign route adjacency was interpreted as source slot zero");
    disconnectedRejected=false;
    try{(void)originalNavStageFollowerState_57c7cc(owners,injected,0);}
    catch(const std::invalid_argument&){disconnectedRejected=true;}
    check(disconnectedRejected,"Original route follower accepted disconnected source path");
    disconnectedRejected=false;
    try{(void)originalNavStageRoutePolygon(owners,injected,1);}
    catch(const std::invalid_argument&){disconnectedRejected=true;}
    check(disconnectedRejected,"Original polygon accessor accepted a disconnected route");
    const auto edge=originalNavStageRouteEdge_57bd88(owners,route,0);
    check(edge.sourceEdgeSlot==0&&edge.originalActionKind==1 &&
          !edge.originalKindTwoBranch,"Original type-1 edge identity lost");
    auto follow=originalNavStageFollowerState_57c7cc(owners,route,0);
    check(follow.sourceHasMultiplePolygonsAtController1c &&
          follow.originalRoutePolygonIndices.size()==3 &&
          follow.originalStagePolygonAtActor30==&owners.originalAt(0).decodedGraph->records[0],
          "Route did not connect to original actor follower");
    // Explicit original 0x2207D0 $8 search index uses admitted DFS
    // order, independently of original route mode nearest/farthest.
    q.routeMode=2;q.explicitDestinationSearchIndex=2;
    const auto indexed=originalNavStageDispatch_2207d0(owners,0,q);
    check(indexed.originalForwardSourcePolygons==
          std::vector<std::uint32_t>({0,1,3}),"Original explicit source search index changed");
    q.explicitDestinationSearchIndex=-1;
    check(originalNavStageDispatch_2207d0(owners,0,q)
          .originalForwardSourcePolygons.empty(),"Unset original $8 should yield no route");
    q.routeMode=0;
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
    check(fallback.hostMembershipBound&&originalNavStageRouteCurrent(owners,fallback),
          "Direct linked-owner route does not retain source membership version");
    check(!group.originalList()->originalInsertUnique_21f82c(
        &owners.originalAt(0).decodedGraph->records[2]),"Duplicate original source member inserted");
    check(originalNavStageRouteCurrent(owners,fallback),
          "Duplicate source-list insertion spuriously invalidated original route");
    check(group.originalList()->originalInsertUnique_21f82c(
        &owners.originalAt(0).decodedGraph->records[1]),"New original source member was rejected");
    check(!originalNavStageRouteCurrent(owners,fallback) &&
          originalNavStageRouteCurrent(owners,route),
          "Direct membership list mutation did not invalidate only its own routes");
    bool staleMemberBlocked=false;
    try{(void)originalNavStageRoutePolygon(owners,fallback,0);}
    catch(const std::logic_error&){staleMemberBlocked=true;}
    check(staleMemberBlocked,"Stale original membership route still exposes source polygons");
    const auto renewed=originalNavStageDispatch_2207d0(owners,0,q,&group);
    check(originalNavStageRouteCurrent(owners,renewed),
          "Renewed source membership route failed after mutation");
    check(group.originalList()->originalErase_21f804(
        &owners.originalAt(0).decodedGraph->records[1])==1,
          "Original member erase did not run");
    check(!originalNavStageRouteCurrent(owners,renewed),
          "Member erase did not invalidate a previously live actor route");
    const auto toDispose=originalNavStageDispatch_2207d0(owners,0,q,&group);
    group.originalDispose_21f64c();
    check(!originalNavStageRouteCurrent(owners,toDispose),
          "Disposing original membership list revived a dangling route");
    struct RouteCalls final : OriginalActorRouteFollowerCalls_57c7cc {
        const OriginalSpatialRecord70* actor=nullptr;
        const OriginalSpatialRecord70* target=nullptr;
        unsigned projected=0,applied=0;
        const OriginalSpatialRecord70* projectedValue=nullptr;
        const OriginalSpatialRecord70* call_original_4c1b3c_project(
            const OriginalSpatialRecord70* incoming) override {
            check(incoming==actor,"Original source actor route projection identity");
            ++projected;
            return projectedValue; // injected VU0/physics test result, not gameplay
        }
        void call_original_57c8ec_update_target(
            const OriginalSpatialRecord70* incoming) override {
            ++applied;target=incoming;
        }
    } callbacks;
    const auto& sourceRecords=owners.originalAt(0).decodedGraph->records;
    callbacks.actor=&sourceRecords[0];
    const OriginalSpatialRecord70* current=nullptr;
    const auto moved=originalNavStageAdvanceRoute_57c7cc(
        owners,route,0,current,callbacks);
    check(current==&sourceRecords[1]&&callbacks.target==&sourceRecords[3] &&
          moved.sourceTargetsApplied==2 && moved.sourceProjectionsPerformed==1,
          "Original actor follower route advancement did not preserve call ordering");
    callbacks.projectedValue=&sourceRecords[1];
    const auto appliedBefore=callbacks.applied;
    const auto early=originalNavStageAdvanceRoute_57c7cc(
        owners,route,0,current,callbacks);
    check(early.returnedEarlyBecauseCurrentTargetMatched &&
          early.sourceTargetsApplied==0 && callbacks.applied==appliedBefore &&
          current==&sourceRecords[1],
          "Original actor active-target match must not select another route polygon");
    // Direct borrowed source filter must reject foreign pointers as
    // rigorously as a registered original navrange list owner.
    OriginalNavDispatchRequest foreignReq=q;
    foreignReq.sourceMembershipListExists=true;
    const auto& external=owners.originalAt(1).decodedGraph->records;
    const std::array<const OriginalSpatialRecord70*,1> foreignItems{{&external[0]}};
    foreignReq.orderedOriginalMembershipList=foreignItems;
    bool foreignRejected=false;
    try{(void)originalNavStageDispatch_2207d0(owners,0,foreignReq);}
    catch(const std::invalid_argument&){foreignRejected=true;}
    check(foreignRejected,"Direct original source filter accepted foreign stage pointers");
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
    OriginalNavStageOwners_21ed30 separateOwner;
    separateOwner.originalInitialize_21ed30();
    separateOwner.originalAttach_4f0a5c(a,{"nav_monkey.nav",0x510000,0x620000});
    check(!originalNavStageRouteCurrent(separateOwner,route),
          "Different original stage owner accepted matching slot/revision");
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
    // Optional owner-private original PCK regression for ANY supplied
    // stage, without hardcoded graph sizes or original asset output.
    // Examples: Getchu3OriginalNavStageRouteTests woods_c/navigation.pck
    //           Getchu3OriginalNavStageRouteTests woods_c/navigation.pck other_stage/navigation.pck
    for(int arg=1;arg<argc;++arg){
        std::ifstream file(argv[arg],std::ios::binary);
        check(bool(file),"Original navigation.pck test input unavailable");
        const std::vector<char> original((std::istreambuf_iterator<char>(file)),{});
        std::vector<std::byte> bytes;
        bytes.reserve(original.size());
        for(unsigned char c:original)bytes.push_back(std::byte(c));
        const auto sources=originalNavReadNativePck_4f0a5c(bytes);
        check(!sources.empty(),"No original navigation allocations found");
        OriginalNavStageOwners_21ed30 retail;
        retail.originalInitialize_21ed30();
        for(const auto& source:sources){
            const auto slot=source.originalSlot;
            const auto sourceVA=std::uint32_t(0x510000+slot*0x20000);
            const auto ownerVA=std::uint32_t(0x620000+slot*0x28);
            retail.originalAttach_4f0a5c(source,
                {source.originalFilename,sourceVA,ownerVA});
            const auto& g=*retail.originalAt(slot).decodedGraph;
            if(g.records.empty())continue;
            const auto count=g.polygonHeaders[0].sourcePointCountAt16;
            const auto points=originalSpatialVertices_21b468(g.records.front());
            const auto query=originalVU0Mean_21b468(
                std::span<const OriginalVU0Vec4>(points.data(),count));
            OriginalNavDispatchRequest request{};
            request.sourceSeedIndex=0;
            request.originalReference=query;
            request.maximumDistance=std::numeric_limits<float>::max();
            request.routeMode=0;
            const auto route=originalNavStageDispatch_2207d0(retail,slot,request);
            check(!route.originalAdmittedSourcePolygons.empty(),
                  "Original retail source seed was excluded");
            check(!route.originalForwardSourcePolygons.empty(),
                  "Original retail route endpoint unexpectedly missing");
            check(originalNavStageRoutePolygon(retail,route,0)==&g.records[0],
                  "Original retail route not rooted at source seed");
            for(std::size_t j=0;j+1<route.originalForwardSourcePolygons.size();++j){
                const auto link=originalNavStageRouteEdge_57bd88(retail,route,j);
                check(link.originalActionKind<=2,
                      "Unknown original retail type-0/1/2 action semantics");
            }
            retail.originalReleaseSlot_21d7d8(slot);
            check(!originalNavStageRouteCurrent(retail,route),
                  "Original retail stage transition did not invalidate route");
        }
    }
}
