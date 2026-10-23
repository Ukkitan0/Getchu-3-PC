#include "ae3/original_movement_vu0_scalar_36f5f4.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
using namespace ae3;
static void put(std::array<std::byte,16>& v,unsigned n,float f){
    const auto u=std::bit_cast<std::uint32_t>(f);
    for(unsigned i=0;i<4;++i)v[4*n+i]=std::byte((u>>(8*i))&255u);
}
static float f(std::uint32_t value){return std::bit_cast<float>(value);}
int main(){
    OriginalMovementVU0ScalarSource_36f5f4 s{};
    for(unsigned i=0;i<4;++i){
        put(s.originalA1,i,float(1+i));
        put(s.originalA2,i,float(1+i));
        put(s.originalA3,i,float(5+i));
    }
    put(s.originalA2,0,1.5f);
    auto o=originalMovementVU0VectorFrame_36f5f4(s);
    assert(o.originalFirstLengthUnderOne);
    assert(f(o.scalar.originalFirstLengthSquaredBits)==0.25f);
    for(unsigned i=0;i<4;++i){
        assert(f(o.originalVF4[i])==float(3+i)); // midpoint A1,A3
        assert(f(o.originalVF3[i])==2.0f); // VF4-VF3
        assert(f(o.originalVF1[i])==2.0f); // VF5-VF4
        assert(f(o.originalVF2[i])==-float(5+i)); // VF7(0)-VF5
    }
    // False first C.OLT.S branch preserves original A2 in VF4.
    put(s.originalA2,0,3.0f);
    o=originalMovementVU0VectorFrame_36f5f4(s);
    assert(!o.originalFirstLengthUnderOne);
    assert(f(o.scalar.originalFirstLengthSquaredBits)==4);
    assert(f(o.originalVF4[0])==3.0f);
    assert(f(o.originalVF3[0])==2.0f);
    assert(f(o.originalVF1[0])==2.0f);
    for(unsigned i=1;i<4;++i){
        assert(f(o.originalVF4[i])==float(1+i));
        assert(f(o.originalVF3[i])==0.0f);
        assert(f(o.originalVF1[i])==4.0f);
    }
    // Unlike scalar reduction, this path actually consumes W;
    // unsupported PS2 VU0 W NaN must be refused, not ignored.
    put(s.originalA3,3,std::bit_cast<float>(0x7fc00000u));
    bool fail=false;
    try{(void)originalMovementVU0VectorFrame_36f5f4(s);}
    catch(const std::invalid_argument&){fail=true;}
    assert(fail);
    std::cout<<"ORIGINAL_36F5F4_VU0_VECTOR_FRAME_PASSED\n";
}
