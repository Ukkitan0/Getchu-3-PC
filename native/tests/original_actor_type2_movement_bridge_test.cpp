#include "ae3/original_actor_type2_movement_bridge.hpp"
#include "ae3/original_actor_type2_target.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void require(bool x,const char* why){if(!x)throw std::runtime_error(why);}
static std::array<std::byte,16> pack(float x,float y,float z,float w){
    const std::array<float,4> v{x,y,z,w};std::array<std::byte,16> b{};
    std::memcpy(b.data(),v.data(),16);return b;
}
static unsigned u16(const std::vector<unsigned char>& v,std::size_t p){
    require(p+2<=v.size(),"Original graph descriptor bounds");
    return v[p]|(unsigned(v[p+1])<<8);
}
static unsigned u32(const std::vector<unsigned char>& v,std::size_t p){
    return u16(v,p)|(u16(v,p+2)<<16);
}
static void exercise(const std::vector<OriginalSpatialRecord70>& polygons,
                     unsigned a,unsigned b){
    const std::array<std::uint32_t,2> route{a,b};
    auto owner=pack(0,0,0,10000);
    auto close=pack(1,0,0,-10000);
    auto far=pack(30,0,0,-10000);
    std::array<std::byte,16> target40{};
    const OriginalSpatialRecord70* target=&polygons[a];
    std::uint32_t route8=0xaabbccdd,routeC=0x01020304;
    OriginalGameRandom_36f3c0 rng(5489),oracle(5489);
    const auto startCursor=rng.originalCursor();
    const auto early=originalActorType2MovementBridge_57bcb8(
        true,true,2,close,owner,polygons,route,rng,
        route8,routeC,target40,target);
    require(early.outcome==OriginalActorType2BridgeOutcome_57bcb8::OriginalDistanceGateReset_57bd3c &&
            route8==0&&routeC==0&&target==&polygons[a]&&rng.originalCursor()==startCursor,
            "Original distance gate must return before any kind-two RNG/target update");
    route8=0xaabbccdd;routeC=0x01020304;
    const auto pending=originalActorType2MovementBridge_57bcb8(
        true,true,1,far,owner,polygons,route,rng,
        route8,routeC,target40,target);
    require(pending.outcome==OriginalActorType2BridgeOutcome_57bcb8::OriginalOtherBranchUnrecovered&&
            route8==0xaabbccdd&&routeC==0x01020304&&rng.originalCursor()==startCursor,
            "Non-type-two original movement must not be fabricated");
    const auto one=originalActorType2MovementBridge_57bcb8(
        false,true,2,far,owner,polygons,route,rng,
        route8,routeC,target40,target);
    const auto original=originalActorType2Target_57bdb8(polygons,route);
    std::array<std::byte,16> oracleTarget40{};
    const auto originalVector=originalActorType2VU0WriteController40_57bdcc(
        original.rawOriginalVertices,oracle,oracleTarget40);
    require(one.outcome==OriginalActorType2BridgeOutcome_57bcb8::OriginalType2TargetWritten_57bed4 &&
            !one.originalGate.originalComputedLengthF0 &&
            target==&polygons[b] && target==original.targetPolygonAt50 &&
            std::memcmp(target40.data(),oracleTarget40.data(),16)==0 &&
            std::memcmp(&originalVector,target40.data(),16)==0&&
            rng.originalCursor()==oracle.originalCursor()&&rng.originalCursor()==1,
            "Original kind-two movement must match separate authentic NAV+VU0 sequence");
}
int main(int argc,char**argv){
    require(argc==1||argc==2,"Optional owner-private woods_c PCK");
    std::vector<OriginalSpatialRecord70> synthetic(2);
    for(unsigned i=0;i<2;++i)for(unsigned j=0;j<3;++j){
        auto v=pack(i*3.0f,j,2.0f*i+j,1.0f);
        std::memcpy(synthetic[i].originalFields.data()+0x40+16*j,v.data(),16);
    }
    exercise(synthetic,0,1);
    if(argc==1)return 0;
    std::ifstream input(argv[1],std::ios::binary);
    require(bool(input),"Original user-owned source PCK missing");
    const std::vector<unsigned char> file((std::istreambuf_iterator<char>(input)),{});
    require(file.size()==3328,"Expected private original nav resource bytes");
    unsigned checked=0,polygons=0;
    for(unsigned g=0;g<2;++g){
        unsigned start=g?2224:96,length=g?1104:2128;
        std::vector<unsigned char> data(file.begin()+start,file.begin()+start+length);
        auto count=u32(data,0x24),headers=u32(data,0x3c),verts=u32(data,0x40);
        require(count==(g?20u:39u),"Original graph counts");
        std::vector<OriginalSpatialRecord70> records(count);
        for(unsigned p=0;p<count;++p){
            for(unsigned v=0;v<3;++v){
                unsigned at=verts+16*u16(data,headers+30*p+0x18+2*v);
                require(at+16<=data.size(),"Original source vertex record bound");
                std::memcpy(records[p].originalFields.data()+0x40+16*v,
                            data.data()+at,16);
            }
            ++polygons;
        }
        for(unsigned p=0;p<count;++p){
            unsigned base=headers+30*p,degree=u16(data,base+2);
            for(unsigned e=0;e<degree;++e){
                const unsigned neighbor=u16(data,base+4+2*e);
                const unsigned kind=u16(data,base+0xa+2*e);
                require(neighbor<count,"Source polygon link invalid");
                if(kind==2){exercise(records,p,neighbor);++checked;}
            }
        }
    }
    require(polygons==59&&checked==15,"All 59 original triangles and 15 type-two edges");
}
