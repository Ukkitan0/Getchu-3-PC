#include "ae3/original_movement_vu0_scalar_36f5f4.hpp"
#include "ae3/original_movement_child_30c57c.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
using namespace ae3;
namespace {
void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
void put(std::array<std::byte,16>& v,std::size_t lane,float x) {
    auto bits=std::bit_cast<std::uint32_t>(x);
    for(unsigned i=0;i<4;++i)
        v[4*lane+i]=std::byte((bits>>(8u*i))&255u);
}
}
int main() {
    std::array<std::byte,0x110> scratch{};
    OriginalMovementVU0ScalarSource_36f5f4 inputs{};
    // Genuine source vectors, not detached caller-computed f1.
    put(inputs.originalA1,0,1);put(inputs.originalA1,1,2);put(inputs.originalA1,2,3);
    put(inputs.originalA2,0,10);put(inputs.originalA2,1,2);put(inputs.originalA2,2,3);
    put(inputs.originalA3,0,1.5f);put(inputs.originalA3,1,2);put(inputs.originalA3,2,3);
    originalMovementScratchFromVU0Scalar_36f5f4(scratch,0x01FFF020u,inputs);
    auto child=originalMovementChildCreate_30c57c(0x00334400u,scratch,1,0x15);
    require(child->originalWordAt(0xB0)==0x00744dd0u,"original runtime 0x744dd0 source VA lost");
    require(child->originalWordAt(0xB4)==0x3f800000u,"true VU0 source distance branch should write 1.0");
    require(child->originalWordAt(0xB8)==0 && child->originalWordAt(0xBC)==0,
            "VU0 selector word status changed");
    // second norm exactly 1 -> C.OLT.S returns false
    put(inputs.originalA3,0,2);
    originalMovementScratchFromVU0Scalar_36f5f4(scratch,0x01FFF020u,inputs);
    child=originalMovementChildCreate_30c57c(0x00334400u,scratch,1,0x15);
    require(child->originalWordAt(0xB4)==0,"VU0 strict source comparison should reject distance squared == 1");
    // Atomic rejection on malformed source FP and guest stack context.
    scratch.fill(std::byte{0xAB});
    put(inputs.originalA3,0,std::bit_cast<float>(0x7fc01234u));
    bool rejected=false;
    try{originalMovementScratchFromVU0Scalar_36f5f4(scratch,0x01FFF020u,inputs);}
    catch(const std::invalid_argument&){rejected=true;}
    require(rejected,"unrecovered NaN VU0 behavior must reject");
    for(auto b:scratch)require(b==std::byte{0xAB},"rejected VU0 source overwrote scratch");
    put(inputs.originalA3,0,2);
    rejected=false;
    try{originalMovementScratchFromVU0Scalar_36f5f4(scratch,0x10001FFF020ull,inputs);}
    catch(const std::invalid_argument&){rejected=true;}
    require(rejected,"source 64-bit high-tagged guest stack VA accepted");
    for(auto b:scratch)require(b==std::byte{0xAB},"invalid source stack changed scratch");
    std::cout<<"ORIGINAL_VU0_SCALAR_SCRATCH_CHILD_CHAIN_PASSED\n";
}
