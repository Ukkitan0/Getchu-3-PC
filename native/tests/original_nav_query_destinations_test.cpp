#include "ae3/original_nav_query_destinations.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <fstream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool b){if(!b)throw std::runtime_error("Original nav destination selection mismatch");}
static unsigned r16(std::span<const std::byte> v,std::size_t p){ck(p+2<=v.size());return std::to_integer<unsigned>(v[p])|(std::to_integer<unsigned>(v[p+1])<<8);}
static unsigned r32(std::span<const std::byte> v,std::size_t p){return r16(v,p)|(r16(v,p+2)<<16);}
static auto decode(std::span<const std::byte> b){
 OriginalStageSpatialLoaded g{};g.sourceHeader.countAt24=r32(b,0x24);
 for(unsigned i=0;i<6;++i)g.sourceHeader.relativeOffsets[i]=r32(b,0x38+4*i);
 ck(g.sourceHeader.countAt24<=1000);g.records.resize(g.sourceHeader.countAt24);
 for(unsigned i=0;i<g.records.size();++i){
  const auto at=g.sourceHeader.relativeOffsets[1]+30u*i;
  const auto n=r16(b,at+0x16);ck(n>0&&n<=3);
  for(unsigned j=0;j<n;++j){
   const auto p=g.sourceHeader.relativeOffsets[2]+16u*r16(b,at+0x18+2*j);
   ck(p+12<=b.size());
   auto& rec=g.records[i].originalFields;
   for(unsigned x=0;x<12;++x)rec[0x40+16*j+x]=b[p+x];
   rec[0x40+16*j+14]=std::byte{0x80};rec[0x40+16*j+15]=std::byte{0x3f};
  }
 }
 return g;
}
int main(int argc,char**argv){
 ck(argc==1||argc==2);
 const OriginalVU0Vec4 origin{0,0,0,1};
 std::vector<OriginalNavSearchNode_220c00> sample(4);
 sample[0].originalCentroid={1,0,0,1};sample[1].originalCentroid={2,0,0,1};
 sample[2].originalCentroid={2,0,0,1};sample[3].originalCentroid={1,0,0,1};
 for(unsigned i=0;i<4;++i){sample[i].originalSpatialRecordIndex=i;sample[i].route={i?0u:0xffffffffu,float(i)};}
 ck(originalNavNearestQueryNode_220f60(sample,origin)==0);
 ck(originalNavFarthestQueryNode_221030(sample,origin)==1);
 ck(originalNavDestinationForMode_2208a8(sample,origin,0)==1);
 ck(originalNavDestinationForMode_2208a8(sample,origin,1)==0);
 ck(originalNavDestinationForMode_2208a8(sample,origin,3)==3);
 ck(originalNavDestinationPath_2208a8(sample,origin,0)==std::vector<std::uint32_t>({0,1}));
 ck(originalNavDestinationPath_2208a8(sample,origin,1)==std::vector<std::uint32_t>({0}));
 ck(originalNavNearestQueryNode_220f60({},origin)==-1);
 ck(originalNavFarthestQueryNode_221030({},origin)==-1);
 ck(originalNavDestinationPath_2208a8({},origin,1).empty());
 bool failed=false;try{(void)originalNavDestinationForMode_2208a8(sample,origin,99);}catch(const std::out_of_range&){failed=true;}
 ck(failed);
 if(argc==2){
  std::ifstream file(argv[1],std::ios::binary);ck(bool(file));
  const std::vector<char> in((std::istreambuf_iterator<char>(file)),{});ck(in.size()==3328);
  std::vector<std::byte> pck;for(unsigned char c:in)pck.push_back(std::byte(c));
  unsigned verified=0;
  for(unsigned file=0;file<2;++file){
   const auto raw=std::span<const std::byte>(pck).subspan(file?2224:96,file?1104:2128);
   const auto graph=decode(raw);const auto all=originalNavBuildAllSearchNodes_220c00(graph,raw);
   const auto query=all.front().originalCentroid;
   auto executed=originalNavQueryAndSearch_2207d4(graph,raw,0,query,1e9f);
   ck(executed.traversal.reachableCount==graph.records.size());
   auto& nodes=executed.nodes;
   for(const auto& point:std::array<OriginalVU0Vec4,3>{{query,{1,2,3,0},{100,-250,150,1}}}){
    int closest=-1,furthest=-1;double closestD=std::numeric_limits<double>::max(),furthestD=-1;
    for(unsigned i=0;i<nodes.size();++i){
     const auto p=nodes[i].originalCentroid;
     const double x=double(p.x)-point.x,y=double(p.y)-point.y,z=double(p.z)-point.z;
     const auto dist=std::sqrt(x*x+y*y+z*z);
     if(dist<closestD){closest=i;closestD=dist;}
     if(furthestD<dist){furthest=i;furthestD=dist;}
    }
    const auto nearest=originalNavDestinationForMode_2208a8(nodes,point,1);
    const auto far=originalNavDestinationForMode_2208a8(nodes,point,0);
    ck(nearest==closest&&far==furthest);
    for(auto mode:{0,1}){
     const auto selected=originalNavDestinationForMode_2208a8(nodes,point,mode);
     const auto route=originalNavDestinationPath_2208a8(nodes,point,mode);
     ck(!route.empty()&&route.front()==0&&route.back()==nodes[selected].originalSpatialRecordIndex);
    }
   }
   verified+=nodes.size();
  }
  ck(verified==59);
 }
}
