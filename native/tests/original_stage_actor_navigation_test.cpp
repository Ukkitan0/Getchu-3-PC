#include "ae3/original_stage_actor_navigation.hpp"
#include <array>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
class Pending final:public OriginalActorUntranslatedMath_57c7cc {
public:
    unsigned oldProjection=0,oldTriangle=0,fallback=0;
    const OriginalSpatialRecord70* call_original_4c1b3c_projection(
        const OriginalSpatialRecord70&,const OriginalVU0Vec4&) override {
        ++oldProjection;
        throw std::runtime_error("Real original raycast incorrectly bypassed");
    }
    std::uint64_t call_original_57ca68_fallback(
        const OriginalSpatialRecord70&) override {
        ++fallback;
        throw std::runtime_error("Unexpected unrecovered 0x57CA68");
    }
    std::uint64_t call_original_57c98c_random_vertex_transform(
        const OriginalSpatialRecord70&,
        const std::array<std::array<std::byte,16>,3>&) override {
        ++oldTriangle;
        throw std::runtime_error("Original shared RNG triangle unexpectedly bypassed");
    }
};
static OriginalVU0Vec4 sourceVertex(const OriginalSpatialRecord70& rec,unsigned slot) {
    ck(slot<3,"Original source vertex slot");
    OriginalVU0Vec4 out{};
    std::memcpy(&out,rec.originalFields.data()+0x40+slot*16,16);
    return out;
}
static OriginalVU0Vec4 blend(const OriginalVU0Vec4& a,const OriginalVU0Vec4& b) {
    return {a.x+(b.x-a.x)*.4f,a.y+(b.y-a.y)*.4f,
            a.z+(b.z-a.z)*.4f,a.w+(b.w-a.w)*.4f};
}
int main(int argc,char**argv) {
    ck(argc==1||argc==2,"Owner private woods_c/navigation.pck is optional");
    // A small structurally authentic source-header fixture makes the
    // connected native test run even without an owner-private PCK path.
    // It is a regression test, never used to replace original game assets.
    {
        std::array<std::byte,512> raw{};
        const auto store16=[&](unsigned at,unsigned value) {
            raw.at(at)=std::byte(value&255);
            raw.at(at+1)=std::byte((value>>8)&255);
        };
        const auto store32=[&](unsigned at,unsigned value) {
            store16(at,value&65535);
            store16(at+2,value>>16);
        };
        store32(0x24,2);
        const std::array<unsigned,6> offsets{96,128,256,400,416,448};
        for(unsigned i=0;i<6;++i)store32(0x38+4*i,offsets[i]);
        const std::array<std::array<OriginalVU0Vec4,3>,2> triangles{{
            {{{0,0,0,0},{0,0,2,0},{2,0,0,0}}},
            {{{2,0,0,0},{0,0,2,0},{2,0,2,0}}}
        }};
        for(unsigned i=0;i<2;++i){
            const unsigned descriptor=128+30*i;
            store16(descriptor,0xffff);
            store16(descriptor+2,i==0?1:0);
            if(i==0){store16(descriptor+4,1);store16(descriptor+0xa,0);}
            store16(descriptor+0x16,3);
            for(unsigned v=0;v<3;++v){
                store16(descriptor+0x18+2*v,i*3+v);
                std::memcpy(raw.data()+256+16*(i*3+v),&triangles[i][v],16);
            }
        }
        OriginalStageSpatialSource source{0x14000000,raw,0x15000000};
        const auto stage=originalStageBuildSourceSpatial_21d46c(source);
        Pending missing;
        OriginalGameRandom_36f3c0 rng(5489);
        std::array<std::byte,16> controller20{};
        OriginalStageActorNavigation_57c7cc actor(source,stage,controller20,false,missing,&rng);
        ck(actor.originalSourceAdjacency()[0].edges[0].neighborSourceIndex==1,
            "Source actor synthetic layout lost native original graph owner");
        const std::array<std::uint32_t,2> route{0,1};
        actor.setOriginalMovementVectors_57c7cc(
            {.5f,0,.5f,1},{1.7f,0,1.7f,1});
        actor.setOriginalTargetAt50(&stage.records[0]);
        const auto changed=actor.advance(route,0,true);
        ck(!changed.returnedEarlyBecauseCurrentTargetMatched&&changed.sourceTargetsApplied==1,
            "Real original source ray should select the neighboring route polygon");
        ck(actor.currentOriginalTargetAt50()==&stage.records[1]&&
            missing.oldProjection==0&&missing.oldTriangle==0,
            "Original native stage pipeline used synthetic gameplay callbacks");
        const auto unchanged=actor.advance(route,0,true);
        ck(unchanged.returnedEarlyBecauseCurrentTargetMatched&&unchanged.sourceTargetsApplied==0,
            "Original projected current target should skip another RNG update");
    }

    unsigned originalPolygons=0,links=0,type2=0,raycasts=0,ranRng=0;
    if(argc==2){
        std::ifstream file(argv[1],std::ios::binary);
        ck(bool(file),"Original woods_c navigation PCK missing");
        const std::vector<char> content((std::istreambuf_iterator<char>(file)),{});
        ck(content.size()==3328,"Original retail woods_c source bytes changed");
        std::vector<std::byte> data;
        for(unsigned char c:content)data.push_back(std::byte(c));
        for(unsigned graph=0;graph<2;++graph){
            const auto originalBytes=std::span<const std::byte>(data)
                .subspan(graph?2224:96,graph?1104:2128);
            const OriginalStageSpatialSource source{0x14000000,originalBytes,0x15000000};
            const auto stage=originalStageBuildSourceSpatial_21d46c(source);
            ck(stage.records.size()==(graph?20u:39u),"Original source record count");
            Pending missing;
            OriginalGameRandom_36f3c0 sharedRng(5489);
            std::array<std::byte,16> vector20{};
            OriginalStageActorNavigation_57c7cc actor(
                source,stage,vector20,false,missing,&sharedRng);
            const auto adjacency=actor.originalSourceAdjacency();
            ck(adjacency.size()==stage.records.size(),"Original source graph binding");
            for(std::uint32_t i=0;i<stage.records.size();++i){
                const auto& linksForPolygon=adjacency[i];
                links+=linksForPolygon.edgeCount;
                for(unsigned j=0;j<linksForPolygon.edgeCount;++j){
                    ck(linksForPolygon.edges[j].neighborSourceIndex<stage.records.size(),
                       "Original source adjacency neighbor");
                    type2+=(linksForPolygon.edges[j].originalActionKind==2);
                }
                const auto a=sourceVertex(stage.records[i],0);
                const auto b=sourceVertex(stage.records[i],1);
                const auto c=sourceVertex(stage.records[i],2);
                const OriginalVU0Vec4 center{
                    (a.x+b.x+c.x)/3,(a.y+b.y+c.y)/3,
                    (a.z+b.z+c.z)/3,1};
                const auto dest=blend(center,a);
                actor.setOriginalMovementVectors_57c7cc(center,dest);
                actor.setOriginalTargetAt50(&stage.records[i]);
                const std::array<std::uint32_t,2> route{i,i};
                const auto before=sharedRng.originalCursor();
                const auto early=actor.advance(route,i,true);
                ck(early.returnedEarlyBecauseCurrentTargetMatched,
                   "Original 0x57C814 projection must find source current polygon");
                ck(early.sourceProjectionsPerformed==1&&early.sourceTargetsApplied==0,
                   "Matching stage projection must bypass target recomputation");
                ck(actor.currentOriginalTargetAt50()==&stage.records[i],
                   "Stage-owned original target pointer changed");
                ck(sharedRng.originalCursor()==before,
                   "No actor target RNG draw on original projection match");
                ++raycasts;

                // Mismatch requires a route target update and uses the
                // real already recovered original game RNG, not a callback.
                if(stage.records.size()>1){
                    actor.setOriginalTargetAt50(&stage.records[(i+1)%stage.records.size()]);
                    const auto consumed=actor.advance(route,i,true);
                    ck(consumed.sourceTargetsApplied==1 &&
                       actor.currentOriginalTargetAt50()==&stage.records[i],
                       "Original mismatch must select route[1] from stage");
                    ck(!actor.originalTargetResults().empty(),
                       "Original 0x57C8EC target dispatch omitted");
                    ++ranRng;
                }
                ++originalPolygons;
            }
            ck(missing.oldProjection==0&&missing.oldTriangle==0&&missing.fallback==0,
               "Native actor pipeline used a fallback callback instead of recovered game routines");
        }
        ck(originalPolygons==59&&links==137&&type2==15&&raycasts==59&&ranRng==59,
           "All original 59 polygons, 137 links, 15 type-two edges and native actor updates");
    }
}
