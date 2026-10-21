#include "ae3/original_actor_target_dispatch.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <span>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool x,const char* reason){if(!x)throw std::runtime_error(reason);}
struct Calls final:OriginalActorTargetGeometryCalls_57c8ec{
 const OriginalSpatialRecord70* expected=nullptr;
 int eligibleCalls=0,geometryCalls=0,threeVertexCalls=0;
 bool allow=true;
 bool call_original_21b788_eligibility(const OriginalSpatialRecord70& p,float f12) override{
  ck(&p==expected,"original 0x21B788 polygon identity");
  ck(std::bit_cast<std::uint32_t>(f12)==0x3dcccccd,"original 0x708A60 F12 bits");
  ++eligibleCalls;return allow;
 }
 std::uint64_t call_original_57ca68_geometry(const OriginalSpatialRecord70& p) override{
  ck(&p==expected,"original 0x57CA68 polygon identity");
  ++geometryCalls;return 0xdeadbeef01020304ULL;
 }
 std::uint64_t call_original_57c98c_three_vertex_transform(
  const OriginalSpatialRecord70& p,
  const std::array<std::array<std::byte,16>,3>& vertices) override{
  ck(&p==expected,"original three-vector polygon identity");
  for(unsigned j=0;j<3;++j)
   for(unsigned k=0;k<16;++k)
    ck(vertices[j][k]==p.originalFields[0x40+16*j+k],"source vector bytes");
  ++threeVertexCalls;return 0x7766554433221100ULL;
 }
};
static std::uint32_t u16(std::span<const std::byte> b,std::size_t p){
 ck(p+2<=b.size(),"retail u16 bounds");
 return std::to_integer<unsigned>(b[p])|(std::to_integer<unsigned>(b[p+1])<<8);
}
static std::uint32_t u32(std::span<const std::byte> b,std::size_t p){
 return u16(b,p)|(u16(b,p+2)<<16);
}
int main(int argc,char**argv){
 OriginalSpatialRecord70 polygon{};
 for(unsigned i=0;i<polygon.originalFields.size();++i)
  polygon.originalFields[i]=std::byte(i*3);
 std::array<std::byte,16> stored{};
 for(unsigned i=0;i<stored.size();++i)stored[i]=std::byte(i+1);
 Calls c;c.expected=&polygon;
 auto a=originalActorTargetVector_57c8ec(polygon,true,stored,c);
 ck(a.branch==OriginalActorTargetBranch_57c8ec::OriginalVariantVectorPassThrough_57c944,
    "original positive eligibility direct-vector branch");
 ck(a.originalVectorLow64==0x0807060504030201ULL,
    "LQC2/DMFC2 original stored vector low 64 bits");
 ck(c.eligibleCalls==1&&c.geometryCalls==0&&c.threeVertexCalls==0,
    "direct stored-vector branch avoids geometry");
 c.allow=false;
 auto b=originalActorTargetVector_57c8ec(polygon,true,stored,c);
 ck(b.branch==OriginalActorTargetBranch_57c8ec::OriginalFallbackGeometry_57ca68,
    "original fallback branch after false eligibility");
 ck(b.originalVectorLow64==0xdeadbeef01020304ULL&&c.geometryCalls==1,
    "forward authentic geometry-result bits");
 auto d=originalActorTargetVector_57c8ec(polygon,false,stored,c);
 ck(d.branch==OriginalActorTargetBranch_57c8ec::OriginalThreeVertexTransform_57c98c,
    "original variant-zero three vertices");
 ck(d.originalVectorLow64==0x7766554433221100ULL&&c.threeVertexCalls==1,
    "original 0x21B458 three-vector external transform");
 ck(c.eligibleCalls==2,"variant-zero does not call 0x21B788");
 ck(argc==1||argc==2,"optional original woods_c/navigation.pck");
 if(argc==2){
  std::ifstream f(argv[1],std::ios::binary);ck(bool(f),"original woods_c PCK missing");
  const std::vector<char> input((std::istreambuf_iterator<char>(f)),{});
  ck(input.size()==3328,"authentic woods_c PCK expected bytes");
  std::vector<std::byte> bytes;
  for(unsigned char x:input)bytes.push_back(std::byte(x));
  unsigned realRecords=0,direct=0,fallback=0,triangles=0;
  for(unsigned graph=0;graph<2;++graph){
   const auto raw=std::span<const std::byte>(bytes).subspan(graph?2224:96,graph?1104:2128);
   const unsigned count=u32(raw,0x24),desc=u32(raw,0x3c),vertices=u32(raw,0x40);
   ck(count==(graph?20u:39u),"original source graph polygon count");
   for(unsigned i=0;i<count;++i){
    const auto at=desc+30*i,degree=u16(raw,at+0x16);
    ck(degree>=1&&degree<=3,"original spatial vertex count");
    OriginalSpatialRecord70 record{};
    for(unsigned j=0;j<degree;++j){
     const auto offset=vertices+16*u16(raw,at+0x18+2*j);
     ck(offset+16<=raw.size(),"retail source vector bounds");
     for(unsigned k=0;k<16;++k)record.originalFields[0x40+16*j+k]=raw[offset+k];
    }
    Calls call;call.expected=&record;
    const auto pass=originalActorTargetVector_57c8ec(record,true,stored,call);
    ck(pass.branch==OriginalActorTargetBranch_57c8ec::OriginalVariantVectorPassThrough_57c944
       &&pass.originalVectorLow64==0x0807060504030201ULL,
       "source polygon stored-vector pass-through");
    ++direct;
    call.allow=false;
    const auto fail=originalActorTargetVector_57c8ec(record,true,stored,call);
    ck(fail.branch==OriginalActorTargetBranch_57c8ec::OriginalFallbackGeometry_57ca68,
       "source polygon geometry-fallback branch");++fallback;
    const auto three=originalActorTargetVector_57c8ec(record,false,stored,call);
    ck(three.branch==OriginalActorTargetBranch_57c8ec::OriginalThreeVertexTransform_57c98c,
       "source polygon three original vertex vector reads");
    ck(call.threeVertexCalls==1,"original vector transform called");++triangles;
    ++realRecords;
   }
  }
  ck(realRecords==59&&direct==59&&fallback==59&&triangles==59,
     "59 original polygon records through each of three source dispatch paths");
 }
}
