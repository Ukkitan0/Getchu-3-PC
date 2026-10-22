#include "ae3/original_actor_fallback_sampler.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool b,const char* why){if(!b)throw std::runtime_error(why);}
static unsigned u16(std::span<const std::byte> src,unsigned at){
    ck(at+2<=src.size(),"Original source halfword bounds");
    return std::to_integer<unsigned>(src[at])|
      (std::to_integer<unsigned>(src[at+1])<<8);
}
static unsigned u32(std::span<const std::byte> src,unsigned at){
    return u16(src,at)|(u16(src,at+2)<<16);
}
class OriginalTransformCheck final:public OriginalActorFallbackVU0Transform_57ca68 {
public:
    std::array<std::array<std::byte,16>,3> expectedVertices{};
    std::array<std::byte,16> expectedOwnerA1{};
    std::array<std::byte,16> expectedControllerA2{};
    std::uint32_t expectedVU0Q=0;
    OriginalGameRandom_36f3c0 independent;
    unsigned sourceCalls=0;
    bool initialScoreNaN=false;
    explicit OriginalTransformCheck(std::uint32_t seed):independent(seed){}
    OriginalActorFallbackCandidate_57ca68 call_original_57ca68_candidate_VU0(
        const std::array<std::array<std::byte,16>,3>& originalVertices,
        const std::array<std::byte,16>& originalA1,
        const std::array<std::byte,16>& originalA2,
        std::uint32_t originalQ,
        const OriginalActorTriangleWeights_57c98c& weights,
        std::size_t trial) override {
        ck(trial==sourceCalls&&trial<6,"Original source sample-order mismatch");
        ck(originalVertices==expectedVertices,"Original polygon 0x21B458 vertex bytes changed");
        ck(originalA1==expectedOwnerA1,"Original fallback actor owner A1 vector changed");
        ck(originalA2==expectedControllerA2,"Original fallback controller A2 vector changed");
        ck(originalA1!=originalA2,"Fallback tests must use distinct original source arguments");
        ck(originalQ==expectedVU0Q,"Original VMULq source Q must be passed unmodified");
        const auto expected=originalActorTriangleWeights_57c98c(independent);
        const auto bits=[](float value){return std::bit_cast<std::uint32_t>(value);};
        ck(bits(weights.firstDraw)==bits(expected.firstDraw)&&
           bits(weights.secondDraw)==bits(expected.secondDraw)&&
           bits(weights.remainder)==bits(expected.remainder),
           "Original six-sample RNG stream differs from isolated source");
        for(unsigned i=0;i<3;++i)
           ck(bits(weights.originalWeightedCoefficients[i])==
              bits(expected.originalWeightedCoefficients[i]),
              "Original source fallback weighting changed");
        const std::array<float,6> scores{1.0f,3.0f,3.0f,
            std::numeric_limits<float>::quiet_NaN(),5.0f,4.0f};
        const float score=initialScoreNaN&&trial==0?
            std::numeric_limits<float>::quiet_NaN():scores[trial];
        ++sourceCalls;
        return {{score,0,0,0},{1,0,0,0},std::uint64_t(0x1000+trial)};
    }
};
static void oneOriginalPolygon(const OriginalSpatialRecord70& polygon,std::uint32_t seed) {
    std::array<std::byte,16> originalInput{};
    for(unsigned i=0;i<16;++i)originalInput[i]=std::byte(i*7);
    std::array<std::byte,16> controllerVector{};
    for(unsigned i=0;i<16;++i)controllerVector[i]=std::byte(i*11+3);
    const std::uint32_t sourceQ=(seed&1)?0xbfc90fdau:0x3fc90fdau;
    OriginalGameRandom_36f3c0 sourceRNG(seed);
    OriginalTransformCheck geometry(seed);
    geometry.expectedOwnerA1=originalInput;
    geometry.expectedControllerA2=controllerVector;
    geometry.expectedVU0Q=sourceQ;
    for(unsigned i=0;i<3;++i)
        geometry.expectedVertices[i]=originalSpatialVector_21b458(polygon,i);
    OriginalActorFallbackVu0R_57ca68 vuRandom{0xdeadbeefu};
    const auto result=originalActorFallbackSelect_57ca68(
        polygon,originalInput,controllerVector,sourceQ,sourceRNG,geometry,&vuRandom);
    ck(vuRandom.rawR==0x3f800000u &&
       vuRandom.sourceVRINITCalls==12 && vuRandom.sourceVRXORCalls==12,
       "Actual source VU0 VI[R] final state or special-op count changed");
    ck(result.sourceCandidatesEvaluated==6&&geometry.sourceCalls==6,
       "Original 0x57CA68 includes initial and five loop candidates");
    ck(result.sourceWinningCandidate==4&&result.originalBestXYZDot==5.0f&&
       result.originalWinningPackedTargetLow64==0x1004,
       "Original strict C.OLT source selects higher score not tie/NaN");
    ck(sourceRNG.originalCursor()==geometry.independent.originalCursor(),
       "Original 0x57CA68 RNG cursor differs from six consecutive samples");
    ck(sourceRNG.originalCursor()==11,
       "Original source RNG must consume twelve random floats");
    OriginalGameRandom_36f3c0 na(seed);
    OriginalTransformCheck geoNaN(seed);
    geoNaN.initialScoreNaN=true;
    geoNaN.expectedVertices=geometry.expectedVertices;
    geoNaN.expectedOwnerA1=originalInput;
    geoNaN.expectedControllerA2=controllerVector;
    geoNaN.expectedVU0Q=sourceQ;
    const auto nan=originalActorFallbackSelect_57ca68(
       polygon,originalInput,controllerVector,sourceQ,na,geoNaN);
    ck(nan.sourceWinningCandidate==0&&
       nan.originalWinningPackedTargetLow64==0x1000,
       "Original C.OLT must not replace NaN first-best score");
}
int main(int argc,char**argv){
    ck(argc==1||argc==2,"Optional owner-private original woods_c/navigation.pck");
    OriginalSpatialRecord70 polygon{};
    for(unsigned i=0;i<polygon.originalFields.size();++i)
       polygon.originalFields[i]=std::byte(i*5);
    for(std::uint32_t seed:{0u,1u,5489u,0xffffffffu})
       oneOriginalPolygon(polygon,seed);
    unsigned genuinePolygons=0,originalLinks=0,sourceSpecial=0;
    if(argc==2){
       std::ifstream input(argv[1],std::ios::binary);
       ck(bool(input),"Owner original woods_c PCK missing");
       const std::vector<char> chars((std::istreambuf_iterator<char>(input)),{});
       ck(chars.size()==3328,"Original woods_c PCK length");
       std::vector<std::byte> bytes;
       for(unsigned char c:chars)bytes.push_back(std::byte(c));
       for(unsigned g=0;g<2;++g){
          const auto nav=std::span<const std::byte>(bytes)
              .subspan(g?2224:96,g?1104:2128);
          const auto count=u32(nav,0x24),descs=u32(nav,0x3c),vecs=u32(nav,0x40);
          ck(count==(g?20u:39u),"Original graph count");
          for(unsigned polygonId=0;polygonId<count;++polygonId){
             const auto record=descs+0x1e*polygonId;
             const auto degree=u16(nav,record+2);
             ck(degree<=3&&u16(nav,record+0x16)==3,
                "Original source spatial triangle and degree");
             for(unsigned edge=0;edge<degree;++edge){
                ck(u16(nav,record+4+2*edge)<count,
                   "Original graph neighbor outside stage");
                sourceSpecial+=(u16(nav,record+0x0a+2*edge)==2);
                ++originalLinks;
             }
             OriginalSpatialRecord70 original{};
             for(unsigned vertex=0;vertex<3;++vertex){
                const auto off=vecs+16*u16(nav,record+0x18+2*vertex);
                ck(off+16<=nav.size(),"Original PCK polygon vertex bytes");
                std::memcpy(original.originalFields.data()+0x40+16*vertex,
                            nav.data()+off,16);
             }
             oneOriginalPolygon(original,5489u+polygonId+g*100);
             ++genuinePolygons;
          }
       }
       ck(genuinePolygons==59&&originalLinks==137&&sourceSpecial==15,
          "Original all woods_c polygon/edge counts");
    }
}
