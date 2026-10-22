#include "ae3/original_render_visibility.hpp"
#include <iostream>
#include <stdexcept>
namespace {
void check(bool v,const char* m){if(!v)throw std::runtime_error(m);}
void test(){
    ae3::OriginalRenderEarlyGate a{0x100000000ULL,0x100000000ULL,1,1,0};
    int calls=0;
    const auto originalVisible=[&]{++calls;return true;};
    check(ae3::passesOriginalRetailRenderEarlyGate(a,originalVisible),"64-bit overlap should pass");
    check(!calls,"bounds was called without culling flag");
    a.sceneMask=1;
    check(!ae3::passesOriginalRetailRenderEarlyGate(a,originalVisible),"wrong scene visibility group");
    a.sceneMask=a.objectMask;a.flag30=0;
    check(!ae3::passesOriginalRetailRenderEarlyGate(a,originalVisible),"first source enable bit missing");
    a.flag30=1;a.flag35=0;
    check(!ae3::passesOriginalRetailRenderEarlyGate(a,originalVisible),"second source enable bit missing");
    a.flag35=1;a.flag32=1;
    check(ae3::passesOriginalRetailRenderEarlyGate(a,originalVisible)&&calls==1,
          "retail nonzero bounds result must continue draw");
    check(!ae3::passesOriginalRetailRenderEarlyGate(a,[]{return false;}),
          "retail bounds allow must reach next gate");
    bool missing=false;
    try{ae3::passesOriginalRetailRenderEarlyGate(a,{});}
    catch(const std::runtime_error&){missing=true;}
    check(missing,"missing original bounds function must not be guessed");
}
}
int main(){
    try{test();std::cout<<"ORIGINAL_RETAIL_RENDER_GATE_PASS: mask64 flags30_35 optional_cull32 early_skip\n";}
    catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
