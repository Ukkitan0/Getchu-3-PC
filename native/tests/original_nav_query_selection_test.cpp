#include "ae3/original_nav_query_selection.hpp"
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
static void ck(bool b,const char* m){if(!b)throw std::runtime_error(m);}
static std::uint16_t r16(std::span<const std::byte> b,size_t at){
 ck(at+2<=b.size(),"retail u16 bounds");return std::uint16_t(std::to_integer<unsigned>(b[at]))|
 (std::uint16_t(std::to_integer<unsigned>(b[at+1]))<<8);}
static std::uint32_t r32(std::span<const std::byte> b,size_t at){return r16(b,at)|(std::uint32_t(r16(b,at+2))<<16);}
static auto decode(std::span<const std::byte> b){
 OriginalStageSpatialLoaded g{};g.sourceHeader.countAt24=r32(b,0x24);
 for(unsigned i=0;i<6;++i)g.sourceHeader.relativeOffsets[i]=r32(b,0x38+4*i);
 ck(g.sourceHeader.countAt24<=1000,"original graph count");g.records.resize(g.sourceHeader.countAt24);
 for(unsigned i=0;i<g.records.size();++i){
  auto at=g.sourceHeader.relativeOffsets[1]+30u*i;
  const auto n=r16(b,at+0x16);ck(n>0&&n<=3,"vertices");
  for(unsigned j=0;j<n;++j){
   const auto pos=g.sourceHeader.relativeOffsets[2]+16u*r16(b,at+0x18+2*j);
   ck(pos+12<=b.size(),"source vertices");
   auto& rec=g.records[i].originalFields;
   for(unsigned z=0;z<12;++z)rec[0x40+16*j+z]=b[pos+z];
   rec[0x40+16*j+14]=std::byte{0x80};rec[0x40+16*j+15]=std::byte{0x3f};
  }
 }
 return g;
}
int main(int argc,char**argv){
 ck(argc==1||argc==2,"optional private woods_c/navigation.pck");
 {
  // Original 0x220994 translates predecessor search indexes into
  // forward source spatial record IDs; this tests nonidentity IDs.
  std::vector<OriginalNavSearchNode_220c00> nodes(4);
  for(unsigned i=0;i<4;++i)nodes[i].originalSpatialRecordIndex=100+3*i;
  nodes[0].route={0xffffffffu,0};nodes[1].route={0,3};
  nodes[2].route={1,5};nodes[3].route={0,9};
  ck(originalNavExtractPath_220994(nodes,2)==std::vector<std::uint32_t>({100,103,106}),"path0-1-2");
  ck(originalNavExtractPath_220994(nodes,0)==std::vector<std::uint32_t>({100}),"source root path");
  ck(originalNavExtractPath_220994(nodes,3)==std::vector<std::uint32_t>({100,109}),"alternate predecessor");
  ck(originalNavExtractPath_220994(nodes,-1).empty(),"original -1 no-path marker");
  nodes[3].route.originalPredecessorAt30=3;bool fail=false;
  try{(void)originalNavExtractPath_220994(nodes,3);}catch(const std::invalid_argument&){fail=true;}
  ck(fail,"cyclical original predecessor rejected");
  nodes[3].route.originalDistanceAt34=std::numeric_limits<float>::max();
  ck(originalNavExtractPath_220994(nodes,3).empty(),"unreachable has no path");
 }
 if(argc==2){
  std::ifstream f(argv[1],std::ios::binary);ck(bool(f),"private retail navigation.pck missing");
  std::vector<char> chars((std::istreambuf_iterator<char>(f)),{});ck(chars.size()==3328,"real original PCK length");
  std::vector<std::byte> pck;
  for(unsigned char c:chars)pck.push_back(std::byte(c));
  unsigned total=0;
  for(unsigned file=0;file<2;++file){
   const auto raw=std::span<const std::byte>(pck).subspan(file?2224:96,file?1104:2128);
   const auto graph=decode(raw);
   const auto all=originalNavBuildAllSearchNodes_220c00(graph,raw);
   const auto count=graph.records.size();ck(count==(file?20u:39u),"real stage polygon count");
   const auto point=all[0].originalCentroid;
   // All 59 original spatial records are within this generous query
   // radius, and both genuine source graphs are connected.
   const auto full=originalNavQueryAndSearch_2207d4(graph,raw,0,point,1e9f);
   ck(full.selected.sourceRecordIndices.size()==count,"real original query selects full connected graph");
   ck(full.traversal.visitedCount==count,"source query shortest path reaches all polygons");
   const auto start=full.selected.sourceRecordIndices.front();ck(start==0,"original starts at source polygon 0");
   for(unsigned i=0;i<full.nodes.size();++i){
    const auto path=originalNavExtractPath_220994(full.nodes,i);
    ck(path.size()<=count&&path.front()==0,"original backtrace reaches source root");
    ck(path.back()==full.nodes[i].originalSpatialRecordIndex,"backtrace ends at requested polygon");
   }
   // Radius restriction: independent DFS reachability oracle from
   // raw source adjacency and centroid eligibility, not using the
   // new production selection function to build its expected set.
   for(float radius: {0.f,1.f,30.f,100.f,500.f,10000.f}){
    const auto got=originalNavSelectByRadius_220aa4(graph,raw,0,point,radius);
    std::vector<unsigned char> seen(count,0),included(count,0);
    std::vector<unsigned> stack{0};
    while(!stack.empty()){
      auto i=stack.back();stack.pop_back();if(seen[i])continue;seen[i]=1;
      const auto d=originalVU0DistanceXYZ_21b234(all[i].originalCentroid,point);
      if(!(d<radius))continue;
      included[i]=1;
      const auto at=graph.sourceHeader.relativeOffsets[1]+30u*i;
      const auto degree=r16(raw,at+2);ck(degree<=3,"original source max adjacency");
      for(unsigned e=0;e<degree;++e){const auto target=r16(raw,at+4+e*2);
        ck(target<count,"source adjacency bounds");stack.push_back(target);}
    }
    std::vector<unsigned char> actual(count,0);
    for(auto i:got.sourceRecordIndices){ck(i<count&&!actual[i],"original selection unique");actual[i]=1;}
    ck(included==actual,"original radius query disagrees with independent source DFS");
    if(radius==0.f)ck(got.sourceRecordIndices.empty(),"strict radius 0 excludes centroid");
   }
   const auto gated=originalNavSelectByRadius_220aa4(graph,raw,0,point,1e9f,true,
     [](std::uint32_t id){return id==0;});
   ck(gated.sourceRecordIndices.size()==1,"original optional eligibility excludes later nodes");
   bool threw=false;
   try{(void)originalNavSelectByRadius_220aa4(graph,raw,0,point,50.f,true);}
   catch(const std::invalid_argument&){threw=true;}
   ck(threw,"original eligibility cannot silently be skipped");
   total+=count;
  }
  ck(total==59,"all retail nav polygons used");
 }
}
