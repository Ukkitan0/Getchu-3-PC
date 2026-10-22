#include "ae3/original_actor_xz_proximity.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool b,const char*m){if(!b)throw std::runtime_error(m);}
static std::array<std::byte,16> pack(float x,float y,float z,float w){
    const std::array<float,4> value{x,y,z,w};std::array<std::byte,16> bytes{};
    std::memcpy(bytes.data(),value.data(),16);return bytes;
}
static void trial(std::array<std::byte,16> a,std::array<std::byte,16> b){
    const auto actual=originalActorXZProximity_57c080(a,b);
    OriginalVU0Vec4 av{},bv{};
    std::memcpy(&av,a.data(),16);std::memcpy(&bv,b.data(),16);
    const float dx=av.x-bv.x,dy=av.y-bv.y,dz=av.z-bv.z,dw=av.w-bv.w;
    const float score=(dx*dx+0.0f)+dz*dz;
    ck(std::bit_cast<std::uint32_t>(actual.originalDifferenceVF1.x)==std::bit_cast<std::uint32_t>(dx)&&
       std::bit_cast<std::uint32_t>(actual.originalDifferenceVF1.y)==std::bit_cast<std::uint32_t>(dy)&&
       std::bit_cast<std::uint32_t>(actual.originalDifferenceVF1.z)==std::bit_cast<std::uint32_t>(dz)&&
       std::bit_cast<std::uint32_t>(actual.originalDifferenceVF1.w)==std::bit_cast<std::uint32_t>(dw),
       "Full source VSUB differs");
    ck(std::bit_cast<std::uint32_t>(actual.originalPackedVF3.x)==std::bit_cast<std::uint32_t>(dx)&&
       std::bit_cast<std::uint32_t>(actual.originalPackedVF3.y)==0&&
       std::bit_cast<std::uint32_t>(actual.originalPackedVF3.z)==std::bit_cast<std::uint32_t>(dz)&&
       std::bit_cast<std::uint32_t>(actual.originalPackedVF3.w)==std::bit_cast<std::uint32_t>(dw),
       "Original MMI PEXTLW PCPYUD PCPYLD lanes differ");
    ck(std::bit_cast<std::uint32_t>(actual.originalSquaredDistanceF0)==std::bit_cast<std::uint32_t>(score)&&
       actual.originalWithin2Units==(std::sqrt(score)<2.0f),
       "Original 0x57C0A8 squared XZ distance gate differs");
}
static unsigned u16(const std::vector<unsigned char>& b,unsigned a){
    ck(a+2<=b.size(),"PCK halfword bounds");return b[a]|(unsigned(b[a+1])<<8);
}
static unsigned u32(const std::vector<unsigned char>& b,unsigned a){return u16(b,a)|(u16(b,a+2)<<16);}
int main(int argc,char**argv){
    ck(argc==1||argc==2,"Optional private woods_c/navigation.pck argument");
    trial(pack(0,200,0,123),pack(0,-400,0,-456));
    trial(pack(1,1000,0,0),pack(0,-1000,0,0));
    trial(pack(2,0,0,0),pack(0,0,0,0));
    trial(pack(std::nextafter(2.0f,0.0f),0,0,0),pack(0,0,0,0));
    trial(pack(std::nextafter(2.0f,3.0f),0,0,0),pack(0,0,0,0));
    trial(pack(0,0,3,0),pack(0,0,0,0));
    std::array<std::byte,16> raw=pack(15,5,21,27);
    const auto packed=originalActorPack21B854Return_57c07c(raw);
    auto expect=pack(0,0,21,27);
    ck(packed==expect,"Original external return PCPYUD must retain only upper half");
    if(argc==1)return 0;
    std::ifstream f(argv[1],std::ios::binary);
    ck(bool(f),"Original private PCK missing");
    const std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(f)),{});
    ck(bytes.size()==3328,"Private PCK size changed");
    unsigned count=0;
    for(unsigned g=0;g<2;++g){
        const unsigned start=g?2224:96,len=g?1104:2128;
        const std::vector<unsigned char> b(bytes.begin()+start,bytes.begin()+start+len);
        const auto n=u32(b,0x24),desc=u32(b,0x3c),vert=u32(b,0x40);
        ck(n==(g?20u:39u),"Original polygon count");
        for(unsigned p=0;p<n;++p){
            std::array<std::array<std::byte,16>,3> v{};
            for(unsigned i=0;i<3;++i){
                const unsigned o=vert+16*u16(b,desc+30*p+0x18+2*i);
                ck(o+16<=b.size(),"PCK vertex bounds");
                std::memcpy(v[i].data(),b.data()+o,16);
            }
            trial(v[0],v[1]);trial(v[1],v[2]);trial(v[2],v[0]);
            const auto packedSource=originalActorPack21B854Return_57c07c(v[0]);
            trial(packedSource,v[1]);
            ++count;
        }
    }
    ck(count==59,"Expected all 59 genuine polygons");
}
