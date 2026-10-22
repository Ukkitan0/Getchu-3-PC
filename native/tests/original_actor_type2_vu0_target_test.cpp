#include "ae3/original_actor_type2_vu0_target.hpp"
#include "ae3/original_actor_type2_target.hpp"
#include "ae3/original_vu0_geometry.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool p,const char* e){if(!p)throw std::runtime_error(e);}
static unsigned u16(const std::vector<unsigned char>& b,unsigned x){
    ck(x+2<=b.size(),"Private source PCK halfword bounds");
    return b[x]|(unsigned(b[x+1])<<8);
}
static unsigned u32(const std::vector<unsigned char>& b,unsigned x){
    return u16(b,x)|(u16(b,x+2)<<16);
}
static float lane(const std::array<std::byte,16>& v,unsigned i){
    float f{};
    std::memcpy(&f,v.data()+4*i,4);
    return f;
}
static void verify(const std::array<std::array<std::byte,16>,3>& src,
                   unsigned seed){
    OriginalGameRandom_36f3c0 game(seed),oracle(seed);
    std::array<std::byte,16> controller;
    controller.fill(std::byte{0xae});
    const auto result=originalActorType2VU0WriteController40_57bdcc(
        src,game,controller);
    OriginalVU0Vec4 stored{};
    std::memcpy(&stored,controller.data(),16);
    const auto coeff=originalActorTriangleWeights_57c98c(oracle);
    const auto expected=[&](unsigned i){
        const float a=lane(src[0],i)*coeff.originalWeightedCoefficients[0];
        const float b=lane(src[1],i)*coeff.originalWeightedCoefficients[1];
        const float c=lane(src[2],i)*coeff.originalWeightedCoefficients[2];
        return (a+b)+c;
    };
    ck(std::bit_cast<std::uint32_t>(stored.x)==0 &&
       std::bit_cast<std::uint32_t>(stored.y)==0,
       "Original PCPYUD must clear SQC2 controller X/Y lanes");
    ck(std::bit_cast<std::uint32_t>(stored.z)==
            std::bit_cast<std::uint32_t>(expected(2)) &&
       std::bit_cast<std::uint32_t>(stored.w)==
            std::bit_cast<std::uint32_t>(expected(3)),
       "Source 0x57BE8C/BEAC/BEC0/BECC float operation order differs");
    ck(std::memcmp(&stored,&result,16)==0,
       "Controller+0x40 must receive exact native VU0 VF3");
    ck(game.originalCursor()==oracle.originalCursor() &&
       game.originalCursor()==1,
       "Original type-two movement consumes exactly two EE RNG draws");
}
int main(int argc,char**argv){
    ck(argc==1||argc==2,"Optional private source navigation PCK path");
    // Pure native test fixture (not gameplay data).
    std::array<std::array<std::byte,16>,3> synthetic{};
    const std::array<std::array<float,4>,3> floats{{{{1,2,-4,1}},
        {{-8,3,7,2}},{{6,-3,12,3}}}};
    for(unsigned v=0;v<3;++v)
        std::memcpy(synthetic[v].data(),floats[v].data(),16);
    for(unsigned seed:{0u,1u,5489u,0xffffffffu})
        verify(synthetic,seed);
    if(argc==1)return 0;
    std::ifstream f(argv[1],std::ios::binary);
    ck(bool(f),"Private source woods_c PCK unavailable");
    const std::vector<unsigned char> all((std::istreambuf_iterator<char>(f)),{});
    ck(all.size()==3328,"Original PCK size");
    unsigned polygons=0,kindTwo=0,allLinks=0;
    for(unsigned graph=0;graph<2;++graph){
        const unsigned off=graph?2224:96,nbytes=graph?1104:2128;
        const std::vector<unsigned char> b(
            all.begin()+off,all.begin()+off+nbytes);
        const unsigned n=u32(b,0x24),hdr=u32(b,0x3c),vbase=u32(b,0x40);
        ck(n==(graph?20u:39u),"Original polygon group count");
        std::vector<std::array<std::array<std::byte,16>,3>> polygonBytes(n);
        std::vector<OriginalSpatialRecord70> nativeRecords(n);
        for(unsigned id=0;id<n;++id){
            ck(u16(b,hdr+30*id+0x16)==3,"Original triangle vertex count");
            for(unsigned v=0;v<3;++v){
                const unsigned at=vbase+16*u16(b,hdr+30*id+0x18+2*v);
                ck(at+16<=b.size(),"Source vertex outside PCK");
                std::memcpy(polygonBytes[id][v].data(),b.data()+at,16);
                std::memcpy(nativeRecords[id].originalFields.data()+0x40+16*v,
                            b.data()+at,16);
            }
            verify(polygonBytes[id],5489u+polygons);
            ++polygons;
        }
        for(unsigned id=0;id<n;++id){
            const unsigned desc=hdr+30*id;
            const unsigned degree=u16(b,desc+2);
            ck(degree<=3,"Source link degree");
            for(unsigned edge=0;edge<degree;++edge){
                const unsigned dest=u16(b,desc+4+2*edge);
                const unsigned kind=u16(b,desc+10+2*edge);
                ck(dest<n,"Source type-two destination out of range");
                if(kind==2){
                    // Connect source 0x57BDB8 native NAV selection to
                    // the new 0x57BDCC..BED4 type-two VU0/controller write.
                    // The genuine 2-entry route ends at this action-kind-2
                    // source destination. Navigation code stays read-only.
                    const std::array<std::uint32_t,2> sourceRoute{id,dest};
                    const auto nav=originalActorType2Target_57bdb8(
                        nativeRecords,sourceRoute);
                    ck(nav.targetPolygonAt50==&nativeRecords[dest],
                       "Native original type-two route selected wrong polygon");
                    ck(nav.rawOriginalVertices==polygonBytes[dest],
                       "Original navigation source vertex bytes were altered");
                    OriginalGameRandom_36f3c0 integratedRNG(5489);
                    std::array<std::byte,16> controller40{};
                    const auto moved=originalActorType2VU0WriteController40_57bdcc(
                        nav.rawOriginalVertices,integratedRNG,controller40);
                    ck(std::memcmp(&moved,controller40.data(),16)==0 &&
                       integratedRNG.originalCursor()==1,
                       "Original native route -> VU0 -> SQC2 controller+0x40 chain failed");
                    for(unsigned seed:{0u,1u,5489u,0xffffffffu})
                        verify(polygonBytes[dest],seed);
                    ++kindTwo;
                }
                ++allLinks;
            }
        }
    }
    ck(polygons==59&&allLinks==137&&kindTwo==15,
       "Original 59 polygons, 137 edges and 15 type-two links");
    std::cout<<"Type-2 original 0x57BDCC..BED4: "<<polygons
             <<" polygons and "<<kindTwo<<" action-kind-2 edges verified\n";
}
