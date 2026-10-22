#include "ae3/original_actor_controller_vector_compare.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool p,const char*msg){if(!p)throw std::runtime_error(msg);}
static std::array<std::byte,16> packWords(std::array<std::uint32_t,4> words){
    std::array<std::byte,16> output{};
    std::memcpy(output.data(),words.data(),16);return output;
}
static void trial(const std::array<std::byte,16>& a,const std::array<std::byte,16>& b){
    const bool expected=std::memcmp(a.data(),b.data(),16)==0;
    const auto result=originalActorControllerCompare_57c1d4(true,a,b);
    ck(result.originalBitwiseEqual==expected&&result.originalReturnV0==(expected?2u:1u),
       "Original PCEQW/PPACH/SLTIU return is not bitwise exact");
    const auto disabled=originalActorControllerCompare_57c1d4(false,a,b);
    ck(disabled.originalReturnV0==2,"Original variant-disabled branch result");
}
static unsigned u16(const std::vector<unsigned char>& b,unsigned a){
    ck(a+2<=b.size(),"PCK read bounds");return b[a]|(unsigned(b[a+1])<<8);
}
static unsigned u32(const std::vector<unsigned char>& b,unsigned a){return u16(b,a)|(u16(b,a+2)<<16);}
int main(int argc,char**argv){
    ck(argc==1||argc==2,"Optional genuine owner-private PCK path");
    const auto all=packWords({0xffffffffu,0u,0x7fc00000u,0x80000000u});
    trial(all,all);
    const auto zeros=packWords({0,0,0,0});
    const auto signedZero=packWords({0x80000000u,0,0,0});
    trial(zeros,signedZero);
    const auto nan1=packWords({0x7fc00000u,0,0,0});
    const auto nan2=packWords({0x7fc00001u,0,0,0});
    trial(nan1,nan1);trial(nan1,nan2);
    // Every individual raw EE lane participates in the source comparison.
    for(unsigned i=0;i<16;++i){auto changed=all;changed[i]^=std::byte{1};trial(all,changed);}
    if(argc==1)return 0;
    std::ifstream f(argv[1],std::ios::binary);
    ck(bool(f),"Private PCK missing");
    const std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(f)),{});
    ck(bytes.size()==3328,"Source PCK size differs");
    unsigned checked=0;
    for(unsigned g=0;g<2;++g){
        const unsigned start=g?2224:96,len=g?1104:2128;
        const std::vector<unsigned char> b(bytes.begin()+start,bytes.begin()+start+len);
        const auto n=u32(b,0x24),desc=u32(b,0x3c),vert=u32(b,0x40);
        ck(n==(g?20u:39u),"Source polygon count");
        for(unsigned p=0;p<n;++p){
            std::array<std::array<std::byte,16>,3> v{};
            for(unsigned i=0;i<3;++i){
                const unsigned o=vert+16*u16(b,desc+30*p+0x18+2*i);
                ck(o+16<=b.size(),"Original source vertex invalid");
                std::memcpy(v[i].data(),b.data()+o,16);
            }
            for(unsigned i=0;i<3;++i){
                trial(v[i],v[i]);trial(v[i],v[(i+1)%3]);
                for(unsigned k=0;k<16;++k){auto changed=v[i];changed[k]^=std::byte{1};trial(v[i],changed);}
            }
            ++checked;
        }
    }
    ck(checked==59,"All 59 genuine polygons checked");
}
