#include "ae3/original_actor_distance_gate.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void check(bool p,const char* msg){if(!p)throw std::runtime_error(msg);}
static std::array<std::byte,16> pack(const OriginalVU0Vec4& a){
    std::array<std::byte,16> b{};
    std::memcpy(b.data(),&a,16);
    return b;
}
static void verifyPair(const OriginalVU0Vec4& start,
                       const OriginalVU0Vec4& end) {
    const auto a=pack(start),b=pack(end);
    const auto gate=originalActorDistanceGate_57bcdc(true,a,b);
    const auto diff=OriginalVU0Vec4{start.x-end.x,start.y-end.y,
                                    start.z-end.z,start.w-end.w};
    const float length=std::sqrt((diff.x*diff.x+diff.y*diff.y)+diff.z*diff.z);
    check(gate.originalComputedLengthF0.has_value(),"Source distance calculation missing");
    check(std::bit_cast<std::uint32_t>(*gate.originalComputedLengthF0)==
          std::bit_cast<std::uint32_t>(length),"Source XYZ SQRT float32 differs");
    check(gate.originalClearsControllerRoute08And0c==(length<2.0f),
          "Strict source 2.0 threshold differs");
    std::uint32_t field08=0x12345678u,field0c=0xaabbccddu;
    originalActorApplyDistanceGate_57bd3c(gate,field08,field0c);
    check(field08==(gate.originalClearsControllerRoute08And0c?0u:0x12345678u) &&
          field0c==(gate.originalClearsControllerRoute08And0c?0u:0xaabbccddu),
          "Original source controller+08/+0C write branch differs");
    const auto off=originalActorDistanceGate_57bcdc(false,a,b);
    check(!off.originalComputedLengthF0.has_value() &&
          !off.originalClearsControllerRoute08And0c,
          "Variant-zero source must skip all distance math");
}
static std::uint16_t read16(const std::vector<unsigned char>& b,std::size_t at){
    check(at+2<=b.size(),"Original PCK read bounds");
    return b[at]|(std::uint16_t(b[at+1])<<8);
}
static std::uint32_t read32(const std::vector<unsigned char>& b,std::size_t at){
    return read16(b,at)|(std::uint32_t(read16(b,at+2))<<16);
}
int main(int argc,char**argv) {
    check(argc==1||argc==2,"Optional private original woods_c/navigation.pck");
    verifyPair({0,0,0,0},{0,0,0,1e9f});
    verifyPair({0,0,0,0},{2,0,0,0});
    verifyPair({0,0,0,0},{std::nextafter(2.0f,0.0f),0,0,0});
    verifyPair({0,0,0,0},{std::nextafter(2.0f,3.0f),0,0,0});
    verifyPair({1,2,3,4},{2,3,4,5});
    verifyPair({0,0,0,0},{std::numeric_limits<float>::quiet_NaN(),0,0,0});
    if(argc==1)return 0;
    std::ifstream f(argv[1],std::ios::binary);
    check(bool(f),"Original owner-private PCK not found");
    const std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(f)),{});
    check(bytes.size()==3328,"Source PCK size changed");
    std::size_t checked=0;
    for(unsigned group=0;group<2;++group){
        const unsigned base=group?2224:96,count=group?20:39,
                       length=group?1104:2128;
        const std::vector<unsigned char> b(bytes.begin()+base,bytes.begin()+base+length);
        check(read32(b,0x24)==count,"Original group polygon count");
        const unsigned head=read32(b,0x3c),vertices=read32(b,0x40);
        for(unsigned i=0;i<count;++i){
            const unsigned p=head+30*i;
            check(read16(b,p+0x16)==3,"Source triangle count");
            std::array<OriginalVU0Vec4,3> v{};
            for(unsigned j=0;j<3;++j){
                unsigned at=vertices+16*read16(b,p+0x18+2*j);
                check(at+16<=b.size(),"Original vector out of bounds");
                std::memcpy(&v[j],b.data()+at,16);
            }
            verifyPair(v[0],v[1]);
            verifyPair(v[1],v[2]);
            verifyPair(v[2],v[0]);
            verifyPair({v[0].x,v[0].y,v[0].z,1.0e10f},
                       {v[1].x,v[1].y,v[1].z,-1.0e10f});
            ++checked;
        }
    }
    check(checked==59,"Authentic 59 source polygons expected");
}
