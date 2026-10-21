#include "ae3/original_actor_route_follower.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool ok,const char* m){if(!ok)throw std::runtime_error(m);}
struct Original:OriginalActorRouteFollowerCalls_57c7cc {
    std::vector<const OriginalSpatialRecord70*> applied;
    std::vector<const OriginalSpatialRecord70*> projected;
    const OriginalSpatialRecord70* stage=nullptr;
    void call_original_57c8ec_update_target(const OriginalSpatialRecord70* target) override {
        applied.push_back(target);
    }
    const OriginalSpatialRecord70* call_original_4c1b3c_project(
        const OriginalSpatialRecord70* originalActorStagePolygon) override {
        ck(originalActorStagePolygon==stage,"original projection receives actor+0x30");
        ck(!projected.empty(),"original projection expected input");
        const auto* result=projected.front();
        projected.erase(projected.begin());
        return result;
    }
};
static std::uint32_t u16(std::span<const std::byte> data,std::size_t p) {
    ck(p+2<=data.size(),"retail graph halfword bounds");
    return std::to_integer<unsigned>(data[p])|(std::to_integer<unsigned>(data[p+1])<<8);
}
static std::uint32_t u32(std::span<const std::byte> data,std::size_t p) {
    return u16(data,p)|(u16(data,p+2)<<16);
}
int main(int argc,char**argv) {
    std::array<OriginalSpatialRecord70,5> records{};
    const std::array<std::uint32_t,5> route{0,1,2,3,4};
    Original original;original.stage=&records[0];
    OriginalActorRouteFollowerState_57c7cc state{records,route,&records[0],nullptr,false};
    auto result=originalActorAdvanceRoute_57c7cc(state,original);
    ck(state.activePolygonAtController50==&records[0],"single-polygon fallback");
    ck(result.sourceTargetsApplied==1&&result.sourceProjectionsPerformed==0,
       "single-polygon original calls");
    ck(original.applied==std::vector<const OriginalSpatialRecord70*>({&records[0]}),
       "single-polygon original target");
    original.applied.clear();
    state.sourceHasMultiplePolygonsAtController1c=true;
    original.projected={&records[0]};
    result=originalActorAdvanceRoute_57c7cc(state,original);
    ck(result.returnedEarlyBecauseCurrentTargetMatched&&result.sourceProjectionsPerformed==1,
       "unchanged projected target must exit");
    ck(original.applied.empty(),"unchanged target must not reapply");
    // First projection rejects current target. Route[1] is applied,
    // route[2] projects equal, route[3] projects different: stop there.
    original.projected={nullptr,&records[2],&records[4]};
    result=originalActorAdvanceRoute_57c7cc(state,original);
    ck(!result.returnedEarlyBecauseCurrentTargetMatched,"route advances");
    ck(state.activePolygonAtController50==&records[2],
       "mismatch must retain last projected matching route polygon");
    ck(original.applied==std::vector<const OriginalSpatialRecord70*>(
        {&records[1],&records[2],&records[3]}),
       "source applies later candidate BEFORE checking projection");
    ck(result.sourceProjectionsPerformed==3&&result.sourceTargetsApplied==3&&
       result.followingRouteIndicesTested==2,"source loop counts");
    original.applied.clear();
    original.projected={&records[2]};
    result=originalActorAdvanceRoute_57c7cc(state,original);
    ck(result.returnedEarlyBecauseCurrentTargetMatched&&original.applied.empty(),
       "current target match must avoid updates");
    state.activePolygonAtController50=nullptr;
    original.projected={&records[2],&records[3],&records[4]};
    result=originalActorAdvanceRoute_57c7cc(state,original);
    ck(state.activePolygonAtController50==&records[4],"full route advancement");
    ck(result.sourceTargetsApplied==4&&result.sourceProjectionsPerformed==3,
       "source applies route[1] then tests route[2..4]");
    original.applied.clear();original.projected.clear();
    state.originalRoutePolygonIndices=std::span<const std::uint32_t>(route).first(2);
    state.activePolygonAtController50=nullptr;
    result=originalActorAdvanceRoute_57c7cc(state,original);
    ck(state.activePolygonAtController50==&records[1]&&
       result.sourceTargetsApplied==1&&result.sourceProjectionsPerformed==0,
       "two-polygon route");
    bool threw=false;
    state.originalRoutePolygonIndices=std::span<const std::uint32_t>(route).first(1);
    try{(void)originalActorAdvanceRoute_57c7cc(state,original);}
    catch(const std::out_of_range&){threw=true;}
    ck(threw,"missing route[1] must be rejected by host");
    std::array<std::uint32_t,2> invalid{0,7};
    state.originalRoutePolygonIndices=invalid;state.activePolygonAtController50=nullptr;
    threw=false;
    try{(void)originalActorAdvanceRoute_57c7cc(state,original);}
    catch(const std::out_of_range&){threw=true;}
    ck(threw,"invalid source polygon index must be rejected by host");
    state.originalStagePolygonAtActor30=nullptr;
    threw=false;
    try{(void)originalActorAdvanceRoute_57c7cc(state,original);}
    catch(const std::invalid_argument&){threw=true;}
    ck(threw,"missing actual original actor stage owner");

    ck(argc==1||argc==2,"optional original woods_c/navigation.pck");
    if(argc==2){
        std::ifstream file(argv[1],std::ios::binary);
        ck(bool(file),"retail woods_c navigation input missing");
        const std::vector<char> chars((std::istreambuf_iterator<char>(file)),{});
        ck(chars.size()==3328,"retail PCK byte length");
        std::vector<std::byte> pck;
        for(unsigned char c:chars)pck.push_back(std::byte(c));
        unsigned edges=0,kindTwo=0;
        for(unsigned graph=0;graph<2;++graph){
            const auto raw=std::span<const std::byte>(pck).subspan(graph?2224:96,graph?1104:2128);
            const auto count=u32(raw,0x24),base=u32(raw,0x3c),vertexBase=u32(raw,0x40);
            ck(count==(graph?20u:39u),"genuine nav polygon count");
            std::vector<OriginalSpatialRecord70> originalRecords(count);
            for(unsigned i=0;i<count;++i){
                const auto at=base+0x1e*i,vertexCount=u16(raw,at+0x16);
                ck(vertexCount>=1&&vertexCount<=3,"original vertex count");
                for(unsigned j=0;j<vertexCount;++j){
                    const auto offset=vertexBase+16u*u16(raw,at+0x18+2*j);
                    ck(offset+16<=raw.size(),"original vector source range");
                    for(unsigned k=0;k<16;++k)
                        originalRecords[i].originalFields[0x40+16*j+k]=raw[offset+k];
                }
            }
            for(std::uint32_t from=0;from<count;++from){
                const auto at=base+0x1e*from,degree=u16(raw,at+2);
                ck(degree<=3,"original source edge degree");
                for(unsigned slot=0;slot<degree;++slot){
                    const std::uint32_t to=u16(raw,at+4+slot*2);
                    ck(to<count,"original source edge target");
                    const std::array<std::uint32_t,2> edge{from,to};
                    OriginalActorRouteFollowerState_57c7cc current{
                        originalRecords,edge,&originalRecords[from],nullptr,true};
                    Original callbacks;callbacks.stage=&originalRecords[from];
                    const auto follow=originalActorAdvanceRoute_57c7cc(current,callbacks);
                    ck(current.activePolygonAtController50==&originalRecords[to],
                        "real original route edge was not chosen");
                    ck(follow.sourceTargetsApplied==1&&follow.sourceProjectionsPerformed==0,
                        "original two-node route projection count");
                    ck(callbacks.applied==std::vector<const OriginalSpatialRecord70*>(
                        {&originalRecords[to]}),"original target update identity");
                    kindTwo+=(u16(raw,at+0x0a+2*slot)==2);
                    ++edges;
                }
            }
        }
        ck(edges==137&&kindTwo==15,
            "all 137 original route edges and 15 original special actions");
    }
}
