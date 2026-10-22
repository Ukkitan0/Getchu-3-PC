#include "ae3/original_stage_polygon_raycast.hpp"
#include <array>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void require(bool condition,const char* msg){
    if(!condition)throw std::runtime_error(msg);
}
static OriginalVU0Vec4 sourceVertex(const OriginalSpatialRecord70& rec,unsigned slot){
    require(slot<3,"Original source vertex slot invalid");
    OriginalVU0Vec4 out{};
    std::memcpy(&out,rec.originalFields.data()+0x40+slot*16,16);
    return out;
}
int main(int argc,char**argv){
    require(argc==1||argc==2,"Optional authentic original woods_c/navigation.pck");
    unsigned decoded=0,edges=0,typeTwo=0,projected=0;
    if(argc==2){
        std::ifstream f(argv[1],std::ios::binary);
        require(bool(f),"Private original woods_c navigation.pck missing");
        const std::vector<char> chars((std::istreambuf_iterator<char>(f)),{});
        require(chars.size()==3328,"Unexpected genuine woods_c PCK bytes");
        std::vector<std::byte> bytes;
        for(unsigned char c:chars)bytes.push_back(std::byte(c));
        for(unsigned graph=0;graph<2;++graph){
            const auto content=std::span<const std::byte>(bytes)
                .subspan(graph?2224:96,graph?1104:2128);
            // Same authentic original stage asset passed to the existing
            // recovered 0x21D46C loader, not a synthesized record array.
            const OriginalStageSpatialSource source{
                0x14000000u,content,0x15000000u};
            auto stage=originalStageBuildSourceSpatial_21d46c(source);
            const auto headers=originalStagePolygonLinks_21bd3c(source,stage);
            require(headers.size()==(graph?20u:39u),
                    "Source stage descriptor count not preserved");
            require(stage.records.size()==headers.size(),
                    "Source original 0x70 record count differs from header");
            for(std::size_t i=0;i<headers.size();++i){
                const auto& h=headers[i];
                require(h.edgeCount<=3,"Original polygon edge degree");
                for(unsigned j=0;j<h.edgeCount;++j){
                    require(h.edges[j].neighborSourceIndex<headers.size(),
                            "Original stage neighbor source index invalid");
                    edges++;
                    typeTwo+=(h.edges[j].originalActionKind==2);
                }
                const auto a=sourceVertex(stage.records[i],0);
                const auto b=sourceVertex(stage.records[i],1);
                const auto c=sourceVertex(stage.records[i],2);
                const OriginalVU0Vec4 center{
                    (a.x+b.x+c.x)/3,(a.y+b.y+c.y)/3,
                    (a.z+b.z+c.z)/3,1};
                const OriginalVU0Vec4 destination{
                    center.x+(a.x-center.x)*.4f,
                    center.y+(a.y-center.y)*.4f,
                    center.z+(a.z-center.z)*.4f,1};
                const auto ray=originalStagePolygonRaycast_4c1b3c(
                    source,stage,static_cast<std::uint32_t>(i),center,destination);
                require(ray.originalTraversal.sourceHitPoint,
                        "Original 0x21D46C stage -> original raycast did not find interior point");
                require(ray.originalTraversal.originalLastAcceptedPolygon==&stage.records[i],
                        "Original stage allocated record identity changed");
                require(std::fabs(ray.originalTraversal.sourceHitPoint->x-destination.x)<3e-4f &&
                        std::fabs(ray.originalTraversal.sourceHitPoint->z-destination.z)<3e-4f,
                        "Original source world ray endpoint differs");
                ++projected;
            }
            decoded+=static_cast<unsigned>(headers.size());
        }
        require(decoded==59&&projected==59&&edges==137&&typeTwo==15,
                "Original all stage polygons, directed edges and actor type-two actions");
    }
}
