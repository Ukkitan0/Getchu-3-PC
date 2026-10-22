#include "ae3/original_polygon_raycast.hpp"
#include <array>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool ok,const char* msg){if(!ok)throw std::runtime_error(msg);}
static unsigned u16(std::span<const std::byte> b,unsigned at){
    ck(at+2<=b.size(),"Original retail u16 outside range");
    return std::to_integer<unsigned>(b[at])|(std::to_integer<unsigned>(b[at+1])<<8);
}
static unsigned u32(std::span<const std::byte> b,unsigned at){return u16(b,at)|(u16(b,at+2)<<16);}
int main(int argc,char**argv){
    ck(argc==1||argc==2,"Optional original woods_c/navigation.pck");
    std::array<OriginalSpatialRecord70,2> sample{};
    const std::array<std::array<OriginalVU0Vec4,3>,2> triangles{{
        {{{0,0,0,0},{0,0,2,0},{2,0,0,0}}},
        {{{2,0,0,0},{0,0,2,0},{2,0,2,0}}}}};
    for(unsigned i=0;i<2;++i)for(unsigned j=0;j<3;++j)
        std::memcpy(sample[i].originalFields.data()+0x40+16*j,&triangles[i][j],16);
    std::array<OriginalPolygonLinks_21bd3c,2> links{};
    links[0].edgeCount=1;links[0].edges[0]={1,0};
    auto hit=originalPolygonRaycast_4c1b3c(sample,links,0,{.5f,0,.5f,0},{1.7f,0,1.7f,0});
    ck(hit.originalTraversal.sourceHitPoint&&
       hit.originalTraversal.originalLastAcceptedPolygon==&sample[1],
       "Original segment failed to traverse source action-zero neighbor");
    ck(std::fabs(hit.originalTraversal.sourceHitPoint->x-1.7f)<1e-5f &&
       std::fabs(hit.originalTraversal.sourceHitPoint->z-1.7f)<1e-5f,
       "Original source segment vector endpoint changed");
    links[0].edges[0].originalActionKind=2;
    auto blocked=originalPolygonRaycast_4c1b3c(sample,links,0,{.5f,0,.5f,0},{1.7f,0,1.7f,0});
    ck(!blocked.originalTraversal.sourceHitPoint,
       "Original source kind-two action must prevent traversal");
    unsigned realPolygons=0;
    if(argc==2){
        std::ifstream in(argv[1],std::ios::binary);
        ck(bool(in),"Original woods_c PCK missing");
        const std::vector<char> c((std::istreambuf_iterator<char>(in)),{});
        ck(c.size()==3328,"Original nav PCK size changed");
        std::vector<std::byte> bytes;
        for(unsigned char ch:c)bytes.push_back(std::byte(ch));
        for(unsigned g=0;g<2;++g){
            auto src=std::span<const std::byte>(bytes).subspan(g?2224:96,g?1104:2128);
            const unsigned n=u32(src,0x24),header=u32(src,0x3c),vbase=u32(src,0x40);
            ck(n==(g?20u:39u),"Retail navigation graph polygon count");
            std::vector<OriginalSpatialRecord70> records(n);
            std::vector<OriginalPolygonLinks_21bd3c> graph(n);
            std::vector<std::array<OriginalVU0Vec4,3>> vertices(n);
            for(unsigned i=0;i<n;++i){
                const unsigned p=header+30*i;
                graph[i].edgeCount=u16(src,p+2);
                ck(graph[i].edgeCount<=3,"Original polygon degree exceeds 3");
                for(unsigned j=0;j<graph[i].edgeCount;++j){
                    const unsigned dst=u16(src,p+4+2*j),type=u16(src,p+10+2*j);
                    ck(dst<n&&type<=2,"Original source navigation neighbor invalid");
                    graph[i].edges[j]={dst,static_cast<std::uint16_t>(type)};
                }
                ck(u16(src,p+0x16)==3,"Original source polygon no longer triangle");
                for(unsigned j=0;j<3;++j){
                    const unsigned at=vbase+16*u16(src,p+0x18+2*j);
                    ck(at+16<=src.size(),"Original vertex outside source PCK");
                    std::memcpy(&vertices[i][j],src.data()+at,16);
                    std::memcpy(records[i].originalFields.data()+0x40+16*j,
                                src.data()+at,16);
                }
            }
            for(unsigned i=0;i<n;++i){
                const auto& v=vertices[i];
                OriginalVU0Vec4 start{
                    (v[0].x+v[1].x+v[2].x)/3,
                    (v[0].y+v[1].y+v[2].y)/3,
                    (v[0].z+v[1].z+v[2].z)/3,0};
                OriginalVU0Vec4 finish{start.x+.4f*(v[0].x-start.x),
                    start.y+.4f*(v[0].y-start.y),
                    start.z+.4f*(v[0].z-start.z),0};
                auto ray=originalPolygonRaycast_4c1b3c(records,graph,i,start,finish);
                ck(ray.originalTraversal.sourceHitPoint &&
                   ray.originalTraversal.originalLastAcceptedPolygon==&records[i],
                   "Genuine polygon interior source raycast failed");
                ck(std::fabs(ray.originalTraversal.sourceHitPoint->x-finish.x)<3e-4f &&
                   std::fabs(ray.originalTraversal.sourceHitPoint->z-finish.z)<3e-4f,
                   "Genuine polygon source end point differs");
                ++realPolygons;
            }
        }
        ck(realPolygons==59,"All 59 owner-authentic source triangles not verified");
    }
}
