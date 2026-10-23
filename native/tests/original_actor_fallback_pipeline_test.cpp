#include "ae3/original_actor_navigation_pipeline.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void require(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
struct Missing final:OriginalActorUntranslatedMath_57c7cc {
    unsigned wrongFallback=0;
    const OriginalSpatialRecord70* call_original_4c1b3c_projection(
        const OriginalSpatialRecord70&,const OriginalVU0Vec4&) override {
        throw std::runtime_error("No projection call in source two-record initial selection");
    }
    std::uint64_t call_original_57ca68_fallback(const OriginalSpatialRecord70&) override {
        ++wrongFallback;
        throw std::runtime_error("Recovered original fallback chooser was not used");
    }
    std::uint64_t call_original_57c98c_random_vertex_transform(
        const OriginalSpatialRecord70&,
        const std::array<std::array<std::byte,16>,3>&) override {
        throw std::runtime_error("No source variant-zero triangle callback");
    }
};
struct Candidate:OriginalActorFallbackVU0Transform_57ca68 {
    unsigned calls=0;
    std::array<std::array<std::byte,16>,3> expected{};
    OriginalActorFallbackCandidate_57ca68 call_original_57ca68_candidate_VU0(
       const std::array<std::array<std::byte,16>,3>& bytes,
       const std::array<std::byte,16>& originalOwnerA1,
       const std::array<std::byte,16>& originalControllerA2,
       std::uint32_t originalQ,
       const OriginalActorTriangleWeights_57c98c&,std::size_t index) override {
        require(originalQ==0x3fc90fdau,"Original VU0 Q not forwarded into fallback candidate");
        require(originalOwnerA1[0]==std::byte(0xab),
            "Original fallback $a1 not actor owner's +0x14->+0x40 vector");
        require(originalControllerA2[0]==std::byte(0),
            "Original fallback $a2 not controller+0x20 source");
        require(bytes==expected,"Actual original source vertex bytes changed");
        require(index==calls++,"Original six candidate order");
        const float score= index==4?25.0f:float(index);
        return {{score,0,0,0},{1,0,0,0},std::uint64_t(0x1200+index)};
    }
};
static void runPolygon(const OriginalSpatialRecord70& original,
                       std::span<const OriginalVU0Vec4> sourcePoints) {
    std::array<OriginalSpatialRecord70,1> records{original};
    const std::array<OriginalSpatialSourcePolygon,1> geometry{{{sourcePoints}}};
    const std::array<std::uint32_t,2> route{0,0};
    OriginalActorRouteFollowerState_57c7cc state{records,route,&records[0],nullptr,true};
    std::array<std::byte,16> veryFarOutside{};
    const OriginalVU0Vec4 query{1.0e8f,0,1.0e8f,1};
    std::memcpy(veryFarOutside.data(),&query,16);
    Missing missing;
    OriginalGameRandom_36f3c0 random(5489);
    Candidate candidate;
    for(unsigned i=0;i<3;++i)
        candidate.expected[i]=originalSpatialVector_21b458(original,i);
    OriginalActorNavigationPipeline_57c7cc pipeline(
        records,geometry,veryFarOutside,true,missing,&random,{},&candidate);
    // Distinct source arguments must be explicitly supplied. No fallback
    // may silently substitute controller+0x20 for the actor owner vector.
    bool missingOwnerRejected=false;
    try{(void)pipeline.advance(state);}
    catch(const std::invalid_argument&){missingOwnerRejected=true;}
    require(missingOwnerRejected,"Missing original owner+0x14->+0x40 vector was silently substituted");
    std::array<std::byte,16> ownerVector{};
    ownerVector[0]=std::byte(0xab);
    pipeline.setOriginalFallbackOwnerVector_57ca68(ownerVector);
    bool missingQRejected=false;
    try{(void)pipeline.advance(state);}
    catch(const std::invalid_argument&){missingQRejected=true;}
    require(missingQRejected,"Original VU0 Q was silently guessed or defaulted");
    pipeline.setOriginalFallbackQBits_57ca68(0x3fc90fdau);
    const auto result=pipeline.advance(state);
    require(result.sourceTargetsApplied==1&&result.sourceProjectionsPerformed==0,
            "Original route[1] initial target must bypass projection");
    require(state.activePolygonAtController50==&records[0],
            "Original stage record target identity changed");
    require(candidate.calls==6&&missing.wrongFallback==0,
            "Source six-candidate native fallback was not used");
    require(random.originalCursor()==11,
            "Source 0x57CA68 must consume twelve RNG float draws");
    require(pipeline.sourceTargetResults().size()==1,
            "Original source target result missing");
    const auto& output=pipeline.sourceTargetResults().front();
    require(output.branch==OriginalActorTargetBranch_57c8ec::OriginalFallbackGeometry_57ca68 &&
            output.originalVectorLow64==0x1204,
            "Native original fallback winner did not reach actor target");

    // Connected source-instruction fallback, with no test scoring callback.
    // The retail PCPYUD result occupies the upper GPR half. Passing only
    // the previous low 64 bits would have silently dropped the target.
    OriginalGameRandom_36f3c0 genuineRandom(5489);
    OriginalActorNavigationPipeline_57c7cc originalNative(
        records,geometry,veryFarOutside,true,missing,&genuineRandom);
    originalNative.setOriginalFallbackOwnerVector_57ca68(ownerVector);
    originalNative.setOriginalFallbackQBits_57ca68(0x3fc90fdau);
    OriginalActorFallbackVu0R_57ca68 originalR{0xdeadbeefu};
    originalNative.setOriginalFallbackRState_57ca68(&originalR);
    OriginalActorRouteFollowerState_57c7cc nativeState{
        records,route,&records[0],nullptr,true};
    const auto nativeResult=originalNative.advance(nativeState);
    require(nativeResult.sourceTargetsApplied==1,
            "Native original fixed-instruction candidate not dispatched");
    const auto& nativeTarget=originalNative.sourceTargetResults().front();
    require(nativeTarget.branch==
                OriginalActorTargetBranch_57c8ec::OriginalFallbackGeometry_57ca68 &&
            nativeTarget.originalVectorLow64==0 &&
            nativeTarget.originalVectorHigh64!=0,
            "Original PCPYUD winner high-half target lost in movement pipeline");
    require(genuineRandom.originalCursor()==11,
            "Native VU0 six-candidate path must consume 12 genuine RNG draws");
    require(originalR.rawR==0x3f800000u &&
            originalR.sourceVRINITCalls==12 &&
            originalR.sourceVRXORCalls==12,
            "Connected native VU0 R source effects lost during actor advance");
    require(&originalNative.sourceFallbackRState_57ca68()==&originalR,
            "Connected VU0 R state did not alias the owner-provided global");
    // Independently test no explicit R binding: retail VRINIT/VRXOR
    // still execute and update the native pipeline's R register.
    OriginalGameRandom_36f3c0 unboundRandom(5489);
    OriginalActorNavigationPipeline_57c7cc unbound(
        records,geometry,veryFarOutside,true,missing,&unboundRandom);
    unbound.setOriginalFallbackOwnerVector_57ca68(ownerVector);
    unbound.setOriginalFallbackQBits_57ca68(0x3fc90fdau);
    OriginalActorRouteFollowerState_57c7cc unboundState{
        records,route,&records[0],nullptr,true};
    (void)unbound.advance(unboundState);
    const auto& retained=unbound.sourceFallbackRState_57ca68();
    require(retained.rawR==0x3f800000u &&
            retained.sourceVRINITCalls==12 &&
            retained.sourceVRXORCalls==12,
            "Native actor fallback skipped original R instructions when not externally bound");
}
static unsigned u16(std::span<const std::byte> b,std::size_t at){
    require(at+2<=b.size(),"Original source u16 bounds");
    return std::to_integer<unsigned>(b[at])|
        (std::to_integer<unsigned>(b[at+1])<<8);
}
static unsigned u32(std::span<const std::byte> b,std::size_t at){
    return u16(b,at)|(u16(b,at+2)<<16);
}
int main(int argc,char**argv){
    require(argc==1||argc==2,"Owner original navigation.pck optional");
    OriginalSpatialRecord70 source{};
    const std::array<OriginalVU0Vec4,3> points{{
       {0,0,0,1},{0,0,2,1},{2,0,0,1}
    }};
    for(unsigned i=0;i<3;++i)
        std::memcpy(source.originalFields.data()+0x40+16*i,&points[i],16);
    runPolygon(source,points);

    unsigned polygons=0,links=0,special=0;
    if(argc==2){
       std::ifstream f(argv[1],std::ios::binary);
       require(bool(f),"Original genuine PCK missing");
       const std::vector<char> raw((std::istreambuf_iterator<char>(f)),{});
       require(raw.size()==3328,"Original PCK size differs");
       std::vector<std::byte> nav;
       for(unsigned char c:raw)nav.push_back(std::byte(c));
       for(unsigned graph=0;graph<2;++graph){
          auto g=std::span<const std::byte>(nav)
              .subspan(graph?2224:96,graph?1104:2128);
          const auto count=u32(g,0x24),desc=u32(g,0x3c),vertices=u32(g,0x40);
          require(count==(graph?20u:39u),"Source polygon count differs");
          for(unsigned i=0;i<count;++i){
             const auto at=desc+30*i,degree=u16(g,at+2);
             require(degree<=3&&u16(g,at+0x16)==3,"Source polygon degree/vertex count");
             OriginalSpatialRecord70 polygon{};
             std::array<OriginalVU0Vec4,3> pointsFromOwner{};
             for(unsigned j=0;j<3;++j){
                 const auto off=vertices+16*u16(g,at+0x18+2*j);
                 require(off+16<=g.size(),"Source original PCK vertex range");
                 std::memcpy(&pointsFromOwner[j],g.data()+off,16);
                 std::memcpy(polygon.originalFields.data()+0x40+16*j,g.data()+off,16);
             }
             for(unsigned j=0;j<degree;++j){
                const auto dest=u16(g,at+4+2*j),kind=u16(g,at+10+2*j);
                require(dest<count&&kind<=2,"Original neighbor/action metadata");
                links++;special+=(kind==2);
             }
             runPolygon(polygon,pointsFromOwner);
             ++polygons;
          }
       }
       require(polygons==59&&links==137&&special==15,
             "All 59 original source polygons and 137 original links");
    }
}
