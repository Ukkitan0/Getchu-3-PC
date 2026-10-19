#include "ae3/tim2.hpp"
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
namespace {
void check(bool ok,const char* why){
    if(!ok)throw std::runtime_error(why);
}
void put16(std::vector<std::uint8_t>& b,std::size_t at,unsigned v){
    b.at(at)=std::uint8_t(v);b.at(at+1)=std::uint8_t(v>>8);
}
void put32(std::vector<std::uint8_t>& b,std::size_t at,unsigned v){
    put16(b,at,v);put16(b,at+2,v>>16);
}
std::vector<std::uint8_t> sample(unsigned type,unsigned w,unsigned h,
                                  const std::vector<std::uint8_t>& pixels,
                                  const std::vector<std::uint8_t>& clut={},
                                  unsigned ncolors=0,unsigned clutType=3){
    std::vector<std::uint8_t> b(16+48+pixels.size()+clut.size());
    b[0]='T';b[1]='I';b[2]='M';b[3]='2';
    b[4]=4;b[5]=0;put16(b,6,1);
    put32(b,16,static_cast<unsigned>(b.size()-16));
    put32(b,20,static_cast<unsigned>(clut.size()));
    put32(b,24,static_cast<unsigned>(pixels.size()));
    put16(b,28,48);put16(b,30,ncolors);
    b[34]=std::uint8_t(clutType);
    b[35]=std::uint8_t(type);
    put16(b,36,w);put16(b,38,h);
    std::copy(pixels.begin(),pixels.end(),b.begin()+64);
    std::copy(clut.begin(),clut.end(),b.begin()+64+pixels.size());
    return b;
}
void equal(const std::vector<std::uint8_t>& got,const std::vector<std::uint8_t>& expected){
    check(got==expected,"incorrect native host RGBA texture conversion");
}
}
int main(){
    try{
        auto direct=sample(3,2,1,{255,30,60,128,20,40,80,64});
        check(ae3::isTim2(direct),"valid TIM2 not recognized");
        auto tex=ae3::decodeTim2(direct);
        check(tex.width==2 && tex.height==1,"wrong 32-bit TIM2 dimensions");
        equal(tex.rgba,{255,30,60,255,20,40,80,127});
        equal(ae3::decodeTim2(sample(2,1,1,{10,20,30})).rgba,{10,20,30,255});
        equal(ae3::decodeTim2(sample(1,1,1,{0xff,0x7f})).rgba,{255,255,255,0});

        // Indexed palettes use unmodified native RGBA32 entries in asset data.
        equal(ae3::decodeTim2(sample(5,2,1,{1,0},
             {100,50,20,128,30,120,240,64},2,3)).rgba,
             {30,120,240,127,100,50,20,255});
        equal(ae3::decodeTim2(sample(4,2,1,{0x10},
             {100,50,20,128,30,120,240,64},2,3)).rgba,
             {100,50,20,255,30,120,240,127});
        auto invalid=direct;invalid[35]=66;
        bool rejected=false;
        try{ae3::decodeTim2(invalid);}catch(const std::runtime_error&){rejected=true;}
        check(rejected,"unsupported pixel format accepted");
        invalid=direct;put32(invalid,24,100000);
        rejected=false;
        try{ae3::decodeTim2(invalid);}catch(const std::runtime_error&){rejected=true;}
        check(rejected,"truncated TIM2 pixel image accepted");
        rejected=false;
        try{ae3::decodeTim2(direct,1);}catch(const std::runtime_error&){rejected=true;}
        check(rejected,"invalid TIM2 picture index accepted");
        std::cout<<"NATIVE_TIM2_TEST_PASSED: true host pixels with no PS2 graphics simulation\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"NATIVE_TIM2_TEST_FAILED: "<<e.what()<<"\n";
        return 1;
    }
}
