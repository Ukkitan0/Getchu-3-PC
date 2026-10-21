#include "ae3/original_navgraph_topology.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
static void check(bool x,const char* message){if(!x)throw std::runtime_error(message);}
static void w16(std::vector<std::uint8_t>& b,std::size_t at,std::uint16_t n){b.at(at)=n&255;b.at(at+1)=n>>8;}
static void w32(std::vector<std::uint8_t>& b,std::size_t at,std::uint32_t n){for(int i=0;i<4;i++)b.at(at+i)=(n>>(8*i))&255;}
static void wf(std::vector<std::uint8_t>& b,std::size_t at,float n){w32(b,at,std::bit_cast<std::uint32_t>(n));}
static std::vector<std::uint8_t> sample(){
 std::vector<std::uint8_t> bytes(0x200);
 w32(bytes,0x24,3);
 w32(bytes,0x38,0x60);w32(bytes,0x3c,0x80);
 w32(bytes,0x40,0xe0);w32(bytes,0x44,0x120);
 w32(bytes,0x48,0x150);w32(bytes,0x4c,0x180);
 for(int i=0;i<3;i++){
  auto desc=0x80+0x1e*i;
  w16(bytes,desc,0xffff);
  w16(bytes,desc+0x16,1);
  w16(bytes,desc+0x18,i);
  wf(bytes,0xe0+16*i,5.0f*i);
  wf(bytes,0xe0+16*i+4,1);
  wf(bytes,0xe0+16*i+8,0);
 }
 w16(bytes,0x80+2,2);w16(bytes,0x80+4,1);w16(bytes,0x80+6,2);
 w16(bytes,0x9e + 2,1);w16(bytes,0x9e + 4,0);
 w16(bytes,0xbc+2,0);
 return bytes;
}
struct Called:OriginalNavGraphWalkCalls{
 const OriginalSpatialRecord70* base{};
 std::vector<std::size_t> order;
 void call_original_21db64(const OriginalSpatialRecord70& rec)override{
  order.push_back(static_cast<std::size_t>(&rec-base));
 }
};
int main(){
 const auto source=sample();
 const auto graph=originalNavGraphConstruct_4f0b14(source,0x510000,0x620000);
 check(graph.records.records.size()==3,"failed constructing original records");
 check(graph.topology.degree_21b3f8(0)==2,"wrong 0x21B3F8 count");
 check(graph.topology.degree_21b3f8(2)==0,"zero-degree record not supported");
 check(graph.topology.links(0)[1]==2,"original 16-bit neighbor target mismatch");
 check(graph.topology.neighbor_21b404(graph.records.records,0,0)==&graph.records.records[1],"original 0x21B404 neighbor wrong");
 check(graph.topology.neighbor_21b404(graph.records.records,0,2)==nullptr,"original out-of-range link must yield null");
 OriginalNavGraphSlots slots{};
 originalNavBindSlot_4f0b00(slots,"nav_monkey.nav",graph);
 check(originalNavLookupSlot_647b80(slots,0)==&graph,"original monkey graph pointer not registered");
 originalNavBindSlot_4f0b00(slots,"nav_enemy.nav",graph);
 check(originalNavLookupSlot_647b80(slots,1)==&graph,"original enemy slot missing");
 originalNavBindSlot_4f0b00(slots,"other.nav",graph);
 check(originalNavLookupSlot_647b80(slots,2)==&graph,"original slot two fallthrough differs");
 check(originalNavLookupSlot_647b80(slots,3)==nullptr,"outside original three slots must be null");
 OriginalSpatialGridFields grid{};
 grid.dimensionX18=grid.dimensionY1c=1;
 const std::array<std::uint16_t,1> cellOffset{{0}};
 const std::array<std::uint16_t,1> noHits{{0}};
 grid.cellOffsets50=cellOffset;grid.hitBuffer08=noHits;
 Called called{};called.base=graph.records.records.data();
 const auto chosen=originalNavVisitAdjacent_21e648(graph,grid,0,0,{0,1,0,0},called);
 check(chosen==&graph.records.records[0],"original nearest fallback did not choose first source record");
 check(called.order==std::vector<std::size_t>({0,1,2}),"original 0x21E648 current and neighbors order differs");
 called.order.clear();
 const auto chosenZero=originalNavVisitAdjacent_21e648(graph,grid,0,0,{10,1,0,0},called);
 check(chosenZero==&graph.records.records[2],"original nearest did not select third source record");
 check(called.order==std::vector<std::size_t>({2}),"zero-degree original selected record must visit once");
 auto corrupt=source;w16(corrupt,0x80+4,30000);
 bool rejected=false;
 try{(void)OriginalNavGraphTopology(std::as_bytes(std::span(corrupt)));}catch(const std::out_of_range&){rejected=true;}
 check(rejected,"invalid original adjacency must fail visibly");
 corrupt=source;w16(corrupt,0x80+2,4);
 rejected=false;
 try{(void)OriginalNavGraphTopology(std::as_bytes(std::span(corrupt)));}catch(const std::out_of_range&){rejected=true;}
 check(rejected,"source descriptor with >3 links must fail visibly");
}
