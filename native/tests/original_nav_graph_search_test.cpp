#include "ae3/original_nav_graph_search.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool b){if(!b)throw std::runtime_error("original 0x220D5C search regression");}
static unsigned u16(std::span<const std::byte> b,size_t i){ck(i+2<=b.size());return std::to_integer<unsigned>(b[i])|(std::to_integer<unsigned>(b[i+1])<<8);}
static unsigned u32(std::span<const std::byte> b,size_t i){return u16(b,i)|(u16(b,i+2)<<16);}
static auto decode(std::span<const std::byte> src){
 OriginalStageSpatialLoaded g{};g.sourceHeader.countAt24=u32(src,0x24);
 for(unsigned k=0;k<6;++k)g.sourceHeader.relativeOffsets[k]=u32(src,0x38+4*k);
 ck(g.sourceHeader.countAt24<=1000);g.records.resize(g.sourceHeader.countAt24);
 const auto h=g.sourceHeader.relativeOffsets[1],v=g.sourceHeader.relativeOffsets[2];
 for(unsigned i=0;i<g.records.size();++i){
  const auto p=h+30*i,n=u16(src,p+22);ck(n>0&&n<=3);
  for(unsigned j=0;j<n;++j){
   const auto from=v+16*u16(src,p+24+2*j);ck(from+12<=src.size());
   for(unsigned z=0;z<12;++z)g.records[i].originalFields[0x40+16*j+z]=src[from+z];
   g.records[i].originalFields[0x40+16*j+14]=std::byte{0x80};
   g.records[i].originalFields[0x40+16*j+15]=std::byte{0x3f};
  }
 }
 return g;
}
static auto vectorBytes(float x,float y,float z,float w){
 std::array<std::byte,16> b{};const float a[]={x,y,z,w};
 for(unsigned i=0;i<4;++i){const auto v=std::bit_cast<unsigned>(a[i]);
  for(unsigned j=0;j<4;++j)b[4*i+j]=std::byte((v>>(8*j))&255u);}return b;
}
int main(int argc,char**argv){
 ck(argc==1||argc==2);
 OriginalStageSpatialLoaded g{};g.records.resize(3);
 g.originalType1Children.push_back({0,0,0,{vectorBytes(0,0,0,1),vectorBytes(3,0,0,8),vectorBytes(3,4,0,21)}});
 std::vector<OriginalNavSearchNode_220c00> q(3);
 for(unsigned i=0;i<3;++i){q[i].originalSpatialRecordIndex=i;q[i].originalCentroid={10.f*i,0,0,1};}
 q[0].sourceEdges={{1,1,0},{2,0,1}};q[1].sourceEdges={{2,0,1}};
 const auto r=originalNavExecuteSearch_220d5c(q,g);
 ck(r.visitedCount==3&&r.reachableCount==3);
 ck(q[0].route.originalPredecessorAt30==0xffffffffu);
 ck(q[1].route.originalDistanceAt34==7.f&&q[2].route.originalDistanceAt34==17.f);
 ck(q[2].route.originalPredecessorAt30==1);
 std::vector<OriginalNavSearchNode_220c00> lone(2);
 lone[0].originalSpatialRecordIndex=0;lone[1].originalSpatialRecordIndex=1;
 ck(originalNavExecuteSearch_220d5c(lone,g).visitedCount==1);
 ck(lone[1].route.originalDistanceAt34==std::numeric_limits<float>::max());
 lone[0].sourceEdges={{99,0,0}};bool caught=false;
 try{(void)originalNavExecuteSearch_220d5c(lone,g);}catch(const std::out_of_range&){caught=true;}
 ck(caught);
 if(argc==2){
  std::ifstream f(argv[1],std::ios::binary);ck(bool(f));
  std::vector<char> chars((std::istreambuf_iterator<char>(f)),{});
  ck(chars.size()==3328);std::vector<std::byte> pck;
  for(unsigned char c:chars)pck.push_back(std::byte(c));
  unsigned total=0,edges=0,types2=0;
  for(unsigned file=0;file<2;++file){
   auto raw=std::span<const std::byte>(pck).subspan(file?2224:96,file?1104:2128);
   auto source=decode(raw);auto nodes=originalNavBuildAllSearchNodes_220c00(source,raw);
   ck(nodes.size()==(file?20u:39u));const auto result=originalNavExecuteSearch_220d5c(nodes,source);
   ck(result.visitedCount==nodes.size()&&result.reachableCount==nodes.size());
   std::vector<double> d(nodes.size(),std::numeric_limits<double>::infinity());d[0]=0;
   for(unsigned round=1;round<nodes.size();++round){
    bool any=false;
    for(unsigned i=0;i<nodes.size();++i)for(const auto& e:nodes[i].sourceEdges){
     const auto a=nodes[i].originalCentroid,b=nodes[e.neighborSearchIndex].originalCentroid;
     const double x=double(a.x)-b.x,y=double(a.y)-b.y,z=double(a.z)-b.z;
     const auto cost=d[i]+std::sqrt(x*x+y*y+z*z);
     if(cost<d[e.neighborSearchIndex]){d[e.neighborSearchIndex]=cost;any=true;}
    }
    if(!any)break;
   }
   for(unsigned i=0;i<nodes.size();++i){
    ck(std::fabs(double(nodes[i].route.originalDistanceAt34)-d[i])<0.003);
    if(i){const auto pred=nodes[i].route.originalPredecessorAt30;ck(pred<nodes.size());
     ck(std::any_of(nodes[pred].sourceEdges.begin(),nodes[pred].sourceEdges.end(),
      [i](const auto& e){return e.neighborSearchIndex==i;}));}
    edges+=nodes[i].sourceEdges.size();
    for(const auto& e:nodes[i].sourceEdges)types2+=(e.originalKind==2);
   }
   const std::array<std::uint32_t,1> subset{{0}};
   auto one=originalNavBuildSearchNodes_220c00(source,raw,subset);
   ck(one.size()==1&&one[0].sourceEdges.empty());
   total+=nodes.size();
  }
  ck(total==59&&edges==137&&types2==15);
 }
}
