#include "ae3/original_movement_step_fast_36fb18.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
using namespace ae3;
static std::unique_ptr<OriginalMovementChildD0_30c57c> child(
    std::uint32_t raw,bool known=true) {
    std::array<std::byte,0x110> scratch{};
    std::bitset<0x110> mask;
    if(known) for(std::size_t i=0xA4;i<0xA8;++i)mask.set(i);
    for(std::size_t i=0;i<4;++i)
        scratch[0xA4+i]=std::byte((raw>>(i*8))&255u);
    return originalMovementChildCreate_30c57c(
        0x123000u,scratch,mask,1,2);
}
int main() {
    const auto one=std::bit_cast<std::uint32_t>(1.0f);
    const auto oneHalf=std::bit_cast<std::uint32_t>(1.5f);
    const auto two=std::bit_cast<std::uint32_t>(2.0f);
    const auto negOne=std::bit_cast<std::uint32_t>(-1.0f);
    auto complete=child(one);
    // Original threshold 1.0 and status>=threshold: MOV f0,f12
    // in source 36FB48 delay slot; original input bits preserved.
    assert(originalMovementStepFast_36fb18(*complete,two)==two);
    assert(originalMovementStepFast_36fb18(*child(two),negOne)==negOne);
    assert(originalMovementStepFast_36fb18(*child(oneHalf),two)==std::nullopt);
    // Nonpositive f12 + child status below threshold:
    // BC1FL source 36FB54 returns exact *positive zero*.
    assert(originalMovementStepFast_36fb18(*child(0u),0u)==0u);
    assert(originalMovementStepFast_36fb18(*child(0u),0x80000000u)==0u);
    assert(originalMovementStepFast_36fb18(*child(0u),negOne)==0u);
    // Unknown original copied status+0xB4 cannot be invented.
    bool rejected=false;
    try {(void)originalMovementStepFast_36fb18(*child(one,false),two);}
    catch(const std::logic_error&){rejected=true;}
    assert(rejected);
    rejected=false;
    try {(void)originalMovementStepFast_36fb18(*child(0x7fc01234u),two);}
    catch(const std::invalid_argument&){rejected=true;}
    assert(rejected);
    std::cout<<"ORIGINAL_MOVEMENT_36FB18_FAST_RETURNS_PASSED\n";
}
