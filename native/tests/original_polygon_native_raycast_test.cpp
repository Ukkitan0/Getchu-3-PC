#include "ae3/original_polygon_edge_clip.hpp"
#include <array>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void check(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
static unsigned u16(std::span<const std::byte> s,unsigned at){
    check(at+2<=s.size(),"Retail source halfword outside PCK");
    return std::to_integer<unsigned>(s[at])|(std::to_integer<unsigned>(s[at+1])<<8);
}
static unsigned u32(std::span<const std::byte> s,unsigned at){return u16(s,at)|(u16(s,at+2)<<16);}
static void install(OriginalSpatialRecord70& rec,const std::array<OriginalVU0Vec4,3>& vs){
    for(unsigned i=0;i<3;++i)std::memcpy(rec.originalFields.data()+0x40+16*i,&vs[i],16);
}
static OriginalVU0Vec4 lerp(const OriginalVU0Vec4& a,const OriginalVU0Vec4& b,float t){
    return {a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t,a.z+(b.z-a.z)*t,a.w+(b.w-a.w)*t};
}
int main(int argc,char**argv){
    check(argc==1||argc==2,"Optional genuine woods_c/navigation.pck required");
    OriginalPolygonNativeEdgeClip_21ee00 clip{};
    const OriginalVU0Vec4 a{0,0,0,0},b{0,0,2,0};
    OriginalPolygonRayDistanceBounds_21bd3c bounds{0,3};
    check(originalPolygonClipEdge_21ee00(a,b,{.5f,0,.5f,0},{1,0,0,0},bounds),
          "Original positive-direction clip failed");
    check(bounds.originalLowerAt20==0&&bounds.originalUpperAt24==3,
          "Original positive clip bounds");
    bounds={0,3};
    check(originalPolygonClipEdge_21ee00(a,b,{.5f,0,.5f,0},{-1,0,0,0},bounds),
          "Original negative-direction clip failed");
    check(std::fabs(bounds.originalUpperAt24-.5f)<1e-6f,
          "Original negative-denominator MIN.S bound");
    bounds={0,3};
    check(!originalPolygonClipEdge_21ee00(a,b,{-.5f,0,.5f,0},{0,0,1,0},bounds),
          "Original parallel exterior rejection failed");
    bounds={0,3};
    check(originalPolygonClipEdge_21ee00(a,b,{.5f,0,.5f,0},{0,0,1,0},bounds),
          "Original parallel interior acceptance failed");

    std::array<OriginalSpatialRecord70,2> triangles{};
    install(triangles[0],{{{0,0,0,0},{0,0,2,0},{2,0,0,0}}});
    install(triangles[1],{{{2,0,0,0},{0,0,2,0},{2,0,2,0}}});
    std::array<OriginalPolygonLinks_21bd3c,2> links{};
    links[0].edgeCount=1;links[0].edges[0]={1,0};
    const float sqrt2=std::sqrt(2.0f);
    auto crossing=originalPolygonTraverse_21bd3c(
        triangles,links,0,{.5f,0,.5f,0},{1/sqrt2,0,1/sqrt2,0},1.2f*sqrt2,clip);
    check(crossing.sourceHitPoint&&crossing.originalLastAcceptedPolygon==&triangles[1],
          "Source type-zero neighboring polygon traversal failed");
    check(std::fabs(crossing.sourceHitPoint->x-1.7f)<2e-5f &&
          std::fabs(crossing.sourceHitPoint->z-1.7f)<2e-5f,
          "Source native intersection across two real triangles returned wrong point");
    check(crossing.sourcePolygonsTested==2&&crossing.sourceZeroKindRecursions==1,
          "Original source traversal did not recurse across shared edge");
    links[0].edges[0].originalActionKind=2;
    auto prohibited=originalPolygonTraverse_21bd3c(
        triangles,links,0,{.5f,0,.5f,0},{1/sqrt2,0,1/sqrt2,0},1.2f*sqrt2,clip);
    check(!prohibited.sourceHitPoint,
          "Original traversal must reject a kind-two transition");

    unsigned polygons=0,outside=0,typeZero=0;
    if(argc==2){
        std::ifstream input(argv[1],std::ios::binary);
        check(bool(input),"Owner-private PCK unavailable");
        const std::vector<char> file((std::istreambuf_iterator<char>(input)),{});
        check(file.size()==3328,"Authentic woods_c PCK size changed");
        std::vector<std::byte> bytes;
        for(unsigned char c:file)bytes.push_back(std::byte(c));
        for(unsigned gr=0;gr<2;++gr){
            auto src=std::span<const std::byte>(bytes).subspan(gr?2224:96,gr?1104:2128);
            const unsigned count=u32(src,0x24),hdr=u32(src,0x3c),base=u32(src,0x40);
            check(count==(gr?20u:39u),"Original graph count changed");
            std::vector<OriginalSpatialRecord70> records(count);
            std::vector<OriginalPolygonLinks_21bd3c> graph(count);
            std::vector<std::array<OriginalVU0Vec4,3>> vertices(count);
            for(unsigned i=0;i<count;++i){
                const unsigned at=hdr+i*30;
                graph[i].edgeCount=u16(src,at+2);
                check(graph[i].edgeCount<=3,"Source graph degree exceeded 3");
                for(unsigned j=0;j<graph[i].edgeCount;++j){
                    const unsigned neighbor=u16(src,at+4+2*j),type=u16(src,at+10+2*j);
                    check(neighbor<count&&type<=2,"Source action type or neighbor invalid");
                    graph[i].edges[j]={neighbor,static_cast<std::uint16_t>(type)};
                    typeZero+=(type==0);
                }
                check(u16(src,at+0x16)==3,"Original woods_c vertices no longer triangular");
                for(unsigned v=0;v<3;++v){
                    const unsigned offset=base+16*u16(src,at+0x18+2*v);
                    check(offset+16<=src.size(),"Source vertex outside original PCK");
                    std::memcpy(&vertices[i][v],src.data()+offset,16);
                }
                install(records[i],vertices[i]);
            }
            for(unsigned i=0;i<count;++i){
                const auto& vs=vertices[i];
                const OriginalVU0Vec4 center{
                    (vs[0].x+vs[1].x+vs[2].x)/3,
                    (vs[0].y+vs[1].y+vs[2].y)/3,
                    (vs[0].z+vs[1].z+vs[2].z)/3,0};
                const auto dest=lerp(center,vs[0],.4f);
                const float dx=dest.x-center.x,dy=dest.y-center.y,dz=dest.z-center.z;
                const float length=std::sqrt(dx*dx+dy*dy+dz*dz);
                check(length>0,"Original polygon internal source ray length zero");
                auto hit=originalPolygonTraverse_21bd3c(records,graph,i,center,
                    {dx/length,dy/length,dz/length,0},length,clip);
                check(hit.sourceHitPoint&&hit.originalLastAcceptedPolygon==&records[i],
                      "Native original 0x21EE00 + 0x21BD3C failed interior segment");
                check(std::fabs(hit.sourceHitPoint->x-dest.x)<3e-4f &&
                      std::fabs(hit.sourceHitPoint->z-dest.z)<3e-4f,
                      "Native original source intersection changed interior endpoint");
                check(hit.sourcePolygonsTested==1&&hit.sourceEdgeTests==3,
                      "Original interior ray must test exactly three edge planes");
                for(unsigned edge=0;edge<3;++edge){
                    const auto& p=vs[(edge+2)%3];const auto& q=vs[edge];
                    const float edgeX=q.x-p.x,edgeZ=q.z-p.z;
                    const float n=std::sqrt(edgeX*edgeX+edgeZ*edgeZ);
                    check(n>0,"Original edge has zero length");
                    const OriginalVU0Vec4 ext{
                        (p.x+q.x)*.5f-edgeZ/n*2,
                        (p.y+q.y)*.5f,
                        (p.z+q.z)*.5f+edgeX/n*2,0};
                    auto miss=originalPolygonTraverse_21bd3c(records,graph,i,ext,{0,0,0,0},1,clip);
                    check(!miss.sourceHitPoint,"Original parallel exterior ray incorrectly accepted");
                    ++outside;
                }
                ++polygons;
            }
        }
        check(polygons==59&&outside==177&&typeZero>50,
              "Original 59 polygons, 177 outside edge rays and connected routes");
    }
}
