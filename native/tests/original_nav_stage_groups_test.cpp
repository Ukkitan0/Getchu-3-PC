#include "ae3/original_nav_stage_groups.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

using namespace ae3;
static void ck(bool value,const char* text){if(!value)throw std::runtime_error(text);}
static void put16(std::vector<std::uint8_t>& b,std::size_t p,std::uint16_t v){
    b.at(p)=static_cast<std::uint8_t>(v);
    b.at(p+1)=static_cast<std::uint8_t>(v>>8);
}
static void put32(std::vector<std::uint8_t>& b,std::size_t p,std::uint32_t v){
    for(int k=0;k<4;++k)b.at(p+k)=static_cast<std::uint8_t>(v>>(8*k));
}
static void set32(std::span<std::byte> b,std::size_t p,std::uint32_t v) {
    for(int k=0;k<4;++k)b[p+k]=std::byte((v>>(8*k))&255);
}
static OriginalNavSource makeNav(const char* name,std::size_t slot) {
    OriginalNavSource g{};g.originalFilename=name;g.originalSlot=slot;
    auto& n=g.originalNavBytes;n.resize(0x230);
    put32(n,0x24,4);put32(n,0x38,0x60);put32(n,0x3c,0x80);
    put32(n,0x40,0x120);put32(n,0x44,0x180);
    put32(n,0x48,0x1a0);put32(n,0x4c,0x1c0);
    for(unsigned i=0;i<4;++i){
        const auto p=0x80+0x1e*i;
        put16(n,p,0xffff);put16(n,p+0x16,1);
        put16(n,p+0x18,static_cast<std::uint16_t>(i));
        put32(n,0x120+16*i,std::bit_cast<std::uint32_t>(float(i)));
    }
    put16(n,0x80+2,2);put16(n,0x80+4,1);put16(n,0x80+6,2);
    put16(n,0x80+0x1e+2,1);put16(n,0x80+0x1e+4,3);
    put16(n,0x80+2*0x1e+2,1);put16(n,0x80+2*0x1e+4,3);
    put16(n,0x80+2*0x1e+0x0a,2);
    put16(n,0x80+2*0x1e+0x10,0);
    return g;
}
static std::array<std::byte,0x64> makeRangeHeader() {
    std::array<std::byte,0x64> h{};
    constexpr char name[]="navrange";
    for(unsigned i=0;i<8;++i)h[i]=std::byte(static_cast<unsigned char>(name[i]));
    set32(h,0x3c,std::bit_cast<std::uint32_t>(1.f));
    set32(h,0x40,std::bit_cast<std::uint32_t>(2.f));
    set32(h,0x44,std::bit_cast<std::uint32_t>(3.f));
    set32(h,0x48,std::bit_cast<std::uint32_t>(4.f));
    set32(h,0x54,2);set32(h,0x5c,0x650000);
    return h;
}
int main(){
    OriginalNavStageOwners_21ed30 owners;
    owners.originalInitialize_21ed30();
    const auto nav0=makeNav("nav_monkey.nav",0);
    const auto nav1=makeNav("nav_enemy.nav",1);
    owners.originalAttach_4f0a5c(nav0,{"nav_monkey.nav",0x510000,0x620000});
    owners.originalAttach_4f0a5c(nav1,{"nav_enemy.nav",0x520000,0x620028});
    OriginalNavStageGroupSession first(owners,0);
    OriginalNavStageGroupSession other(owners,1);
    std::array<std::byte,0x40> groupKeys{};
    groupKeys[0]=std::byte{0x10};groupKeys[0x20]=std::byte{0x20};
    std::array<OriginalNavGroupAllocation,1> alloc{{{0x650000,groupKeys}}};
    auto header=makeRangeHeader();
    const OriginalNavGroupSource resource{header,alloc};
    const auto created=first.originalRegisterRange_4e4060(resource);
    ck(created.sourceGroups==2&&created.ownersCreated==2&&
       created.sourcePolygonsInserted==6,"Original group registration missed eligible polygons");
    ck(first.originalRegisteredGroupCount()==2&&
       other.originalRegisteredGroupCount()==0,"Original stage group owner isolation");
    const auto keys=originalNavGroupKeys_4e4060(resource);
    ck(keys.size()==2,"Original header+0x54 group count");
    for(const auto& key:keys){
        const auto* list=first.originalGroups().originalFind_36d6f4(key);
        ck(list && list->originalList()->originalOrderedValues()==
           std::vector<const OriginalSpatialRecord70*>({
               &owners.originalAt(0).decodedGraph->records[2],
               &owners.originalAt(0).decodedGraph->records[1],
               &owners.originalAt(0).decodedGraph->records[0]}),
           "Original head insertion order or 0x21EF3C eligibility changed");
    }
    OriginalNavDispatchRequest q{};
    q.sourceSeedIndex=0;q.maximumDistance=100;q.routeMode=0;
    q.originalReference={0,0,0,1};
    auto filtered=first.originalRouteByKey_2207d0(keys[0],q);
    ck(filtered.originalAdmittedSourcePolygons==
       std::vector<std::uint32_t>({0,1,2}),"Original range filter did not prune neighbor 3");
    ck(filtered.originalForwardSourcePolygons==
       std::vector<std::uint32_t>({0,2}),"Original grouped farthest route wrong");
    auto unfiltered=originalNavStageDispatch_2207d0(owners,0,q);
    ck(unfiltered.originalForwardSourcePolygons==
       std::vector<std::uint32_t>({0,1,3}),"Ungrouped retail branch changed");
    ck(first.originalRegisterRange_4e4060(resource).sourcePolygonsInserted==0,
       "Original group duplicate insertion must be ignored");
    q.sourceSeedIndex=3;q.originalReference={3,0,0,1};
    const auto fallback=first.originalRouteByKey_2207d0(keys[0],q);
    ck(fallback.originalFallbackUsed &&
       fallback.originalFallbackSourcePolygon==2 &&
       fallback.originalForwardSourcePolygons==
       std::vector<std::uint32_t>({3}),"Original out-of-group fallback incorrect");
    OriginalNavGroupKey20 missing{};
    missing.originalBytes[0]=std::byte{0xff};
    bool caught=false;
    try{(void)first.originalRouteByKey_2207d0(missing,q);}
    catch(const std::out_of_range&){caught=true;}
    ck(caught,"Unregistered retail group key silently accepted");
    // Original type-2 edge 2->3 must produce three genuine source
    // destination vectors, not a fake transform.
    const auto type2=originalNavStageDispatch_2207d0(owners,0,q);
    ck(type2.originalForwardSourcePolygons.size()==1,
       "Isolated source polygon route length");
    q.sourceSeedIndex=2;q.originalReference={0,0,0,1};
    const auto kind2=originalNavStageDispatch_2207d0(owners,0,q);
    auto type2Target=originalNavStageType2Target_57bdb8(owners,kind2);
    ck(type2Target && type2Target->targetPolygonAt50==
       &owners.originalAt(0).decodedGraph->records[3],
       "Original 0x57BDB8 route[1] pointer mismatch");
    const auto targetBytes=originalSpatialVector_21b458(
        owners.originalAt(0).decodedGraph->records[3],0);
    ck(type2Target->rawOriginalVertices[0]==targetBytes,
       "Original type-2 source target vector not preserved");
    ck(!originalNavStageType2Target_57bdb8(owners,unfiltered),
       "Original type-0 edge invoked type-2 target path");
    // Releasing one original slot invalidates only its stage group
    // session and routes. The other slot is independent.
    owners.originalReleaseSlot_21d7d8(0);
    ck(!first.originalCurrentSourceGraph()&&other.originalCurrentSourceGraph(),
       "Original group invalidation affected wrong stage slot");
    caught=false;
    try{(void)first.originalRouteByKey_2207d0(keys[0],q);}
    catch(const std::logic_error&){caught=true;}
    ck(caught,"Freed stage registry still routed through stale polygon pointers");
    owners.originalAttach_4f0a5c(nav0,{"nav_monkey.nav",0x510000,0x620000});
    ck(!first.originalCurrentSourceGraph(),"Reattached stage reused old group generation");
    first.originalRebindSourceGraph();
    ck(first.originalCurrentSourceGraph()&&first.originalRegisteredGroupCount()==0,
       "Stage group rebind should clear old polygon list");
    caught=false;
    try{(void)first.originalRouteByKey_2207d0(keys[0],q);}
    catch(const std::out_of_range&){caught=true;}
    ck(caught,"Stale navrange key silently recreated after graph change");
    ck(first.originalRegisterRange_4e4060(resource).sourcePolygonsInserted==6,
       "Navrange group rebuilding on reattached graph failed");
    owners.originalTeardown_21ed94();
    ck(!first.originalCurrentSourceGraph()&&!other.originalCurrentSourceGraph(),
       "Stage teardown retained source group generations");
}
