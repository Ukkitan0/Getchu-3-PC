#include "ae3/original_navrange_eligibility_finish.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <stdexcept>
using namespace ae3;
static void require(bool x){if(!x)throw std::runtime_error("retail navrange optional output mismatch");}
int main(){
    const std::array<OriginalVU0Vec4,4> mat{{
        {1.f,2.f,3.f,4.f},{5.f,6.f,7.f,8.f},
        {9.f,10.f,11.f,12.f},{13.f,14.f,15.f,16.f}}};
    const OriginalVU0Vec4 query{2.f,-1.f,0.5f,1.f};
    const OriginalVU0Vec4 offset{-3.f,4.f,1.f,5.f};
    // Original 0x21F754 sets t0=0 for the real group-list path;
    // no SQC2 output is made although the scalar is returned.
    auto suppressed=originalNavrangeFinish_21f3c8(-2.25f,false,mat,query,offset);
    require(!suppressed.optionalResult.has_value());
    require(suppressed.sourceScalarF0==-2.25f);
    auto full=originalNavrangeFinish_21f3c8(7.f,true,mat,query,offset);
    require(full.sourceScalarF0==7.f && full.optionalResult.has_value());
    const auto p=*full.optionalResult;
    require(p.x==11.5f && p.y==21.f && p.z==20.5f && p.w==27.f);
    const std::array<OriginalVU0Vec4,4> eye{{
        {1.f,0.f,0.f,0.f},{0.f,1.f,0.f,0.f},
        {0.f,0.f,1.f,0.f},{0.f,0.f,0.f,1.f}}};
    auto e=originalNavrangeFinish_21f3c8(0.f,true,eye,query,{0.f,0.f,0.f,0.f});
    require(e.optionalResult.has_value());
    require(e.optionalResult->x==query.x && e.optionalResult->y==query.y);
    require(e.optionalResult->z==query.z && e.optionalResult->w==query.w);
    const float qnan=std::bit_cast<float>(0x7fc01234u);
    const float negativeZero=std::bit_cast<float>(0x80000000u);
    auto nan=originalNavrangeFinish_21f3c8(qnan,false,mat,query,offset);
    auto nz=originalNavrangeFinish_21f3c8(negativeZero,false,mat,query,offset);
    require(std::bit_cast<std::uint32_t>(nan.sourceScalarF0)==0x7fc01234u);
    require(std::bit_cast<std::uint32_t>(nz.sourceScalarF0)==0x80000000u);
    for(int i=0;i<500;i++){
        const float x=static_cast<float>(i)*0.125f;
        auto a=originalNavrangeFinish_21f3c8(x,false,eye,{x,x,x,1.f},offset);
        require(!a.optionalResult && a.sourceScalarF0==x);
    }
}
