#include "ae3/original_resource_stage_scope.hpp"
#include "ae3/original_resource_stage_registration.hpp"
#include "ae3/original_resource_stage_directory_registration.hpp"
#include <array>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>
#include <zlib.h>

namespace fs=std::filesystem;
using namespace ae3;
static void check(bool yes,const char* message){if(!yes)throw std::runtime_error(message);}
static void put16(std::vector<std::uint8_t>& b,std::size_t o,std::uint16_t x){
 b.at(o)=std::uint8_t(x);b.at(o+1)=std::uint8_t(x>>8);
}
static void put32(std::vector<std::uint8_t>& b,std::size_t o,std::uint32_t x){
 for(unsigned j=0;j<4;j++)b.at(o+j)=std::uint8_t(x>>(j*8));
}
static void ascii(std::vector<std::uint8_t>& b,std::size_t o,std::string_view s){
 for(char c:s)b.at(o++)=std::uint8_t(c);
}
static void save(const fs::path& path,const std::vector<std::uint8_t>& b){
 std::ofstream out(path,std::ios::binary|std::ios::trunc);
 out.write(reinterpret_cast<const char*>(b.data()),std::streamsize(b.size()));
 if(!out)throw std::runtime_error("Cannot save synthetic original-format fixture");
}
static std::vector<std::uint8_t> pck(std::string_view type,std::string_view name,
                                    std::span<const std::uint8_t> contents){
 std::vector<std::uint8_t> b(0x60+contents.size());
 ascii(b,0,"PCK");put32(b,4,0x40);put32(b,8,1);
 check(0x10+type.size()+1+name.size()+1<0x40,"String pool fixture too large");
 ascii(b,0x10,type);
 const auto nameOff=std::size_t(0x10)+type.size()+1;
 ascii(b,nameOff,name);
 put32(b,0x40,static_cast<std::uint32_t>(nameOff));
 put32(b,0x44,0x10);
 put32(b,0x48,0x60);put32(b,0x4c,static_cast<std::uint32_t>(contents.size()));
 std::copy(contents.begin(),contents.end(),b.begin()+0x60);
 return b;
}
static std::vector<std::uint8_t> makeSz(std::span<const std::uint8_t> raw){
 z_stream z{};
 check(deflateInit2(&z,Z_BEST_COMPRESSION,Z_DEFLATED,-MAX_WBITS,8,Z_DEFAULT_STRATEGY)==Z_OK,"Deflate setup");
 std::vector<std::uint8_t> b(4+deflateBound(&z,static_cast<uLong>(raw.size())));
 put32(b,0,static_cast<std::uint32_t>(raw.size()));
 z.next_in=const_cast<Bytef*>(raw.data());z.avail_in=static_cast<uInt>(raw.size());
 z.next_out=b.data()+4;z.avail_out=static_cast<uInt>(b.size()-4);
 const auto result=deflate(&z,Z_FINISH);const auto produced=z.total_out;
 deflateEnd(&z);check(result==Z_STREAM_END,"Deflate completion");
 b.resize(4+produced);
 b.insert(b.end(),4,0); // previously observed original nav package padding
 return b;
}
static std::vector<std::uint8_t> makeVfi(
     const std::vector<std::pair<std::string,std::vector<std::uint8_t>>>& items){
 check(items.size()<=3,"Synthetic fixture has room for 3 original members");
 std::vector<std::uint8_t> b(8192,0);
 ascii(b,0,"VFI");put32(b,4,1);put32(b,8,1);
 put16(b,16,static_cast<std::uint16_t>(items.size()));put16(b,18,0);
 put32(b,20,32);
 std::size_t cursor=32;
 for(std::size_t i=0;i<items.size();i++) {
  const auto& [path,raw]=items[i];
  const auto n=12+path.size()+1;
  check(n<65535 && cursor+n<2048 && raw.size()<2048,"VFI synthetic source too large");
  put16(b,cursor,static_cast<std::uint16_t>(n));put16(b,cursor+2,0);
  put32(b,cursor+4,static_cast<std::uint32_t>(i+1));
  put32(b,cursor+8,static_cast<std::uint32_t>(raw.size()));
  ascii(b,cursor+12,path);
  std::copy(raw.begin(),raw.end(),b.begin()+(i+1)*2048);
  cursor+=n;
 }
 put32(b,24,static_cast<std::uint32_t>(cursor));
 put32(b,28,static_cast<std::uint32_t>(cursor));
 return b;
}
static void isoRecord(std::vector<std::uint8_t>& b,std::size_t at,
                      std::string_view name,std::uint32_t sector,std::uint32_t len,bool directory){
 const auto sz=33+name.size()+(name.size()%2==0);
 b.at(at)=static_cast<std::uint8_t>(sz);put32(b,at+2,sector);put32(b,at+10,len);
 b[at+25]=directory?2:0;b[at+28]=1;b[at+30]=1;
 b[at+32]=static_cast<std::uint8_t>(name.size());ascii(b,at+33,name);
}
static void exercise(const fs::path& source) {
 OriginalStageResourceScope stage(source,"woods_c");
 check(stage.stageName()=="woods_c","Stage owner identity");
 check(stage.originalStageVfiIndices().size()==3,"Original stage VFI file count");
 check(stage.loadedRawFileCount()==0 && stage.loadedPackageCount()==0,"Lazy original stage allocation");
 const auto lua=stage.file("area.luc");
 check(lua.size()==8 && lua[0]==0x1b && lua[1]=='L',"Original Lua 5.0 signature passed through byte exact");
 const auto ptr=lua.data();
 check(stage.file("area.luc").data()==ptr,"Original source allocation must be reused");
 check(stage.loadedRawFileCount()==1,"Cached original raw member");
 const auto& nav=stage.package("navigation.pck.sz");
 check(nav.entries().size()==1 && nav.entries()[0].name=="nav_monkey.nav","Typed original NavGraph source");
 check(stage.filterMembers_26d24c("navigation.pck.sz","nav").size()==1,"Original 0x26D24C selection");
 check(stage.requireMember("navigation.pck.sz","nav_monkey.nav","nav").payload[0x24]==std::byte{1},
       "Original NavGraph count at +0x24");
 const auto* navOwner=&stage.package("navigation.pck.sz");
 check(navOwner==&stage.package("navigation.pck.sz"),"Package source owner must remain stable");
 check(stage.file("area.luc").data()==ptr,"Stage allocations stay stable when PCKs are loaded");
 const std::array<std::string_view,3> sourceOrder{{
     "area.luc","navigation.pck.sz","exdb.pck"
 }};
 OriginalStageScopedRegistration_25182c registration(stage,sourceOrder,"*");
 check(registration.records().size()==3,
       "Source registration must traverse exact three source VFI entries");
 check(registration.originalFind_24d4f0("area")->borrowedOriginalPayload[0]==std::byte{0x1b},
       "Source registration preserves original Lua source identity");
 check(registration.originalFind_24d4f0("navigation")->borrowedOriginalPayload[0]==std::byte{'P'},
       "Source registration passes real decompressed nav PCK bytes");
 check(registration.records()[1].originalTypeAt04=="pck.sz",
       "Source filename parser must retain literal indexed VFI suffix");
 check(registration.originalFind_24d4f0("exdb")->borrowedOriginalPayload[0]==std::byte{'P'},
       "Stage EXDB PCK registration");
 check(stage.file("area.luc").data()==ptr,
       "Original source lifetime must survive registry/vector allocation");
 OriginalStageScopedRegistration_25182c onlyPck(stage,sourceOrder,"pck");
 check(onlyPck.records().size()==1 &&
       onlyPck.originalFind_24d4f0("exdb")!=nullptr &&
       onlyPck.originalFind_24d4f0("navigation")==nullptr,
       "Original 0x2519FC filter compares literal source TYPE suffix");
 bool registrationRejected=false;
 const std::array<std::string_view,1> missingSource{{"not_real.plc"}};
 try{OriginalStageScopedRegistration_25182c bad(stage,missingSource,"*");}
 catch(const std::out_of_range&){registrationRejected=true;}
 check(registrationRejected,"Stage registration invented absent VFI resource");
 // The original directory map is iterated through 0x49C150 after
 // ordered byte-key lookup 0x509C10/0x627E60; this differs from source
 // VFI enumeration order (area, navigation, exdb) in this fixture.
 OriginalStageDirectoryRegistration_251958 directoryRegistered(stage,"","*");
 const auto sorted=directoryRegistered.actualDirectorySourceOrder();
 check(sorted.size()==3 &&
       sorted[0].originalLeafFilename=="area.luc" &&
       sorted[1].originalLeafFilename=="exdb.pck" &&
       sorted[2].originalLeafFilename=="navigation.pck.sz",
       "Original 0x49C150 ordered source directory enumeration");
 check(directoryRegistered.registeredRecords().size()==3 &&
       directoryRegistered.originalFind_24d4f0("navigation")->borrowedOriginalPayload[0]==std::byte{'P'},
       "Original directory order->stage VFI->SZ->registration->name lookup");
 OriginalStageDirectoryRegistration_251958 filteredDir(stage,"","pck");
 check(filteredDir.registeredRecords().size()==1 &&
       filteredDir.originalFind_24d4f0("exdb")!=nullptr,
       "Original directory type-selection uses source suffix from 0x24D858");


 bool rejected=false;
 try{stage.file("not_a_real_original_asset.pck");}catch(const std::out_of_range&){rejected=true;}
 check(rejected,"Missing original source file was fabricated");
 rejected=false;
 try{stage.package("area.luc");}catch(const std::invalid_argument&){rejected=true;}
 check(rejected,"Lua source falsely interpreted as original PCK");
 rejected=false;
 try{stage.requireMember("navigation.pck.sz","nav_extra0.nav","nav");}
 catch(const std::out_of_range&){rejected=true;}
 check(rejected,"Nonexistent typed navigation member was fabricated");
 rejected=false;
 try{(void)stage.exdb("exdb.pck","MainCameraParam.exdb");}
 catch(const std::exception&){rejected=true;}
 check(rejected,"Malformed synthetic EDB unexpectedly decoded");
 check(stage.decodedExdbCount()==0 && stage.loadedPackageCount()==2,
       "Failed EXDB decode must not publish a partially decoded document");
}
int main() {
 const auto folder=fs::temp_directory_path()/"getchu3_assets_original_stage_scope_test";
 try {
  fs::create_directories(folder);
  std::vector<std::uint8_t> navigation(0x80,0);
  put32(navigation,0x24,1);
  auto navPck=pck("nav","nav_monkey.nav",navigation);
  std::vector<std::uint8_t> exdbRaw(0x40,0);
  ascii(exdbRaw,0,"EDB\n");
  auto exdbPck=pck("exdb","MainCameraParam.exdb",exdbRaw);
  const std::vector<std::pair<std::string,std::vector<std::uint8_t>>> entries{
   {"debug/us/stage/woods_c/area.luc",{0x1b,'L','u','a',0x50,0,0,0}},
   {"debug/us/stage/woods_c/navigation.pck.sz",makeSz(navPck)},
   {"debug/us/stage/woods_c/exdb.pck",exdbPck}
  };
  const auto vfi=makeVfi(entries);
  const auto databin=folder/"DATA.BIN";save(databin,vfi);
  exercise(databin);
  std::vector<std::uint8_t> iso(23*2048,0);
  iso[16*2048]=1;ascii(iso,16*2048+1,"CD001");iso[16*2048+6]=1;
  isoRecord(iso,16*2048+156,std::string(1,'\0'),17,2048,true);
  isoRecord(iso,17*2048,"DATA.BIN;1",18,static_cast<std::uint32_t>(vfi.size()),false);
  std::copy(vfi.begin(),vfi.end(),iso.begin()+18*2048);
  const auto isopath=folder/"AE3 USA.iso";save(isopath,iso);
  exercise(isopath);
  fs::remove_all(folder);
  std::cout<<"ORIGINAL_STAGE_RESOURCE_SCOPE_TEST_PASSED mount=vfi,iso scope=raw,pck,exdb lifetime=stable\n";
  return 0;
 }catch(const std::exception& e) {
  fs::remove_all(folder);
  std::cerr<<"ORIGINAL_STAGE_RESOURCE_SCOPE_TEST_FAILED "<<e.what()<<'\n';return 1;
 }
}
