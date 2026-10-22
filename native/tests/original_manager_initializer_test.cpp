#include "ae3/original_manager_initializer.hpp"
#include "ae3/original_stage_spatial_loader.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool b,const char* message){if(!b)throw std::runtime_error(message);}
static bool same(const OriginalVU0Vec4& a,const OriginalVU0Vec4& b){
    return std::memcmp(&a,&b,sizeof(a))==0;
}
struct Calls final:OriginalManagerInitCalls_57bcb8 {
    unsigned projected=0,advanced=0;
    bool missingProjection=false;
    const OriginalSpatialRecord70* expectedPolygon=nullptr;
    OriginalVU0Vec4 expectedStart{},expectedEnd{},returned{30,40,50,60};
    OriginalVU0Vec4 call_original_21b854(const OriginalSpatialRecord70& polygon,
        const OriginalVU0Vec4& a,const OriginalVU0Vec4& b) override {
        ++projected;
        ck(&polygon==expectedPolygon && same(a,expectedStart) && same(b,expectedEnd),
           "Source 0x57BF00 original polygon/start/end operands differ");
        if(missingProjection)throw std::runtime_error("unrecovered original 0x21B854 projection");
        return returned;
    }
    void call_original_57c7cc(OriginalManagerInitState_57bcb8& state,
                            const OriginalVU0Vec4& sourceA1) override {
        ++advanced;
        ck(same(state.midpoint60,sourceA1),"Original 0x57BF24 store must precede route update");
        ck(state.lifecycle.word08==0x1234,"Original callback must precede method-pair replacement");
    }
};
static void focused(){
    OriginalManagerChild70 child;
    child.alternative30=1;child.word08=0x1234;child.word0c=0x5678;
    OriginalManagerInitState_57bcb8 state{child,{1,2,3,4},{7,8,9,10}};
    OriginalManagerInitSource_57bcb8 source;
    source.ownerPosition40={1.5f,2,3,4};
    OriginalGameRandom_36f3c0 rng(5489);
    const auto cursor=rng.originalCursor();
    Calls calls;
    ck(originalManagerInitialize_57bcb8(state,source,rng,calls)==
       OriginalManagerInitBranch_57bcb8::nearOwnerReturn,"Original near-owner early return");
    ck(child.word08==std::bit_cast<std::uint32_t>(-.5f) && child.word0c==0,
       "Original near branch copies preserved $a1 low64 (controller-owner), not invented zero state");
    ck(state.target40.x==7 && calls.projected==0 && calls.advanced==0 &&
       rng.originalCursor()==cursor,"Near branch touched original target/RNG/dependencies");
    OriginalSpatialRecord70 polygon{};
    source.actorPolygon30=&polygon;source.ownerPosition40={3,2,3,4};
    source.ownerDirection50={1,2,3,4};
    child.word08=0x1234;child.word0c=0x5678;
    calls.expectedPolygon=&polygon;calls.expectedStart=state.controller20;
    calls.expectedEnd={26,52,78,104};
    ck(originalManagerInitialize_57bcb8(state,source,rng,calls)==
       OriginalManagerInitBranch_57bcb8::generalProjection,"Distance exactly 2 must not take strict less-than branch");
    ck(same(state.midpoint60,{15.5f,21,26.5f,32}) && calls.projected==1 && calls.advanced==1,
       "Source general branch midpoint/add/multiply or update ordering differs");
    ck(child.word08==0x57bf5c && child.word0c==0 && rng.originalCursor()==cursor,
       "General branch method pair or RNG state differs");
    child.word08=0x1234;child.word0c=0x5678;calls.missingProjection=true;
    bool blocked=false;
    try{(void)originalManagerInitialize_57bcb8(state,source,rng,calls);}
    catch(const std::runtime_error& e){blocked=std::string(e.what()).find("0x21B854")!=std::string::npos;}
    ck(blocked && child.word08==0x1234 && calls.advanced==1,
       "Unrecovered original projection was bypassed or method state published early");
}
static unsigned u16(std::span<const std::byte> b,std::size_t at){
    ck(at+2<=b.size(),"Private source descriptor bound");
    return std::to_integer<unsigned>(b[at])|(std::to_integer<unsigned>(b[at+1])<<8);
}
static unsigned u32(std::span<const std::byte> b,std::size_t at){return u16(b,at)|(u16(b,at+2)<<16);}
static void retail(const char* path){
    std::ifstream f(path,std::ios::binary);ck(bool(f),"Private original navigation PCK missing");
    const std::vector<char> data((std::istreambuf_iterator<char>(f)),{});
    ck(data.size()==3328,"Original woods_c navigation PCK byte count");
    std::vector<std::byte> bytes;for(unsigned char c:data)bytes.push_back(std::byte(c));
    unsigned edges=0;
    for(unsigned graph=0;graph<2;++graph){
        const auto nav=std::span<const std::byte>(bytes).subspan(graph?2224:96,graph?1104:2128);
        auto loaded=originalStageBuildSourceSpatial_21d46c({0x14000000,nav,0x15000000});
        const auto descriptors=u32(nav,0x3c);
        for(unsigned index=0;index<loaded.records.size();++index){
            const auto at=descriptors+index*30;
            for(unsigned edge=0;edge<u16(nav,at+2);++edge){
                if(u16(nav,at+10+edge*2)!=2)continue;
                const auto next=u16(nav,at+4+edge*2);
                const std::array<std::uint32_t,2> route{index,next};
                OriginalManagerChild70 child;child.flag1c=1;
                OriginalManagerInitState_57bcb8 state{child};
                OriginalManagerInitSource_57bcb8 source;
                source.actorPolygon30=&loaded.records[index];source.stageRecords=loaded.records;
                source.routePolygonIds=route;source.originalNavigation=nav;
                OriginalGameRandom_36f3c0 rng(5489),reference(5489);
                const auto vertices=originalActorType2Target_57bdb8(loaded.records,route);
                std::array<OriginalVU0Vec4,3> points{};
                for(unsigned j=0;j<3;++j)std::memcpy(&points[j],vertices.rawOriginalVertices[j].data(),16);
                const auto expected=originalActorRandomTriangle_57c98c(points,reference);
                Calls calls;
                ck(originalManagerInitialize_57bcb8(state,source,rng,calls)==
                   OriginalManagerInitBranch_57bcb8::typeTwoTarget,"Actual original type-2 edge selected wrong initializer branch");
                ck(state.target50==&loaded.records[next] && same(state.target40,expected),
                   "Original source second polygon/random target not connected");
                ck(child.word08==0x57c554 && child.word0c==0 && rng.originalCursor()==1 &&
                   calls.projected==0 && calls.advanced==0,"Type-2 original RNG/method/update ordering differs");
                ++edges;
            }
        }
    }
    ck(edges==15,"Expected all 15 authentic woods_c type-2 directed actions");
    std::cout<<"ORIGINAL_MANAGER_INIT_RETAIL_PASSED type2_edges="<<edges<<'\n';
}
int main(int argc,char**argv){try{focused();if(argc==2)retail(argv[1]);
    std::cout<<"ORIGINAL_MANAGER_57BCB8_INIT_PASSED native_control_flow=1\n";return 0;
}catch(const std::exception& e){std::cerr<<"ORIGINAL_MANAGER_INIT_FAILED: "<<e.what()<<'\n';return 1;}}
