#include "ae3/original_nav_actor_binding.hpp"
#include "ae3/original_stage_actor_navigation.hpp"
#include "ae3/original_game_random.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

using namespace ae3;
static void check(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
static void put16(std::vector<std::uint8_t>& b,std::size_t p,std::uint16_t v){
    b.at(p)=static_cast<std::uint8_t>(v);
    b.at(p+1)=static_cast<std::uint8_t>(v>>8);
}
static void put32(std::vector<std::uint8_t>& b,std::size_t p,std::uint32_t v){
    for(std::size_t k=0;k<4;++k)
        b.at(p+k)=static_cast<std::uint8_t>(v>>(8*k));
}
static OriginalNavSource originalLayout() {
    OriginalNavSource source{};
    source.originalFilename="nav_monkey.nav";
    source.originalSlot=0;
    auto& raw=source.originalNavBytes;
    raw.resize(0x240);
    put32(raw,0x24,2);
    const std::array<std::uint32_t,6> offsets{0x60,0x80,0x120,0x1b0,0x1c0,0x1e0};
    for(unsigned i=0;i<offsets.size();++i)
        put32(raw,0x38+i*4,offsets[i]);
    const std::array<std::array<OriginalVU0Vec4,3>,2> triangles{{
        {{{0,0,0,0},{0,0,2,0},{2,0,0,0}}},
        {{{2,0,0,0},{0,0,2,0},{2,0,2,0}}}
    }};
    for(unsigned i=0;i<2;++i){
        const unsigned p=0x80+0x1e*i;
        put16(raw,p,0xffff);
        put16(raw,p+2,i==0?1:0);
        if(i==0) {
            put16(raw,p+4,1);
            put16(raw,p+0x0a,0);
        }
        put16(raw,p+0x16,3);
        for(unsigned j=0;j<3;++j){
            put16(raw,p+0x18+2*j,i*3+j);
            std::memcpy(raw.data()+0x120+16*(i*3+j),&triangles[i][j],16);
        }
    }
    return source;
}
class MissingSourceMath final:public OriginalActorUntranslatedMath_57c7cc {
public:
    unsigned called=0;
    const OriginalSpatialRecord70* call_original_4c1b3c_projection(
        const OriginalSpatialRecord70&,const OriginalVU0Vec4&) override {
        ++called;
        throw std::runtime_error("Expected recovered source stage ray, not fallback");
    }
    std::uint64_t call_original_57ca68_fallback(
        const OriginalSpatialRecord70&) override {
        ++called;
        throw std::runtime_error("Unexpected unresolved original fallback");
    }
    std::uint64_t call_original_57c98c_random_vertex_transform(
        const OriginalSpatialRecord70&,
        const std::array<std::array<std::byte,16>,3>&) override {
        ++called;
        throw std::runtime_error("Unexpected unresolved original triangle math");
    }
};
int main() {
    const auto input=originalLayout();
    OriginalNavStageOwners_21ed30 owners;
    owners.originalInitialize_21ed30();
    owners.originalAttach_4f0a5c(
        input,{"nav_monkey.nav",0x14000000,0x15000000});
    const auto& slot=owners.originalAt(0);
    const auto& originalGraph=*slot.decodedGraph;
    check(originalGraph.records.size()==2,
          "Original spatial source header not retained");
    const OriginalStageSpatialSource rawStage{
        slot.originalSourceVA,
        std::as_bytes(std::span(slot.originalSourceAllocation)),
        slot.originalOwnerVA
    };
    MissingSourceMath pending;
    OriginalGameRandom_36f3c0 gameRng(5489);
    std::array<std::byte,16> controller20{};
    OriginalStageActorNavigation_57c7cc originalPipeline(
        rawStage,originalGraph,controller20,false,pending,&gameRng);
    originalPipeline.setOriginalMovementVectors_57c7cc(
        {.5f,0,.5f,1},{1.7f,0,1.7f,1});
    originalPipeline.setOriginalTargetAt50(&originalGraph.records[0]);
    OriginalNavDispatchRequest q{};
    q.sourceSeedIndex=0;
    q.maximumDistance=100.f;
    q.routeMode=0;
    q.originalReference={0.f,0.f,0.f,1.f};
    const auto retailRoute=originalNavStageDispatch_2207d0(owners,0,q);
    check(retailRoute.originalForwardSourcePolygons==
          std::vector<std::uint32_t>({0,1}),
          "Original 0x220D5C failed to select connected second polygon");
    OriginalNavActorRouteBinding actor(owners);
    actor.originalUseStageRoute_2207d0(retailRoute);
    // Source-graph mismatch is rejected BEFORE the actor pipeline acts.
    OriginalStageSpatialLoaded unrelated=originalGraph;
    bool invalid=false;
    try{(void)actor.originalAdvanceNativeStage_57c7cc(originalPipeline,unrelated,0);}
    catch(const std::invalid_argument&){invalid=true;}
    check(invalid,"Original actor stage bridge accepted foreign graph identity");
    invalid=false;
    try{(void)actor.originalAdvanceNativeStage_57c7cc(originalPipeline,originalGraph,2);}
    catch(const std::out_of_range&){invalid=true;}
    check(invalid,"Original actor stage bridge accepted foreign polygon index");
    const auto advanced=actor.originalAdvanceNativeStage_57c7cc(
        originalPipeline,originalGraph,0);
    check(advanced.sourceTargetsApplied==1 &&
          originalPipeline.currentOriginalTargetAt50()==&originalGraph.records[1] &&
          actor.originalCurrentTargetAt50()==&originalGraph.records[1] &&
          pending.called==0,
          "Original routed actor failed stage VU0/0x57C8EC target update");
    const auto unchanged=actor.originalAdvanceNativeStage_57c7cc(
        originalPipeline,originalGraph,0);
    check(unchanged.returnedEarlyBecauseCurrentTargetMatched &&
          unchanged.sourceTargetsApplied==0 &&
          pending.called==0,
          "Original 0x57C7CC early return bypassed during direct stage bridge");
    // Changed route target must not overwrite the pipeline's existing
    // controller +0x50 before original 0x57C7CC considers projection.
    q.routeMode=1;
    auto nearest=originalNavStageDispatch_2207d0(owners,0,q);
    check(nearest.originalForwardSourcePolygons==
          std::vector<std::uint32_t>({0}),"Original nearest route expected root");
    actor.originalUseStageRoute_2207d0(std::move(nearest));
    const auto single=actor.originalAdvanceNativeStage_57c7cc(
        originalPipeline,originalGraph,0);
    check(single.sourceTargetsApplied==1 &&
          originalPipeline.currentOriginalTargetAt50()==&originalGraph.records[0],
          "Original single-polygon controller branch not preserved");
    owners.originalReleaseSlot_21d7d8(0);
    check(!actor.originalRouteCurrent() && actor.originalCurrentTargetAt50()==nullptr,
          "Original stage release did not invalidate bound actor target");
    invalid=false;
    try{(void)actor.originalAdvanceNativeStage_57c7cc(originalPipeline,unrelated,0);}
    catch(const std::logic_error&){invalid=true;}
    check(invalid,"Stale stage route reached original stage actor pipeline");
}
