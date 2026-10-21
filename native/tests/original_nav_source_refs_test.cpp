#include "ae3/original_nav_source_refs.hpp"
#include <array>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool v){if(!v)throw std::runtime_error("Original action-2 source reference failed");}
static std::uint32_t r32(std::span<const std::byte> b,std::size_t a){
 ck(a+4<=b.size());return std::uint32_t(std::to_integer<std::uint8_t>(b[a]))|
 (std::uint32_t(std::to_integer<std::uint8_t>(b[a+1]))<<8)|
 (std::uint32_t(std::to_integer<std::uint8_t>(b[a+2]))<<16)|
 (std::uint32_t(std::to_integer<std::uint8_t>(b[a+3]))<<24);
}
static std::uint16_t r16(std::span<const std::byte> b,std::size_t a){
 ck(a+2<=b.size());return std::uint16_t(std::to_integer<std::uint8_t>(b[a]))|
 (std::uint16_t(std::to_integer<std::uint8_t>(b[a+1]))<<8);
}
int main(int argc,char**argv){
 ck(argc==2);
 std::ifstream f(argv[1],std::ios::binary);ck(bool(f));
 const std::vector<char> chars((std::istreambuf_iterator<char>(f)),{});
 std::vector<std::byte> archive;for(unsigned char c:chars)archive.push_back(std::byte{c});
 ck(archive.size()==3328);
 unsigned total=0;
 for(unsigned i=0;i<2;++i){
  const auto member=std::size_t(i?2224u:96u),len=std::size_t(i?1104u:2128u);
  const std::span<const std::byte> data=std::span<const std::byte>(archive).subspan(member,len);
  const auto count=r32(data,0x24),headers=r32(data,0x3c),table=r32(data,0x44);
  ck(count==(i?20u:39u));
  OriginalStageSpatialSource source{0x510000u+0x10000u*i,data,0};
  OriginalStageSpatialLoaded built{};
  built.sourceHeader.relativeOffsets[3]=table;
  built.records.resize(count);
  for(std::uint32_t j=0;j<count;++j){
   const auto at=std::size_t(headers)+std::size_t(j)*0x1e;
   const auto n=r16(data,at+2);
   ck(n<=3);
   for(std::uint16_t k=0;k<n;++k){
    const auto type=r16(data,at+0x0a+2*k);
    const auto arg=r16(data,at+0x10+2*k);
    if(type==1||type==2)built.pendingOriginalActions.push_back({j,k,type,arg});
   }
  }
  const auto refs=originalNavResolveType2_21d6dc(source,built);
  ck(refs.size()==(i?0u:15u));
  ck(built.pendingOriginalActions.empty());
  for(const auto& item:refs){
   ck(item.sourceTableIndex==0);
   ck(item.originalSourceRecordVA==source.runtimeBaseVA+table);
   const auto& record=built.records[item.sourceRecordIndex].originalFields;
   ck(r32(record,0x10+item.sourceActionIndex*4)==source.runtimeBaseVA+table);
  }
  total+=static_cast<unsigned>(refs.size());
 }
 ck(total==15);
 // Type-1 action's 0x21B0B0 allocation must remain pending.
 OriginalStageSpatialLoaded fake{};fake.records.resize(1);
 fake.sourceHeader.relativeOffsets[3]=128;
 fake.pendingOriginalActions={{0,0,1,4},{0,1,2,0}};
 std::array<std::byte,192> data{};
 auto refs=originalNavResolveType2_21d6dc({0x600000,data,0},fake);
 ck(refs.size()==1&&fake.pendingOriginalActions.size()==1);
 ck(fake.pendingOriginalActions.front().type==1);
 ck(r32(fake.records[0].originalFields,0x14)==0x600080);
 fake.pendingOriginalActions={{0,2,2,65535}};
 bool caught=false;
 try{(void)originalNavResolveType2_21d6dc({0x600000,data,0},fake);}
 catch(const std::out_of_range&){caught=true;}
 ck(caught&&fake.pendingOriginalActions.size()==1);
}
