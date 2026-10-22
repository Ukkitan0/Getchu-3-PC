#include "ae3/original_windows_static_resources.hpp"
#include <array>
#include <iostream>
#include <stdexcept>
using namespace ae3::runtime;
int main(int argc,char**argv){
 try {
  OriginalFourthGlobals_39fc78 output{};
  std::array<std::byte,kFourthCtorLiteralBytes_39fc78> missing{};
  if(originalFourthCtor_39fc78(missing,output)||output.initialized)
      throw std::runtime_error("Unverified fourth source bytes were accepted");
  if(argc==1){
      std::cout<<"ORIGINAL_FOURTH_NATIVE_SOURCE_NEGATIVE_PASS\n";return 0;
  }
  if(argc!=2)throw std::invalid_argument("Pass private USA original ELF");
  auto source=originalReadFourthSourceFromElf_39fc78(argv[1]);
  if(!originalFourthCtor_39fc78(source,output)||!output.initialized)
      throw std::runtime_error("Original 0x39FC78 native source data rejected");
  for(std::size_t i=0;i<38;++i)
      for(std::size_t j=0;j<16;++j)
          if(output.namesAndParameters[i][j]!=source[i*16+j])
              throw std::runtime_error("Source 0x39FA10 native SQ output differs");
  source[32]^=std::byte{1};
  if(originalFourthCtor_39fc78(source,output))
      throw std::runtime_error("Modified original source silently accepted");
  std::cout<<"ORIGINAL_FOURTH_NATIVE_SOURCE_PASS 38 qwords\n";
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"ORIGINAL_FOURTH_NATIVE_SOURCE_FAILED: "<<e.what()<<"\n";return 1;
 }
}
