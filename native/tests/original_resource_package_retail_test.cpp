#include "ae3/original_resource_package.hpp"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>

using namespace ae3;
static void ck(bool good,const char* message) {
 if(!good)throw std::runtime_error(message);
}
int main(int argc,char**argv) {
 try {
  if(argc!=2)throw std::invalid_argument("Supply PRIVATE original vfi/debug/us/stage/woods_c/exdb.pck; verify its SHA-256 independently");
  const std::filesystem::path file(argv[1]);
  const auto size=std::filesystem::file_size(file);
  ck(size==0x1660,"Unexpected owner-private USA woods_c EXDB package byte count");
  std::ifstream in(file,std::ios::binary);
  ck(bool(in),"Cannot open original EXDB package");
  const std::vector<char> source((std::istreambuf_iterator<char>(in)),{});
  ck(source.size()==size,"Short original EXDB package read");
  std::vector<std::byte> bytes;
  bytes.reserve(source.size());
  for(const char b:source)bytes.push_back(std::byte(static_cast<unsigned char>(b)));
  OriginalPckResourcePackage package(std::move(bytes));
  const auto members=package.entries();
  ck(members.size()==2,"Retail woods_c EXDB PCK must contain exactly two resources");
  ck(members[0].sourceIndex==0 && members[0].name=="monkeyprofile.exdb" && members[0].type=="exdb", "Authentic monkey-profile name/type mismatch");
  ck(members[0].payloadOffset==0x60 && members[0].payload.size()==0x1400, "Authentic monkey-profile offset/size mismatch");
  ck(members[1].sourceIndex==1 && members[1].name=="MainCameraParam.exdb" && members[1].type=="exdb", "Authentic camera-parameters name/type mismatch");
  ck(members[1].payloadOffset==0x1460 && members[1].payload.size()==0x200, "Authentic camera-parameters offset/size mismatch");
  ck(originalPckPayloadSignature(members[0].payload)==OriginalPckPayloadSignature::exdb,
     "Authentic monkey profile EDB\n magic mismatch");
  ck(originalPckPayloadSignature(members[1].payload)==OriginalPckPayloadSignature::exdb,
     "Authentic camera EDB\n magic mismatch");
  ck(package.selectType_26d24c("exdb").size()==2,"Source ELF 0x26D24C exdb type filter mismatch");
  ck(!package.first("bgm_desc.exdb","exdb"),"Retail woods_c source unexpectedly includes bgm descriptor");
  std::cout<<"ORIGINAL_RESOURCE_PACKAGE_RETAIL_TEST_PASSED source=woods_c/exdb.pck members=2\n";
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"ORIGINAL_RESOURCE_PACKAGE_RETAIL_TEST_FAILED: "<<e.what()<<"\n";
  return 1;
 }
}
