#include "ae3/original_actor_fallback_sampler.hpp"
#include "ae3/original_actor_fallback_vu0_native.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void require(bool ok,const char* error) {
    if(!ok)throw std::runtime_error(error);
}
static std::uint16_t u16(const std::vector<unsigned char>& bytes,std::size_t offset) {
    require(offset+2<=bytes.size(),"Original navigation PCK halfword out of bounds");
    return bytes[offset]|(unsigned(bytes[offset+1])<<8);
}
static std::uint32_t u32(const std::vector<unsigned char>& bytes,std::size_t offset) {
    return u16(bytes,offset)|(std::uint32_t(u16(bytes,offset+2))<<16);
}
static std::array<std::byte,16> encode(float x,float y,float z,float w) {
    const std::array<float,4> value{x,y,z,w};
    std::array<std::byte,16> output{};
    std::memcpy(output.data(),value.data(),16);
    return output;
}
static float sourceLane(const std::array<std::byte,16>& value,std::size_t lane) {
    float answer{};
    std::memcpy(&answer,value.data()+lane*4,4);
    return answer;
}
static void checkPolygon(const OriginalSpatialRecord70& original,
                         std::uint32_t seed) {
    const auto owner=encode(123.0f,-8.0f,-200.0f,2.0f);
    const auto controller=encode(11.0f,30.0f,200.0f,3.0f);
    constexpr std::uint32_t originalQBits=0x3fc90fda;
    const float q=std::bit_cast<float>(originalQBits);
    OriginalGameRandom_36f3c0 nativeRandom(seed),independentRandom(seed);
    OriginalActorFallbackNativeVU0_57ca68 nativeVU0;
    OriginalActorFallbackVu0R_57ca68 sourceR{0xdeadbeefu};
    const auto actual=originalActorFallbackSelect_57ca68(
        original,owner,controller,originalQBits,nativeRandom,nativeVU0,&sourceR);

    const auto v0=originalSpatialVector_21b458(original,0);
    const auto v1=originalSpatialVector_21b458(original,1);
    const auto v2=originalSpatialVector_21b458(original,2);
    float best=0.0f;
    std::uint64_t bestPacked=0;
    std::size_t winner=0;
    for(std::size_t sample=0;sample<6;++sample) {
        const auto weights=originalActorTriangleWeights_57c98c(independentRandom);
        // Independent scalar oracle matching exact source VMULx/VADD order.
        const float za=sourceLane(v0,2)*weights.originalWeightedCoefficients[0];
        const float zb=sourceLane(v1,2)*weights.originalWeightedCoefficients[1];
        const float zc=sourceLane(v2,2)*weights.originalWeightedCoefficients[2];
        const float wa=sourceLane(v0,3)*weights.originalWeightedCoefficients[0];
        const float wb=sourceLane(v1,3)*weights.originalWeightedCoefficients[1];
        const float wc=sourceLane(v2,3)*weights.originalWeightedCoefficients[2];
        const float z=(za+zb)+zc;
        const float w=(wa+wb)+wc;
        const float left=(z-sourceLane(owner,2))*q;
        const float right=(sourceLane(controller,2)-z)*q;
        // Real source 0x4748AC is (X*X+Y*Y)+Z*Z. Original PCPYUD
        // clears X/Y, so only the Z term contributes to this score.
        const float score=(0.0f+0.0f)+(left*right);
        if(sample==0||best<score) {
            best=score;
            winner=sample;
            bestPacked=std::uint64_t(std::bit_cast<std::uint32_t>(z))|
                (std::uint64_t(std::bit_cast<std::uint32_t>(w))<<32);
        }
    }
    require(actual.sourceCandidatesEvaluated==6 &&
            actual.sourceWinningCandidate==winner,
            "Original fallback candidate winner differs from source scalar math");
    require(actual.originalWinningPackedTargetLow64==0 &&
            actual.originalWinningPackedTargetHigh64==bestPacked,
            "Original PCPYUD winning 128-bit target differs from source");
    require(nativeRandom.originalCursor()==independentRandom.originalCursor() &&
            nativeRandom.originalCursor()==11,
            "Original game RNG sequence changed during candidate selection");
    require(sourceR.rawR==0x3f800000u &&
            sourceR.sourceVRINITCalls==12 &&
            sourceR.sourceVRXORCalls==12,
            "Original VU0 R and game RNG were incorrectly combined");
}
int main(int argc,char**argv) {
    require(argc==1||argc==2,"Optional original woods_c navigation.pck path");
    OriginalSpatialRecord70 synthetic{};
    const auto a=encode(-8,0,-20,0);
    const auto b=encode(6,0,40,0);
    const auto c=encode(30,0,-2,0);
    for(unsigned i=0;i<3;++i) {
        const auto& value=i==0?a:(i==1?b:c);
        std::memcpy(synthetic.originalFields.data()+0x40+i*16,value.data(),16);
    }
    for(const auto seed:{0u,1u,5489u,0xffffffffu})
        checkPolygon(synthetic,seed);
    if(argc==1)return 0;
    std::ifstream f(argv[1],std::ios::binary);
    require(bool(f),"Original private navigation PCK missing");
    const std::vector<unsigned char> all((std::istreambuf_iterator<char>(f)),{});
    require(all.size()==3328,"Original owner-private PCK size differs");
    std::size_t checked=0;
    for(unsigned graph=0;graph<2;++graph) {
        const unsigned base=graph?2224:96;
        const unsigned length=graph?1104:2128;
        const std::vector<unsigned char> g(all.begin()+base,all.begin()+base+length);
        const auto count=u32(g,0x24),descs=u32(g,0x3c),verts=u32(g,0x40);
        require(count==(graph?20u:39u),"Original polygon group count changed");
        for(unsigned p=0;p<count;++p) {
            OriginalSpatialRecord70 polygon{};
            for(unsigned v=0;v<3;++v) {
                const unsigned at=verts+16*u16(g,descs+30*p+0x18+2*v);
                require(at+16<=g.size(),"Original polygon source vector out of range");
                std::memcpy(polygon.originalFields.data()+0x40+16*v,g.data()+at,16);
            }
            checkPolygon(polygon,5489u+static_cast<std::uint32_t>(checked));
            ++checked;
        }
    }
    require(checked==59,"All 59 original woods_c polygons not covered");
}
