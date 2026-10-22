#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include "ae3/original_polygon_traversal.hpp"
#include <array>
#include <bit>
#include <cstring>
#include <fstream>
#include <iterator>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace ae3;
static unsigned u16(std::span<const std::byte> b,std::size_t at){
    assert(at+2<=b.size());
    return std::to_integer<unsigned>(b[at])|(std::to_integer<unsigned>(b[at+1])<<8);
}
static unsigned u32(std::span<const std::byte> b,std::size_t at){return u16(b,at)|(u16(b,at+2)<<16);}
struct EdgeResult : OriginalPolygonEdgeIntersection_21ee00 {
    unsigned calls=0,rejectAt=0;
    float consumed=0.0f;
    bool call_original_21ee00(const OriginalVU0Vec4&,const OriginalVU0Vec4&,
       const OriginalVU0Vec4&,const OriginalVU0Vec4&,
       OriginalPolygonRayDistanceBounds_21bd3c& bounds) override{
        ++calls;
        if(rejectAt==calls)return false;
        if(consumed>0.0f)bounds.originalUpperAt24=consumed;
        return true;
    }
};
int main(int argc,char**argv){
    assert(argc==1||argc==2);
    std::array<OriginalSpatialRecord70,3> records{};
    for(unsigned id=0;id<3;++id)for(unsigned k=0;k<3;++k){
        OriginalVU0Vec4 v{float(id),float(k),float(k*2),1};
        std::memcpy(records[id].originalFields.data()+0x40+16*k,&v,16);
    }
    std::array<OriginalPolygonLinks_21bd3c,3> graph{};
    graph[0].edgeCount=2;graph[0].edges[0]={1,0};graph[0].edges[1]={2,2};
    graph[1].edgeCount=1;graph[1].edges[0]={0,1};
    EdgeResult cb;cb.rejectAt=2;
    auto out=originalPolygonTraverse_21bd3c(records,graph,0,{1,3,5,1},{1,0,0,0},3,cb);
    assert(!out.sourceHitPoint&&out.sourceEdgeTests==2&&cb.calls==2);
    cb={};cb.consumed=0.5f;
    out=originalPolygonTraverse_21bd3c(records,graph,0,{1,3,5,1},{1,0,0,0},2,cb);
    assert(!out.sourceHitPoint&&out.sourcePolygonsTested==2&&
           out.sourceZeroKindRecursions==1&&cb.calls==6);
    assert(out.originalLastAcceptedPolygon==&records[1]);
    cb={};cb.consumed=1.0f;
    out=originalPolygonTraverse_21bd3c(records,graph,0,{1,3,5,1},{1,0,0,0},1,cb);
    assert(out.sourceHitPoint&&out.sourceHitPoint->x==2.0f&&
           out.sourcePolygonsTested==1&&cb.calls==3);
    graph[1].edges[0]={0,0};cb.consumed=0.5f;
    bool guarded=false;
    try{(void)originalPolygonTraverse_21bd3c(records,graph,0,{1,3,5,1},
        {1,0,0,0},100,cb,5);}
    catch(const std::runtime_error&){guarded=true;}
    assert(guarded);
    bool invalid=false;
    try{(void)originalPolygonTraverse_21bd3c(records,graph,3,{},{},1,cb);}
    catch(const std::out_of_range&){invalid=true;}
    assert(invalid);
    unsigned totalEdges=0,totalType2=0,graphs=0,realZeroLinks=0;
    if(argc==2){
        std::ifstream f(argv[1],std::ios::binary);assert(f);
        const std::vector<char> input((std::istreambuf_iterator<char>(f)),{});
        assert(input.size()==3328);
        std::vector<std::byte> data;
        for(unsigned char c:input)data.emplace_back(std::byte(c));
        for(unsigned g=0;g<2;++g){
            const auto source=std::span<const std::byte>(data)
                .subspan(g?2224:96,g?1104:2128);
            const auto count=u32(source,0x24),header=u32(source,0x3c),
                vertices=u32(source,0x40);
            assert(count==(g?20u:39u));
            std::vector<OriginalSpatialRecord70> original(count);
            std::vector<OriginalPolygonLinks_21bd3c> links(count);
            for(unsigned i=0;i<count;++i){
                const auto at=header+30*i;
                links[i].edgeCount=u16(source,at+2);
                assert(links[i].edgeCount<=3);
                for(unsigned e=0;e<links[i].edgeCount;++e){
                    const auto dst=u16(source,at+4+2*e);
                    const auto type=u16(source,at+10+2*e);
                    assert(dst<count&&type<=2);
                    links[i].edges[e]={dst,static_cast<std::uint16_t>(type)};
                    totalType2+=(type==2);realZeroLinks+=(type==0);++totalEdges;
                }
                const auto nVerts=u16(source,at+0x16);
                assert(nVerts>=1&&nVerts<=3);
                for(unsigned v=0;v<nVerts;++v){
                    const auto src=vertices+16*u16(source,at+0x18+2*v);
                    assert(src+16<=source.size());
                    std::memcpy(original[i].originalFields.data()+0x40+16*v,
                                source.data()+src,16);
                }
            }
            for(unsigned i=0;i<count;++i){
                EdgeResult immediate;immediate.consumed=1.0f;
                auto hit=originalPolygonTraverse_21bd3c(
                    original,links,i,{0,0,0,1},{1,0,0,0},1.0f,immediate);
                assert(hit.sourceHitPoint&&hit.originalLastAcceptedPolygon==&original[i]);
                assert(hit.sourcePolygonsTested==1&&immediate.calls==3);
            }
            ++graphs;
        }
        assert(graphs==2&&totalEdges==137&&totalType2==15&&realZeroLinks>50);
    }
    std::cout<<"Source 0x21BD3C: "<<graphs<<" retail graphs / "<<totalEdges
             <<" real links / "<<totalType2<<" special edges\n";
}
