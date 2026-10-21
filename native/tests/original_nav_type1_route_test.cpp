#include "ae3/original_nav_type1_route.hpp"
#include "ae3/original_nav_type1_children.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool x){if(!x)throw std::runtime_error("AE3 original route cost mismatch");}
static bool close(float a,float b,float tolerance=0.002f){return std::fabs(a-b)<tolerance;}
static std::array<std::byte,16> bytes(float x,float y,float z,float w){
    std::array<std::byte,16> b{};
    for(auto [i,v]:std::array<std::pair<unsigned,float>,4>{{{0,x},{1,y},{2,z},{3,w}}}){
        const auto u=std::bit_cast<std::uint32_t>(v);
        for(unsigned j=0;j<4;++j)b[4*i+j]=std::byte((u>>(8*j))&255u);
    }
    return b;
}
static unsigned rd16(std::span<const std::byte> d,std::size_t p){
    ck(p+2<=d.size());return std::to_integer<unsigned>(d[p])|
        (std::to_integer<unsigned>(d[p+1])<<8);
}
static std::uint32_t rd32(std::span<const std::byte> d,std::size_t p){
    return std::uint32_t(rd16(d,p))|(std::uint32_t(rd16(d,p+2))<<16);
}
int main(int argc,char**argv){
    OriginalNavType1Child_21b0b0 child{};
    ck(originalNavType1ChildDistance_21b194(child)==0);
    child.originalVectors={bytes(1,2,3,1),bytes(4,6,3,100),bytes(4,6,15,-20),bytes(-1,6,3,1)};
    ck(close(originalNavType1ChildDistance_21b194(child),30));
    auto copy=child;
    for(auto& v:copy.originalVectors)for(unsigned i=12;i<16;++i)v[i]=std::byte{0};
    ck(originalNavType1ChildDistance_21b194(child)==originalNavType1ChildDistance_21b194(copy));
    child.sourceRecordIndex=1;child.sourceActionIndex=2;
    OriginalStageSpatialLoaded graph{};graph.records.resize(2);graph.originalType1Children.push_back(child);
    ck(close(originalNavType1ActionDistance_220f28(graph,1,2),30));
    bool failed=false;
    try{(void)originalNavType1ActionDistance_220f28(graph,0,0);}catch(const std::out_of_range&){failed=true;}
    ck(failed);
    OriginalNavRouteCost_220e8c dst{99,50};
    ck(originalNavType1Relax_220f18(graph,1,2,2.f,dst,42));
    ck(close(dst.originalDistanceAt34,32.f)&&dst.originalPredecessorAt30==42);
    ck(!originalNavType1Relax_220f18(graph,1,2,2.f,dst,5)); // strict less-than
    ck(!originalNavType1Relax_220f18(graph,1,2,std::numeric_limits<float>::quiet_NaN(),dst,5));
    ck(dst.originalPredecessorAt30==42);
    const std::array<OriginalVU0Vec4,3> triangle{{{0,0,0,1},{6,0,0,1},{0,6,9,1}}};
    OriginalNavRouteCost_220e8c direct{90,100};
    ck(originalNavCentroidRelax_220ec8(triangle,{5,6,3,-9},1,direct,77));
    ck(close(direct.originalDistanceAt34,6)&&direct.originalPredecessorAt30==77);
    ck(!originalNavCentroidRelax_220ec8(triangle,{5,6,3,100},2,direct,8));
    // Source field integration: stage+0x4C child count and u16 indices;
    // each index selects one stage+0x40 16-byte source vector.
    std::vector<std::byte> raw(512);
    for(unsigned i=0;i<3;++i){
        auto v=bytes(i==0?0:3,i==0?0:4,i==2?12:0,1);
        for(unsigned j=0;j<16;++j)raw[0x80+16*i+j]=v[j];
    }
    raw[0x180]=std::byte{3};
    raw[0x184]=std::byte{1};raw[0x186]=std::byte{2};
    OriginalStageSpatialLoaded parsed{};parsed.records.resize(1);
    parsed.sourceHeader.relativeOffsets[2]=0x80;
    parsed.sourceHeader.relativeOffsets[5]=0x180;
    parsed.pendingOriginalActions={{0,1,1,0}};
    ck(originalNavMaterializeType1_21d70c({0x510000,raw,0},parsed)==1);
    ck(parsed.pendingOriginalActions.empty());
    ck(close(originalNavType1ActionDistance_220f28(parsed,0,1),17));
    // Independent double reference for the retained original VU0 function.
    std::mt19937 rng(0x21b194);
    for(unsigned rep=0;rep<20000;++rep){
        OriginalNavType1Child_21b0b0 source{};
        double oracle=0;std::array<float,3> prev{};
        const unsigned n=rng()%9;
        for(unsigned i=0;i<n;++i){
            std::array<float,3> p{};
            for(auto& v:p)v=float(int(rng()%201)-100);
            source.originalVectors.push_back(bytes(p[0],p[1],p[2],float(rng()%201)));
            if(i){double squared=0;for(unsigned j=0;j<3;++j){
                const double d=double(p[j])-prev[j];squared+=d*d;}
                oracle+=std::sqrt(squared);}
            prev=p;
        }
        ck(std::fabs(double(originalNavType1ChildDistance_21b194(source))-oracle)<0.002);
    }
    if(argc==2){
        // Real private woods_c/navigation.pck: check original route
        // traversal's centroid path on ALL 59 authentic polygons.
        std::ifstream f(argv[1],std::ios::binary);ck(bool(f));
        const std::vector<char> chars((std::istreambuf_iterator<char>(f)),{});
        ck(chars.size()==3328);
        std::vector<std::byte> v;v.reserve(chars.size());
        for(unsigned char c:chars)v.push_back(std::byte(c));
        const std::span<const std::byte> pck{v};
        unsigned polygons=0,types1=0,types2=0;
        for(unsigned k=0;k<2;++k){
            const auto nav=pck.subspan(k?2224:96,k?1104:2128);
            const auto count=rd32(nav,0x24),descs=rd32(nav,0x3c),base=rd32(nav,0x40);
            ck(count==(k?20u:39u));
            for(unsigned i=0;i<count;++i){
                const auto at=std::size_t(descs)+i*0x1e;
                ck(rd16(nav,at+0x16)==3);
                std::array<OriginalVU0Vec4,3> points{};
                for(unsigned j=0;j<3;++j){
                    const auto pos=std::size_t(base)+rd16(nav,at+0x18+2*j)*16;
                    ck(pos+16<=nav.size());
                    const auto coord=[&](unsigned n){return std::bit_cast<float>(rd32(nav,pos+n*4));};
                    points[j]={coord(0),coord(1),coord(2),1.f};
                }
                const auto center=originalVU0Mean_21b468(points);
                ck(std::isfinite(center.x)&&std::isfinite(center.y)&&std::isfinite(center.z));
                OriginalNavRouteCost_220e8c state{0,1e6f};
                ck(originalNavCentroidRelax_220ec8(points,center,4.f,state,7));
                ck(close(state.originalDistanceAt34,4.f)&&state.originalPredecessorAt30==7);
                const auto actions=rd16(nav,at+2);ck(actions<=3);
                for(unsigned a=0;a<actions;++a){
                    const auto type=rd16(nav,at+0x0a+2*a);
                    types1+=(type==1);types2+=(type==2);
                }
                ++polygons;
            }
        }
        ck(polygons==59&&types1==0&&types2==15);
    }
}
