#include "ae3/original_windows_static_resources.hpp"
#include <array>
#include <filesystem>
#include <iostream>
#include <stdexcept>
using namespace ae3::runtime;
int main(int argc,char**argv){
 try{
  bool rejected=false;
  try{(void)originalReadStaticSourceFromElf_3a6cd0("/missing/private/SCUS_975.01");}
  catch(const std::runtime_error&){rejected=true;}
  if(!rejected)throw std::runtime_error("Absent original ELF accepted");
  if(argc==1){std::cout<<"ORIGINAL_ELF_STATIC_SOURCE_GUARD_PASS\n";return 0;}
  if(argc!=2)throw std::invalid_argument("Private original USA ELF argument expected");
  auto region=originalReadStaticSourceFromElf_3a6cd0(argv[1]);
  OriginalStaticResourceGlobals_3a6cd0 state{};
  if(!originalStaticCtor_3a6cd0(region,state) || !state.initialized)
      throw std::runtime_error("Original third constructor rejected source region");
  auto invalid=region;invalid[5]^=std::byte{0x80};
  if(originalStaticCtor_3a6cd0(invalid,state))
      throw std::runtime_error("Modified source region passed");
  std::cout<<"ORIGINAL_ELF_STATIC_SOURCE_MAPPING_PASS\n";
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"ORIGINAL_ELF_STATIC_SOURCE_MAPPING_FAIL: "<<e.what()<<"\n";
  return 1;
 }
}
