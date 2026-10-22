#include "ae3/original_actor_fallback_vu0_native.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool p,const char* what){if(!p)throw std::runtime_error(what);}
static std::array<std::byte,16> pack(float x,float y,float z,float w){
    std::array<std::byte,16> out{};
    const std::array<float,4> data{x,y,z,w};
    std::memcpy(out.data(),data.data(),16);return out;
}
static float lane(const std::array<std::byte,16>& p,unsigned i){
    float value=0;std::memcpy(&value,p.data()+4*i,4);return value;
}
static unsigned u16(const std::vector<unsigned char>& d,std::size_t i){
    ck(i+2<=d.size(),"Private PCK bounds");return d[i]|(unsigned(d[i+1])<<8);
}
static unsigned u32(const std::vector<unsigned char>& d,std::size_t i){
    return u16(d,i)|(u16(d,i+2)<<16);
}
static void verify(const std::array<std::array<std::byte,16>,3>& points){
    OriginalActorFallbackNativeVU0_57ca68 native;
    const auto owner=pack(199,32,37.5f,-12.5f);
    const auto controller=pack(72,23,-12.0f,30.0f);
    const OriginalActorTriangleWeights_57c98c weights{0.2f,0.3f,0.5f,
        {0.22666667f,0.30666667f,0.46666667f}};
    for(std::uint32_t qb:{0x3fc90fdau,0xbfc90fdau,0x3f800000u})
        for(std::size_t i=0;i<6;i++){
            const auto actual=native.call_original_57ca68_candidate_VU0(
                points,owner,controller,qb,weights,i);
            const float q=std::bit_cast<float>(qb);
            // Independent scalar lane oracle with the exact original order.
            const float a=lane(points[0],2)*weights.originalWeightedCoefficients[0];
            const float b=lane(points[1],2)*weights.originalWeightedCoefficients[1];
            const float c=lane(points[2],2)*weights.originalWeightedCoefficients[2];
            const float candidate=(a+b)+c;
            const float diffOwner=(candidate-lane(owner,2))*q;
            const float diffController=(lane(controller,2)-candidate)*q;
            ck(actual.originalPackedTargetLow64==0,"PCPYUD lower half");
            ck(std::uint32_t(actual.originalPackedTargetHigh64)==
               std::bit_cast<std::uint32_t>(candidate),"PCPYUD upper Z lane");
            ck(std::bit_cast<std::uint32_t>(actual.originalScoreVectorA.z)==
               std::bit_cast<std::uint32_t>(diffOwner),"owner displacement + Q");
            ck(std::bit_cast<std::uint32_t>(actual.originalScoreVectorB.z)==
               std::bit_cast<std::uint32_t>(diffController),"controller displacement + Q");
            ck(actual.originalScoreVectorA.x==0 && actual.originalScoreVectorB.x==0,
               "Original PCPYUD must clear VF.x");
            const float wa=lane(points[0],3)*weights.originalWeightedCoefficients[0];
            const float wb=lane(points[1],3)*weights.originalWeightedCoefficients[1];
            const float wc=lane(points[2],3)*weights.originalWeightedCoefficients[2];
            const float candidateW=(wa+wb)+wc;
            ck(std::uint32_t(actual.originalPackedTargetHigh64>>32)==
               std::bit_cast<std::uint32_t>(candidateW),
               "Original PCPYUD winner W lane must retain first/repeated VMULx order");
            ck(std::bit_cast<std::uint32_t>(actual.originalScoreVectorA.w)==
               std::bit_cast<std::uint32_t>((candidateW-lane(owner,3))*q),
               "Original source owner W displacement or VMULq order differs");
            ck(std::bit_cast<std::uint32_t>(actual.originalScoreVectorB.w)==
               std::bit_cast<std::uint32_t>((lane(controller,3)-candidateW)*q),
               "Original source controller W displacement or VMULq order differs");
            ck(actual.originalScoreVectorA.y==0 && actual.originalScoreVectorB.y==0,
               "Original PCPYUD must clear VF.y");
        }
}
int main(int argc,char**argv){
    ck(argc==1||argc==2,"optional private woods_c/navigation.pck");
    verify({pack(0,0,1,1),pack(0,0,3,1),pack(4,0,2,1)});
    if(argc!=2)return 0;
    std::ifstream f(argv[1],std::ios::binary);
    ck(bool(f),"Private original woods_c PCK missing");
    const std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(f)),{});
    ck(bytes.size()==3328,"Private PCK size");
    std::size_t all=0;
    for(unsigned g=0;g<2;++g){
        const unsigned off=g?2224:96,len=g?1104:2128;
        const std::vector<unsigned char> nav(bytes.begin()+off,bytes.begin()+off+len);
        const auto n=u32(nav,0x24), desc=u32(nav,0x3c), vertices=u32(nav,0x40);
        ck(n==(g?20u:39u),"Original polygon count");
        for(unsigned p=0;p<n;++p){
            std::array<std::array<std::byte,16>,3> pts{};
            for(unsigned v=0;v<3;++v){
                auto at=vertices+16*u16(nav,desc+30*p+0x18+2*v);
                ck(at+16<=nav.size(),"Original polygon vertex bound");
                std::memcpy(pts[v].data(),nav.data()+at,16);
            }
            verify(pts);++all;
        }
    }
    ck(all==59,"Original 59 private polygons covered");
}
