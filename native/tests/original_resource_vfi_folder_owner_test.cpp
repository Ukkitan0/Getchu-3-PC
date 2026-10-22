#include "ae3/archive.hpp"
#include "ae3/original_resource_stage_scope.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>
namespace fs=std::filesystem;
using namespace ae3;
static void ck(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static void w16(std::vector<std::uint8_t>& b,std::size_t p,std::uint16_t x){
 b.at(p)=std::uint8_t(x);b.at(p+1)=std::uint8_t(x>>8);
}
static void w32(std::vector<std::uint8_t>& b,std::size_t p,std::uint32_t x){
 for(int j=0;j<4;++j)b.at(p+j)=std::uint8_t(x>>(8*j));
}
static void text(std::vector<std::uint8_t>& b,std::size_t p,std::string_view s){
 for(char c:s)b.at(p++)=std::uint8_t(c);
}
static void write(const fs::path& p,const std::vector<std::uint8_t>& bytes){
 std::ofstream out(p,std::ios::binary);
 out.write(reinterpret_cast<const char*>(bytes.data()),static_cast<std::streamsize>(bytes.size()));
 if(!out)throw std::runtime_error("Failed to create original-format fixture");
}
static std::uint16_t folder(std::vector<std::uint8_t>& bytes,std::size_t& cursor,
                            std::size_t base,std::string_view name,std::uint16_t parent){
 const auto rel=cursor-base;
 ck(rel>0 || name=="z","VFI parent 0 is reserved for root termination");
 ck(rel<=65535,"VFI folder parent-offset exceeds 16 bits");
 const auto n=8+name.size()+1;
 w16(bytes,cursor,static_cast<std::uint16_t>(n));w16(bytes,cursor+4,parent);
 text(bytes,cursor+8,name);cursor+=n;
 return static_cast<std::uint16_t>(rel);
}
struct Fixture {
 std::vector<std::uint8_t> source;
 std::size_t debugParentField=0;
 std::size_t fileParentField=0;
 std::uint16_t woodsFolderOffset=0;
};
static Fixture sourceVfi(){
 Fixture f;
 f.source.assign(3*2048,0);
 auto& b=f.source;
 text(b,0,"VFI");w32(b,4,1);w32(b,8,1);
 w16(b,16,2);w16(b,18,5);w32(b,20,32);
 const std::string_view area="area.luc",data="exdb.pck";
 const std::size_t file0=32;
 const std::size_t file1=file0+12+area.size()+1;
 const std::size_t folderStart=file1+12+data.size()+1;
 std::size_t p=folderStart;
 // Original root reference (parent 0), then four actual directory
 // components whose parent identifiers are folder-table-relative offsets.
 const auto dummy=folder(b,p,folderStart,"z",0);
 ck(dummy==0,"Dummy root offset");
 const std::size_t debugAt=p;
 const auto debug=folder(b,p,folderStart,"debug",0);
 const auto us=folder(b,p,folderStart,"us",debug);
 const auto stage=folder(b,p,folderStart,"stage",us);
 const auto woods=folder(b,p,folderStart,"woods_c",stage);
 f.woodsFolderOffset=woods;f.debugParentField=debugAt+4;
 ck(p<2048,"Index and folder table must fit before VFI payload extent");
 w32(b,24,static_cast<std::uint32_t>(folderStart));
 w32(b,28,static_cast<std::uint32_t>(p));
 w16(b,file0,static_cast<std::uint16_t>(12+area.size()+1));
 w16(b,file0+2,woods);w32(b,file0+4,1);w32(b,file0+8,8);
 text(b,file0+12,area);
 w16(b,file1,static_cast<std::uint16_t>(12+data.size()+1));
 w16(b,file1+2,woods);w32(b,file1+4,2);
 f.fileParentField=file1+2;
 text(b,file1+12,data);
 const std::vector<std::uint8_t> lua{0x1b,'L','u','a',0x50,0,0,0};
 std::copy(lua.begin(),lua.end(),b.begin()+2048);
 // One authentic-PCK-LAYOUT metadata row with a source EDB signature;
 // completely synthetic content, not any retail EXDB property values.
 std::vector<std::uint8_t> pkg(0x70);
 text(pkg,0,"PCK");w32(pkg,4,0x40);w32(pkg,8,1);
 text(pkg,0x10,"exdb");
 text(pkg,0x18,"MainCameraParam.exdb");
 w32(pkg,0x40,0x18);w32(pkg,0x44,0x10);
 w32(pkg,0x48,0x60);w32(pkg,0x4c,0x10);
 text(pkg,0x60,"EDB\n");
 w32(b,file1+8,static_cast<std::uint32_t>(pkg.size()));
 std::copy(pkg.begin(),pkg.end(),b.begin()+4096);
 return f;
}
int main(){
 const auto home=fs::temp_directory_path()/"ae3_original_vfi_folder_ownership";
 try{
  fs::create_directories(home);
  auto original=sourceVfi();
  auto path=home/"retail_layout.DATA.BIN";
  write(path,original.source);
  Archive archive(path);
  ck(archive.entries().size()==2,"Original VFI file entries");
  ck(archive.entries()[0].path=="debug/us/stage/woods_c/area.luc",
     "Original parent-linked Lua filename resolution");
  ck(archive.entries()[1].path=="debug/us/stage/woods_c/exdb.pck",
     "Original parent-linked PCK filename resolution");
  OriginalStageResourceScope stage(path,"woods_c");
  ck(stage.originalStageVfiIndices().size()==2,"Source stage inventory from parent-linked VFI");
  ck(stage.file("area.luc")[0]==0x1b,"Original Lua bytes from parent-linked source");
  const auto& pck=stage.package("exdb.pck");
  ck(pck.entries().size()==1 &&
     stage.requireMember("exdb.pck","MainCameraParam.exdb","exdb").payloadOffset==0x60,
     "Original PCK resource after VFI nested folder owner resolution");
  auto missing=original;
  w16(missing.source,missing.fileParentField,0xffff);
  auto bad=home/"missing_parent.DATA.BIN";
  write(bad,missing.source);
  bool failed=false;
  try{Archive invalid(bad);}catch(const std::runtime_error&){failed=true;}
  ck(failed,"Missing VFI source folder parent must be rejected");
  auto cyclic=original;
  w16(cyclic.source,cyclic.debugParentField,cyclic.woodsFolderOffset);
  bad=home/"cyclic_parent.DATA.BIN";write(bad,cyclic.source);
  failed=false;
  try{Archive invalid(bad);}catch(const std::runtime_error&){failed=true;}
  ck(failed,"Cyclic source folder parent must be rejected");
  fs::remove_all(home);
  std::cout<<"ORIGINAL_RESOURCE_VFI_FOLDER_OWNER_TEST_PASSED nested=4 stage=woods_c"
           <<" missing_parent=reject cycle=reject\n";
  return 0;
 }catch(const std::exception& e){
  fs::remove_all(home);
  std::cerr<<"ORIGINAL_RESOURCE_VFI_FOLDER_OWNER_TEST_FAILED "<<e.what()<<'\n';
  return 1;
 }
}
