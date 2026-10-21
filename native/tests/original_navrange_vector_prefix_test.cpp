#include "ae3/original_navrange_vector_prefix.hpp"
#include <bit>
#include <cstdint>
#include <stdexcept>
using namespace ae3;
static void check(bool ok){if(!ok)throw std::runtime_error("Original navrange vector prefix test failed");}
int main(){
    static_assert(noexcept(originalNavrangeBarrier_441f40()));
    originalNavrangeBarrier_441f40();
    auto r=originalNavrangePrefix_359ea4(
      {1.f,2.f,3.f,4.f},{2.f,3.f,4.f,5.f},
      {1.f,2.f,3.f,4.f},{4.f,5.f,6.f,7.f});
    check(r.vf1.x==2.f&&r.vf1.y==4.f&&r.vf1.z==6.f&&r.vf1.w==8.f);
    check(r.vf2.x==-2.f&&r.vf2.y==3.f&&r.vf2.z==-4.f&&r.vf2.w==5.f);
    check(r.vf3.x==1.f&&r.vf3.y==-2.f&&r.vf3.z==-3.f&&r.vf3.w==4.f);
    check(r.vf4.x==-4.f&&r.vf4.y==-5.f&&r.vf4.z==6.f&&r.vf4.w==7.f);
    check(r.vf5.x==-13.f&&r.vf5.y==24.f&&r.vf5.z==2.f&&r.vf5.w==-4.f);
    check(r.vf6.x==1.f&&r.vf6.y==1.f&&r.vf6.z==1.f&&r.vf6.w==2.f);
    const float nz=std::bit_cast<float>(0x80000000u);
    const float qnan=std::bit_cast<float>(0x7fc01234u);
    const auto s=originalNavrangePrefix_359ea4(
      {1.f,2.f,3.f,4.f},{2.f,nz,3.f,qnan},
      {nz,2.f,3.f,qnan},{4.f,5.f,nz,qnan});
    check(std::bit_cast<std::uint32_t>(s.vf2.y)==0x80000000u);
    check(std::bit_cast<std::uint32_t>(s.vf2.w)==0x7fc01234u);
    check(std::bit_cast<std::uint32_t>(s.vf3.x)==0x80000000u);
    check(std::bit_cast<std::uint32_t>(s.vf4.z)==0x80000000u);
    check(std::bit_cast<std::uint32_t>(s.vf4.w)==0x7fc01234u);
}
