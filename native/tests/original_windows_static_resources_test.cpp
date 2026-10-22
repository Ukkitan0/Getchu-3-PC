#include "ae3/original_windows_static_resources.hpp"
#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace ae3::runtime;
int main(int argc,char**argv){
 try{
  OriginalStaticResourceGlobals_3a6cd0 result{};
  std::array<std::byte,0xd0> invalid{};
  if(originalStaticCtor_3a6cd0(invalid,result) || result.initialized)
    throw std::runtime_error("Unverified bytes must not initialize original game");
  if(argc==1){
    std::cout<<"ORIGINAL_STATIC_RESOURCE_NEGATIVE_PASS\n";
    return 0;
  }
  if(argc!=2)throw std::invalid_argument("Supply PRIVATE verified 208-byte region");
  std::ifstream in(argv[1],std::ios::binary);
  if(!in)throw std::runtime_error("Private original data inaccessible");
  std::array<std::byte,0xd0> data{};
  in.read(reinterpret_cast<char*>(data.data()),data.size());
  if(in.gcount()!=std::streamsize(data.size()) || in.peek()!=EOF)
    throw std::runtime_error("Private original data region length mismatch");
  if(!originalStaticCtor_3a6cd0(data,result) || !result.initialized)
    throw std::runtime_error("Authenticated original data rejected");
  constexpr std::array<std::uint32_t,9> tags{1,1,1,2,1,1,4,3,2};
  for(std::size_t i=0;i<result.globalLiterals.size();++i)
    for(std::size_t j=0;j<16;++j)
      if(result.globalLiterals[i][j]!=data[i*16+j])
        throw std::runtime_error("Original static global literal mismatch");
  for(std::size_t i=0;i<result.entries.size();++i){
    if(result.entries[i].typeWord!=tags[i])throw std::runtime_error("Original source tag mismatch");
    for(std::size_t j=0;j<16;++j)
      if(result.entries[i].sourceLiteral[j]!=data[0x40+i*16+j])
        throw std::runtime_error("Original static field ordering mismatch");
  }
  std::cout<<"ORIGINAL_STATIC_RESOURCE_PASS original_quadwords=13 tags=9\n";
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"ORIGINAL_STATIC_RESOURCE_FAIL "<<e.what()<<"\n";return 1;
 }
}
