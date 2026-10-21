#include "ae3/original_navrange_axis_score.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
using namespace ae3;
static void ck(bool okay){if(!okay)throw std::runtime_error("Original navrange score regression");}
int main(){
    const std::array<float,3> bounds{2.f,3.f,4.f};
    ck(originalNavrangeAxisScore_21f20c({0.f,0.f,0.f},bounds)==0.f);
    ck(originalNavrangeAxisScore_21f20c({2.f,-3.f,4.f},bounds)==0.f);
    ck(originalNavrangeAxisScore_21f20c({-5.f,8.f,7.f},bounds)==43.f);
    ck(originalNavrangeAxisScore_21f20c({-2.5f,4.f,-4.f},bounds)==1.25f);
    ck(originalNavrangeAxisScore_21f20c({10.f,-3.f,4.f},bounds)==64.f);
    const float nan=std::numeric_limits<float>::quiet_NaN();
    // PS2 C.OLT.S is false for unordered: no branch in that axis.
    ck(originalNavrangeAxisScore_21f20c({nan,5.f,0.f},bounds)==4.f);
    ck(originalNavrangeAxisScore_21f20c({0.f,0.f,0.f},{nan,3.f,4.f})==0.f);
    const OriginalNavrangePacked4 bits{0x00000000u,0x80000000u,0x7fc01234u,0xff800000u};
    for(std::size_t i=0;i<bits.size();++i)
        ck(std::bit_cast<std::uint32_t>(originalNavrangeLaneGet_44bd5c(bits,i))==bits[i]);
    for(std::size_t i=0;i<bits.size();++i){
        const auto changed=originalNavrangeLaneSet_44bd80(bits,i,1.5f);
        for(std::size_t j=0;j<bits.size();++j)
            ck(changed[j]==(i==j?0x3fc00000u:bits[j]));
    }
    bool invalid=false;
    try{(void)originalNavrangeLaneGet_44bd5c(bits,4);}catch(const std::out_of_range&){invalid=true;}
    ck(invalid);
    invalid=false;
    try{(void)originalNavrangeLaneSet_44bd80(bits,999,0.f);}catch(const std::out_of_range&){invalid=true;}
    ck(invalid);
}
