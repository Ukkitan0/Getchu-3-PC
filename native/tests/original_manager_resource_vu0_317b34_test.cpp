#include "ae3/original_manager_resource_vu0_317b34.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
using namespace ae3;
int main() {
    OriginalManagerResourceFloatFields_317b34 parent{
        0x00700000u,0u,std::nullopt,std::nullopt};
    OriginalResourceVU0Inputs_317b34 v{
        0x00234000u,
        {0x40400000u,0x40800000u,0u,0u},
        0x40000000u, // genuine source table+0x364 = +2.0
        0x41200000u  // original table+0x368 cap = +10.0
    };
    auto result=originalManagerResourceVU0_317b34(parent,1,v);
    // Source VF1=(3,4,0,0) => squared length 25;
    // original sqrt 5 + resource offset 2 -> MIN(7,10)=7.
    assert(result.originalF0Raw==std::bit_cast<std::uint32_t>(7.0f));
    assert(result.originalVF1After==
           (std::array<std::uint32_t,4>{
               std::bit_cast<std::uint32_t>(25.0f),
               std::bit_cast<std::uint32_t>(32.0f),
               0u,std::bit_cast<std::uint32_t>(16.0f)}));
    v.originalVF1Raw[0]=std::bit_cast<std::uint32_t>(6.0f);
    v.originalVF1Raw[1]=std::bit_cast<std::uint32_t>(8.0f);
    result=originalManagerResourceVU0_317b34(parent,0x101u,v);
    // Original MAX cap via MIN.S in the JR delay slot.
    assert(result.originalF0Raw==std::bit_cast<std::uint32_t>(10.0f));
    // Original branch is not reached if type one / zero mode.
    bool refused=false;
    parent.originalParentWord40=1;
    try{(void)originalManagerResourceVU0_317b34(parent,1,v);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused);
    parent.originalParentWord40=0;
    refused=false;
    try{(void)originalManagerResourceVU0_317b34(parent,0x100u,v);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused);
    // Other guest pointer tag values take genuine 0x317CDC,
    // so cannot use this source continuation.
    v.originalParentGuestVA=0x100234000ull; refused=false;
    try{(void)originalManagerResourceVU0_317b34(parent,1,v);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused);
    // 317B58 sees the full signed EE parent register. A
    // caller-supplied zero-extended 0x81234000 would bypass
    // the original tagged path if accepted as ordinary.
    v.originalParentGuestVA=0x81234000ull;refused=false;
    try{(void)originalManagerResourceVU0_317b34(parent,1,v);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused);
    v.originalParentGuestVA=0xffffffff81234000ull;refused=false;
    try{(void)originalManagerResourceVU0_317b34(parent,1,v);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused);
    v.originalParentGuestVA=0x234000u;
    // Source special NaNs, subnormals and inexact hardware
    // rounding are NOT silently computed as host IEEE floats.
    v.originalVF1Raw[0]=0x7fc00001u;refused=false;
    try{(void)originalManagerResourceVU0_317b34(parent,1,v);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused);
    v.originalVF1Raw[0]=std::bit_cast<std::uint32_t>(1.0f);
    v.originalVF1Raw[1]=std::bit_cast<std::uint32_t>(1.0f);
    refused=false;
    try{(void)originalManagerResourceVU0_317b34(parent,1,v);}
    catch(const std::domain_error&){refused=true;}
    assert(refused && "source sqrt(2) needs non-host COP1 rounding");
    // Binary64 loses 2^-70 when added to 1. The original
    // VADD cannot be declared exact just because double agrees.
    v.originalVF1Raw[0]=std::bit_cast<std::uint32_t>(1.0f);
    v.originalVF1Raw[1]=std::bit_cast<std::uint32_t>(0x1p-35f);
    refused=false;
    try{(void)originalManagerResourceVU0_317b34(parent,1,v);}
    catch(const std::domain_error&){refused=true;}
    assert(refused);
    std::cout<<"ORIGINAL_RESOURCE_VU0_317B34_EXACT_SUBSET_PASSED\n";
}
