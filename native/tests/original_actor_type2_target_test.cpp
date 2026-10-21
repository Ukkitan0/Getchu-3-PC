#include "ae3/original_actor_type2_target.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <span>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool ok){if(!ok)throw std::runtime_error("Original type-2 actor route target mismatch");}
static unsigned u16(std::span<const std::byte> b,std::size_t at){
 ck(at+2<=b.size());return std::to_integer<unsigned>(b[at])|
     (std::to_integer<unsigned>(b[at+1])<<8);
}
static unsigned u32(std::span<const std::byte> b,std::size_t at){
 return u16(b,at)|(u16(b,at+2)<<16);
}
int main(int argc,char**argv){
 std::vector<OriginalSpatialRecord70> records(4);
 for(unsigned i=0;i<records.size();++i)
  for(unsigned j=0;j<3;++j)
   for(unsigned k=0;k<16;++k)
    records[i].originalFields[0x40+16*j+k]=std::byte(i*29+j*13+k);
 const std::array<std::uint32_t,3> route{0,2,3};
 const auto target=originalActorType2Target_57bdb8(records,route);
 ck(target.targetPolygonAt50==&records[2]);
 for(unsigned j=0;j<3;++j)
  for(unsigned k=0;k<16;++k)
   ck(target.rawOriginalVertices[j][k]==records[2].originalFields[0x40+16*j+k]);
 bool failed=false;
 try{const std::array<std::uint32_t,1> shortRoute{0};
  (void)originalActorType2Target_57bdb8(records,shortRoute);}
 catch(const std::out_of_range&){failed=true;}ck(failed);
 failed=false;
 try{const std::array<std::uint32_t,2> invalid{0,4};
  (void)originalActorType2Target_57bdb8(records,invalid);}
 catch(const std::out_of_range&){failed=true;}ck(failed);
 ck(argc==1||argc==2);
 if(argc==2){
  std::ifstream f(argv[1],std::ios::binary);ck(bool(f));
  const std::vector<char> chars((std::istreambuf_iterator<char>(f)),{});
  ck(chars.size()==3328);
  std::vector<std::byte> pck;
  for(unsigned char c:chars)pck.push_back(std::byte(c));
  unsigned tested=0,polygons=0;
  for(unsigned file=0;file<2;++file){
   const auto raw=std::span<const std::byte>(pck).subspan(file?2224:96,file?1104:2128);
   const auto count=u32(raw,0x24),desc=u32(raw,0x3c),vertices=u32(raw,0x40);
   ck(count==(file?20u:39u));
   std::vector<OriginalSpatialRecord70> actual(count);
   for(unsigned i=0;i<count;++i){
    const auto record=desc+0x1e*i;
    const auto verts=u16(raw,record+0x16);ck(verts>0&&verts<=3);
    for(unsigned j=0;j<verts;++j){
     const auto off=vertices+16u*u16(raw,record+0x18+2*j);
     ck(off+16<=raw.size());
     for(unsigned k=0;k<16;++k)
      actual[i].originalFields[0x40+16*j+k]=raw[off+k];
    }
   }
   polygons+=count;
   for(unsigned i=0;i<count;++i){
    const auto record=desc+0x1e*i,degree=u16(raw,record+2);ck(degree<=3);
    for(unsigned e=0;e<degree;++e){
     if(u16(raw,record+0x0a+2*e)!=2)continue;
     const auto next=u16(raw,record+4+2*e);ck(next<count);
     const std::array<std::uint32_t,2> route{i,next};
     const auto got=originalActorType2Target_57bdb8(actual,route);
     ck(got.targetPolygonAt50==&actual[next]);
     for(unsigned j=0;j<3;++j)
      for(unsigned k=0;k<16;++k)
       ck(got.rawOriginalVertices[j][k]==actual[next].originalFields[0x40+16*j+k]);
     ++tested;
    }
   }
  }
  ck(polygons==59&&tested==15);
 }
}
