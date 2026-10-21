#include "ae3/original_stage_spatial_loader.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void check(bool result) {if(!result)throw std::runtime_error("original 0x21D46C source spatial loader regression");}
static void put16(std::vector<std::byte>& p,std::size_t off,std::uint16_t x) {
 p.at(off)=std::byte(x&255);p.at(off+1)=std::byte(x>>8);
}
static void put32(std::vector<std::byte>& p,std::size_t off,std::uint32_t x) {
 for(int i=0;i<4;i++)p.at(off+i)=std::byte((x>>(i*8))&255);
}
static void vec(std::vector<std::byte>& p,std::size_t off,float x,float y,float z) {
 put32(p,off,std::bit_cast<std::uint32_t>(x));
 put32(p,off+4,std::bit_cast<std::uint32_t>(y));
 put32(p,off+8,std::bit_cast<std::uint32_t>(z));
 put32(p,off+12,0xbadcafeu); // source 4th lane is NOT copied
}
int main(){
 constexpr std::uint32_t sourceBaseVA=0x510000;
 std::vector<std::byte> bytes(0x220);
 put32(bytes,0x24,2); // source+0x24 original spatial record count
 for(std::uint32_t i=0;i<6;i++)put32(bytes,0x38+i*4,0x60+0x20*i);
 // Source table offsets are independent of descriptor table placement.
 put32(bytes,0x38,0x60);put32(bytes,0x3c,0x100);
 put32(bytes,0x40,0x160);put32(bytes,0x44,0x1a0);
 put32(bytes,0x48,0x1c0);put32(bytes,0x4c,0x1e0);
 const auto desc0=std::size_t(0x100),desc1=desc0+0x1e;
 put16(bytes,desc0,0xffff);put16(bytes,desc1,0xffff);
 put16(bytes,desc0+0x16,3);put16(bytes,desc1+0x16,1);
 put16(bytes,desc0+0x18,0);put16(bytes,desc0+0x1a,1);put16(bytes,desc0+0x1c,2);
 put16(bytes,desc1+0x18,3);
 vec(bytes,0x160,1,0,0);vec(bytes,0x170,4,1,0);
 vec(bytes,0x180,8,0,1);vec(bytes,0x190,5,0,0);
 OriginalStageSpatialSource input{sourceBaseVA,bytes,0x6a1000};
 const auto offsets=originalStageReadSpatialOffsets_21d46c(input);
 check(offsets.countAt24==2);
 check(offsets.relativeOffsets[1]==0x100&&offsets.resolvedVAs[1]==sourceBaseVA+0x100);
 const auto result=originalStageBuildSourceSpatial_21d46c(input);
 check(result.records.size()==2&&result.polygonHeaders.size()==2);
 check(result.pendingOriginalActions.empty());
 const auto readWord=[](const auto& r,std::size_t at) {
  const auto& f=r.originalFields;
  return std::uint32_t(std::to_integer<std::uint8_t>(f[at]))|
    (std::uint32_t(std::to_integer<std::uint8_t>(f[at+1]))<<8)|
    (std::uint32_t(std::to_integer<std::uint8_t>(f[at+2]))<<16)|
    (std::uint32_t(std::to_integer<std::uint8_t>(f[at+3]))<<24);
 };
 check(readWord(result.records[0],0)==0x6a1000);
 check(readWord(result.records[0],0x24)==0x6a1018);
 check(originalSpatialHeaderPointer_21b468(result.records[0])==sourceBaseVA+0x100);
 check(originalSpatialHeaderPointer_21b468(result.records[1])==sourceBaseVA+0x11e);
 const auto vertices0=originalSpatialVertices_21b468(result.records[0]);
 check(vertices0[0].x==1.f&&vertices0[1].x==4.f&&vertices0[2].x==8.f);
 check(vertices0[0].w==1.f&&vertices0[1].w==1.f&&vertices0[2].w==1.f);
 const auto vertices1=originalSpatialVertices_21b468(result.records[1]);
 check(vertices1[0].x==5.f&&vertices1[0].w==1.f);
 std::array<OriginalSpatialSourceAllocation,1> raw{{{sourceBaseVA,bytes}}};
 const auto headers=originalSpatialResolveHeaders_21b468(result.records,raw);
 check(headers[0].sourcePointCountAt16==3&&headers[1].sourcePointCountAt16==1);
 const auto nearest=originalSpatialNearestOriginalDataAll_21da2c(result.records,raw,{5,0,0,0});
 check(nearest==&result.records[1]);
 check(originalStageNearestAll_21da2c(result,{5,0,0,0})==&result.records[1]);
 OriginalSpatialGridFields grid{};
 grid.stepX10=grid.stepY14=1;grid.dimensionX18=grid.dimensionY1c=1;
 const std::array<std::uint16_t,1> cellOffsets{{0}};
 const std::array<std::uint16_t,3> pool{{2,0,1}};
 grid.cellOffsets50=cellOffsets;grid.hitBuffer08=pool;
 check(originalStageNearestGrid_21d898(result,grid,0,0,{5,0,0,0})==&result.records[1]);
 // Recover the real, still-untranslated original action branch descriptor.
 put16(bytes,desc0+2,1);put16(bytes,desc0+0x0a,2);
 put16(bytes,desc0+0x10,7);
 input.loadedBytes=bytes;
 const auto withPending=originalStageBuildSourceSpatial_21d46c(input);
 check(withPending.pendingOriginalActions.size()==1);
 check(withPending.pendingOriginalActions[0].type==2&&withPending.pendingOriginalActions[0].argument==7);
 put16(bytes,desc0+0x16,4);
 bool rejected=false;
 try{(void)originalStageBuildSourceSpatial_21d46c({sourceBaseVA,bytes});}
 catch(const std::out_of_range&){rejected=true;}check(rejected);
 put16(bytes,desc0+0x16,3);
 put32(bytes,0x24,0x7fffffffu);
 rejected=false;
 try{(void)originalStageBuildSourceSpatial_21d46c({sourceBaseVA,bytes});}
 catch(const std::out_of_range&){rejected=true;}check(rejected);
 put32(bytes,0x24,2);put32(bytes,0x40,0x218);
 rejected=false;
 try{(void)originalStageBuildSourceSpatial_21d46c({sourceBaseVA,bytes});}
 catch(const std::out_of_range&){rejected=true;}check(rejected);
}
