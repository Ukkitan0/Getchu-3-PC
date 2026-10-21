#include "ae3/original_actor_navigation_pipeline.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <span>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool ok,const char* message){
    if(!ok)throw std::runtime_error(message);
}
class Untranslated final:public OriginalActorUntranslatedMath_57c7cc {
public:
    const OriginalSpatialRecord70* projection=nullptr;
    unsigned projections=0,fallBacks=0,vertexTransforms=0;
    const OriginalSpatialRecord70* call_original_4c1b3c_projection(
        const OriginalSpatialRecord70&,const OriginalVU0Vec4&) override {
        ++projections;return projection;
    }
    std::uint64_t call_original_57ca68_fallback(const OriginalSpatialRecord70&) override {
        ++fallBacks;return 0x8877665544332211ULL;
    }
    std::uint64_t call_original_57c98c_random_vertex_transform(
        const OriginalSpatialRecord70&,
        const std::array<std::array<std::byte,16>,3>&) override {
        ++vertexTransforms;return 0x1122334455667788ULL;
    }
};
static unsigned u16(std::span<const std::byte> b,std::size_t at){
    ck(at+2<=b.size(),"Original PCK halfword out of bounds");
    return std::to_integer<unsigned>(b[at])|
        (std::to_integer<unsigned>(b[at+1])<<8);
}
static unsigned u32(std::span<const std::byte> b,std::size_t at){
    return u16(b,at)|(u16(b,at+2)<<16);
}
int main(int argc,char**argv){
    ck(argc==1||argc==2,"Optional owner-private woods_c/navigation.pck");
    std::array<OriginalSpatialRecord70,3> records{};
    std::array<std::array<OriginalVU0Vec4,3>,3> points{};
    std::array<OriginalSpatialSourcePolygon,3> polygons{};
    for(unsigned i=0;i<3;++i){
        points[i][0]={0,0,0,1};points[i][1]={0,0,0,1};
        points[i][2]={0,0,0,1};polygons[i]={points[i]};
    }
    std::array<std::byte,16> controller20{};
    const OriginalVU0Vec4 inside{0,0,0,1};
    std::memcpy(controller20.data(),&inside,16);
    Untranslated callbacks;
    OriginalActorNavigationPipeline_57c7cc pipeline(records,polygons,controller20,true,callbacks);
    const std::array<std::uint32_t,3> route{0,1,2};
    OriginalActorRouteFollowerState_57c7cc actor{records,route,&records[0],nullptr,true};
    callbacks.projection=&records[2];
    auto first=pipeline.advance(actor);
    ck(first.sourceTargetsApplied==2&&first.sourceProjectionsPerformed==1,
       "Original two-route target application and next polygon projection");
    ck(actor.activePolygonAtController50==&records[2],
       "Real original target must advance to projected matching polygon");
    ck(pipeline.sourceTargetResults().size()==2&&callbacks.fallBacks==0,
       "Fully recovered original eligibility should bypass pending geometry");
    ck(pipeline.sourceTargetResults()[0].branch==
       OriginalActorTargetBranch_57c8ec::OriginalVariantVectorPassThrough_57c944,
       "Connected stored-vector fast path");
    for(unsigned i=0;i<3;++i){
        points[i][0]={0,0,0,1};points[i][1]={0,1,0,1};
        points[i][2]={0,0,1,1};
    }
    const OriginalVU0Vec4 outside{100,0,0,1};
    std::memcpy(controller20.data(),&outside,16);
    OriginalActorNavigationPipeline_57c7cc outsidePipe(records,polygons,controller20,true,callbacks);
    actor.activePolygonAtController50=nullptr;
    auto second=outsidePipe.advance(actor);
    ck(second.sourceTargetsApplied==2&&callbacks.fallBacks==2,
       "Real recovered polygon eligibility must enter original fallback");
    OriginalActorNavigationPipeline_57c7cc other(records,polygons,controller20,false,callbacks);
    actor.activePolygonAtController50=nullptr;
    auto third=other.advance(actor);
    ck(third.sourceTargetsApplied==2&&callbacks.vertexTransforms==2,
       "Original variant-zero target vectors still require original transform");
    actor.originalStagePolygonAtActor30=nullptr;
    bool rejected=false;
    try{(void)other.advance(actor);}
    catch(const std::invalid_argument&){rejected=true;}
    ck(rejected,"Missing source actor owner must fail safely");

    if(argc==2){
        std::ifstream f(argv[1],std::ios::binary);
        ck(bool(f),"Original retail navigation.pck unavailable");
        const std::vector<char> chars((std::istreambuf_iterator<char>(f)),{});
        ck(chars.size()==3328,"Original retail navigation PCK has unexpected size");
        std::vector<std::byte> originalBytes;
        for(unsigned char c:chars)originalBytes.push_back(std::byte(c));
        unsigned edges=0,type2=0,eligibilityPasses=0,eligibilityFails=0;
        for(unsigned graph=0;graph<2;++graph){
            auto raw=std::span<const std::byte>(originalBytes)
                .subspan(graph?2224:96,graph?1104:2128);
            const auto count=u32(raw,0x24),desc=u32(raw,0x3c),vertices=u32(raw,0x40);
            ck(count==(graph?20u:39u),"Original polygon count mismatch");
            std::vector<OriginalSpatialRecord70> realRecords(count);
            std::vector<std::array<OriginalVU0Vec4,3>> originalVertices(count);
            std::vector<OriginalSpatialSourcePolygon> sourcePolygons(count);
            for(unsigned i=0;i<count;++i){
                const auto at=desc+0x1e*i;
                const auto numVerts=u16(raw,at+0x16);
                ck(numVerts>0&&numVerts<=3,"Original vertex count must fit 3");
                for(unsigned v=0;v<numVerts;++v){
                    const auto ptr=vertices+16*u16(raw,at+0x18+2*v);
                    ck(ptr+16<=raw.size(),"Original vertex pointer out of PCK bounds");
                    std::memcpy(&originalVertices[i][v],raw.data()+ptr,16);
                    std::memcpy(realRecords[i].originalFields.data()+0x40+16*v,
                                raw.data()+ptr,16);
                }
                sourcePolygons[i]={
                    std::span<const OriginalVU0Vec4>(originalVertices[i].data(),numVerts)};
            }
            for(unsigned i=0;i<count;++i){
                const auto at=desc+0x1e*i,degree=u16(raw,at+2);
                ck(degree<=3,"Original source edge degree mismatch");
                for(unsigned e=0;e<degree;++e){
                    const unsigned dest=u16(raw,at+4+2*e);
                    ck(dest<count,"Original edge destination mismatch");
                    type2+=(u16(raw,at+0x0a+2*e)==2);
                    OriginalVU0Vec4 query=originalVertices[dest][0];
                    std::array<std::byte,16> sourceController20{};
                    std::memcpy(sourceController20.data(),&query,16);
                    Untranslated pending;
                    OriginalActorNavigationPipeline_57c7cc direct(
                        realRecords,sourcePolygons,sourceController20,true,pending);
                    const std::array<std::uint32_t,2> edge{i,dest};
                    OriginalActorRouteFollowerState_57c7cc stageActor{
                        realRecords,edge,&realRecords[i],nullptr,true};
                    auto result=direct.advance(stageActor);
                    ck(result.sourceTargetsApplied==1&&result.sourceProjectionsPerformed==0,
                       "Original two-polygon route should apply target once");
                    ck(stageActor.activePolygonAtController50==&realRecords[dest],
                       "Source stage target identity mismatch");
                    ck(direct.sourceTargetResults().size()==1,"Missing source target vector result");
                    const bool originalPredicate=originalVU0PolygonEligible_21b788(
                        sourcePolygons[dest].sourcePoints,query,0.1f);
                    const auto expected=originalPredicate?
                        OriginalActorTargetBranch_57c8ec::OriginalVariantVectorPassThrough_57c944:
                        OriginalActorTargetBranch_57c8ec::OriginalFallbackGeometry_57ca68;
                    ck(direct.sourceTargetResults()[0].branch==expected,
                       "Native integrated game predicate differs from original geometry path");
                    if(originalPredicate){
                        ++eligibilityPasses;
                        ck(pending.fallBacks==0,"Unexpected unrecovered geometry callback");
                    }else{
                        ++eligibilityFails;
                        ck(pending.fallBacks==1,"Expected explicit unrecovered geometry callback");
                    }
                    ++edges;
                }
            }
        }
        ck(edges==137&&type2==15&&eligibilityPasses+eligibilityFails==137,
           "Both real woods_c navigation graphs and 15 source special links");
    }
}
