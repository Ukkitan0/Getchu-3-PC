#include "ae3/original_resource_sz.hpp"
#include "ae3/original_resource_package.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>
#include <zlib.h>
using namespace ae3;
static void ck(bool good,const char* msg){if(!good)throw std::runtime_error(msg);}
template<class F> static void rejected(F f) {bool failed=false;try{f();}catch(const std::exception&){failed=true;}ck(failed,"Invalid original SZ was accepted");}
static void writeSize(std::vector<std::uint8_t>& data,std::uint32_t size) {
 for(unsigned i=0;i<4;++i)data.at(i)=std::uint8_t(size>>(i*8));
}
// This is an original-FORMAT synthetic fixture, not copyrighted game bytes.
static std::vector<std::uint8_t> encode(const std::vector<std::uint8_t>& source) {
 z_stream z{};
 if(deflateInit2(&z,Z_BEST_COMPRESSION,Z_DEFLATED,-MAX_WBITS,8,Z_DEFAULT_STRATEGY)!=Z_OK)
   throw std::runtime_error("zlib fixture failure");
 std::vector<std::uint8_t> out(4+compressBound(static_cast<uLong>(source.size())));
 writeSize(out,static_cast<std::uint32_t>(source.size()));
 z.next_in=const_cast<Bytef*>(source.data());z.avail_in=static_cast<uInt>(source.size());
 z.next_out=out.data()+4;z.avail_out=static_cast<uInt>(out.size()-4);
 auto result=deflate(&z,Z_FINISH);
 const std::size_t used=4+z.total_out;
 (void)deflateEnd(&z);
 if(result!=Z_STREAM_END)throw std::runtime_error("zlib fixture not flushed");
 out.resize(used);return out;
}
int main() {
 try {
  const std::array<std::size_t,9> sizes{0,1,2,3,14,1024,65535,65536,524288};
  std::mt19937 rng(0xaee30003);
  for(auto n:sizes) {
   std::vector<std::uint8_t> source(n);
   for(auto& x:source)x=static_cast<std::uint8_t>(rng());
   auto sz=encode(source);
   ck(originalDecodeSz(sz)==source,"Source bytes changed after native SZ expansion");
   if(n>0) rejected([&]{originalDecodeSz(sz,n-1);});
   auto bad=sz;writeSize(bad,static_cast<std::uint32_t>(n+1));
   rejected([&]{originalDecodeSz(bad);});
   bad=sz;bad.push_back(0x66);rejected([&]{originalDecodeSz(bad);});
   ck(originalDecodeSz(bad,OriginalSzMaxOutputBytes,OriginalSzTrailerPolicy::permit)==source,
      "Original navigation SZ trailing bytes must be accepted by compatible decoder");
   if(sz.size()>5){bad=sz;bad.pop_back();rejected([&]{originalDecodeSz(bad);});}
  }
  rejected([]{originalDecodeSz({});});
  rejected([]{const std::array<std::uint8_t,3> shortHeader{1,0,0};originalDecodeSz(shortHeader);});
  rejected([]{const std::array<std::uint8_t,5> badStream{0,0,0,0,0xff};originalDecodeSz(badStream);});
  // Authentic parsing chain at the source boundary: SZ -> PCK resource table
  // -> original I3D payload bytes. Generated structure; no game content.
  std::vector<std::uint8_t> sourcePck(0x40);
  sourcePck[0]='P';sourcePck[1]='C';sourcePck[2]='K';
  sourcePck[4]=0x20;sourcePck[8]=1;
  const char* model="model";for(int i=0;model[i];++i)sourcePck[0x10+i]=model[i];
  const char* type="i3d";for(int i=0;type[i];++i)sourcePck[0x16+i]=type[i];
  sourcePck[0x20]=0x10;sourcePck[0x24]=0x16;
  sourcePck[0x28]=0x30;sourcePck[0x2c]=8;
  const char* signature="I3D_BIN";for(int i=0;signature[i];++i)sourcePck[0x30+i]=signature[i];
  auto expandedPck=originalDecodeSz(encode(sourcePck));
  std::vector<std::byte> sourceOwned;
  for(auto value:expandedPck)sourceOwned.push_back(std::byte{value});
  OriginalPckResourcePackage originalPck(std::move(sourceOwned));
  ck(originalPck.entries().size()==1,"original SZ -> PCK entry count");
  ck(originalPck.require("model","i3d").payloadOffset==0x30,"original SZ -> PCK payload offset");
  ck(originalPckPayloadSignature(originalPck.entries()[0].payload)==OriginalPckPayloadSignature::i3d_mesh,
     "original SZ -> PCK -> original resource magic");
  auto highlyCompressed=std::vector<std::uint8_t>(100000,0x66);
  auto encoded=encode(highlyCompressed);
  ck(originalDecodeSz(encoded)==highlyCompressed,"Source highly-compressible data changed");
  std::cout<<"ORIGINAL_RESOURCE_SZ_TEST_PASSED sizes="<<sizes.size()<<" guards=length,truncation,trailing,budget\n";
  return 0;
 }catch(const std::exception& e){std::cerr<<"ORIGINAL_RESOURCE_SZ_TEST_FAILED "<<e.what()<<"\n";return 1;}
}
