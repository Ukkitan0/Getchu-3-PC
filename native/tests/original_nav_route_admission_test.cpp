#include "ae3/original_nav_route_admission.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void check(bool x,const char* error){if(!x)throw std::runtime_error(error);}
static void w16(std::vector<std::byte>& b,std::size_t off,std::uint16_t v){
    b.at(off)=std::byte(v&255u);b.at(off+1)=std::byte(v>>8);
}
static void w32(std::vector<std::byte>& b,std::size_t off,std::uint32_t v){
    for(unsigned i=0;i<4;i++)b.at(off+i)=std::byte((v>>(8*i))&255u);
}
static std::vector<std::byte> fixture(std::span<const float> locations){
    std::vector<std::byte> nav(0x80+0x1e*locations.size());
    w32(nav,0x24,static_cast<std::uint32_t>(locations.size()));
    w32(nav,0x3c,0x80);
    for(std::size_t i=0;i<locations.size();i++)
        w16(nav,0x80+0x1e*i+0x16,1);
    return nav;
}
static std::vector<OriginalSpatialRecord70> records(std::span<const float> x){
    std::vector<OriginalSpatialRecord70> result(x.size());
    for(std::size_t i=0;i<result.size();i++){
        const auto word=std::bit_cast<std::uint32_t>(x[i]);
        for(unsigned j=0;j<4;j++)
            result[i].originalFields[0x40+j]=std::byte((word>>(8*j))&255u);
    }
    return result;
}
int main(){
    const std::array<float,5> positions{0.f,1.f,2.f,3.f,20.f};
    auto nav=fixture(positions);auto rec=records(positions);
    auto edge=[&](std::size_t from,std::size_t slot,std::uint16_t target,std::uint16_t type){
        const auto at=0x80+0x1e*from;
        w16(nav,at+2,static_cast<std::uint16_t>(slot+1));
        w16(nav,at+4+slot*2,target);
        w16(nav,at+0xa+slot*2,type);
    };
    edge(0,0,1,0);edge(0,1,2,2);edge(1,0,3,0);
    edge(3,0,0,0);edge(2,0,4,0);
    OriginalNavEpochState epoch;
    OriginalNavAdmissionRequest q{};q.maximumDistance=4.f;
    const auto initial=originalNavAdmit_220aa4(rec,nav,q,epoch);
    check(initial.sourceRecordIndicesInDepthFirstOrder==
        std::vector<std::uint32_t>({0,1,3,2}),"Original recursive admission order");
    check(initial.candidateRecordsVisited==5,"Original candidate visit count");
    auto work=originalNavBuildAdmittedWorkspace_220c00(rec,nav,initial);
    check(work.nodes.size()==4,"Original admission count");
    check(work.nodes[0].polygon04==&rec[0]&&work.nodes[2].polygon04==&rec[3],
          "Original DFS source pointer order");
    check(work.nodes[0].destinationIndices0c[0]==1&&
          work.nodes[0].destinationIndices0c[1]==3,"Original adjacency index mapping");
    check(work.nodes[3].linkCount08==0,"Out-of-radius neighbor was not excluded");
    check(work.nodes[0].linkTypes18[1]==2&&
          work.nodes[0].sourceLinkIndices24[1]==1,"Source type and slot lost");
    originalNavRouteRelax_220d5c(work);
    check(work.nodes[3].cost34==2.f,"Connected original shortest cost");
    const auto path=originalNavRouteFromSource_2207d0(rec,nav,q);
    check(path.size()==1&&path[0]==&rec[0],"Partial top-level source path");
    q.maximumDistance=3.f;
    const auto radius=originalNavAdmit_220aa4(rec,nav,q,epoch);
    check(radius.sourceRecordIndicesInDepthFirstOrder==
        std::vector<std::uint32_t>({0,1,2}),"Strict original radius boundary");
    q.maximumDistance=4.f;q.filterEnabled=true;q.originalFilterListExists=true;
    const std::array<const OriginalSpatialRecord70*,2> allowed{{&rec[0],&rec[2]}};
    q.originalFilterList=allowed;
    check(originalNavMembership_4e3dac(&rec[0],allowed),"Source linked membership true");
    check(!originalNavMembership_4e3dac(&rec[1],allowed),"Source linked membership false");
    const auto restricted=originalNavAdmit_220aa4(rec,nav,q,epoch);
    check(restricted.sourceRecordIndicesInDepthFirstOrder==
        std::vector<std::uint32_t>({0,2}),"Source filter must prune recursive branch");
    q.originalFilterListExists=false;
    check(originalNavAdmit_220aa4(rec,nav,q,epoch)
        .sourceRecordIndicesInDepthFirstOrder.size()==4,
        "No original filter list must bypass membership check");
    epoch.lastSeenEpoch.assign(5,0xffffffffu);
    epoch.seen.assign(5,1);epoch.epoch=0xffffffffu;
    originalNavStartEpoch_21afb8(epoch,5);
    check(epoch.epoch==0&&!epoch.seen[0],"Original epoch wrap must clear marks");
    check(!originalNavAlreadySeen_21b014(epoch,0),"Original wrapped mark still active");
    originalNavMarkSeen_21b038(epoch,0);
    check(originalNavAlreadySeen_21b014(epoch,0),"Original mark write lost");
    originalNavStartEpoch_21afb8(epoch,5);
    check(!originalNavAlreadySeen_21b014(epoch,0),"Original epoch invalidation failed");
    std::vector<float> manyX(257,0.f);auto bigNav=fixture(manyX);
    auto bigRecords=records(manyX);
    for(std::size_t i=0;i+1<manyX.size();i++){
        const auto at=0x80+0x1e*i;
        w16(bigNav,at+2,1);w16(bigNav,at+4,static_cast<std::uint16_t>(i+1));
    }
    bool cap=false;
    try{OriginalNavEpochState visit;(void)originalNavAdmit_220aa4(bigRecords,bigNav,q,visit);}
    catch(const std::out_of_range&){cap=true;}
    check(cap,"Original fixed 256-node allocation limit was ignored");
    auto bad=nav;w16(bad,0x80+4,5000);
    bool malformed=false;
    try{OriginalNavEpochState visit;(void)originalNavAdmit_220aa4(rec,bad,q,visit);}
    catch(const std::out_of_range&){malformed=true;}
    check(malformed,"Malformed original source adjacency must fail");
}
