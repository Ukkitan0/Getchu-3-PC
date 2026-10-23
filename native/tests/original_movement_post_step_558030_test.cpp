#include "ae3/original_movement_post_step_558030.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <array>
#include <bit>
#include <bitset>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
namespace {
std::uint32_t bits(float f){return std::bit_cast<std::uint32_t>(f);}
std::unique_ptr<OriginalMovementChildD0_30c57c> makeChild(float progress,bool known=true) {
    std::array<std::byte,0x110> scratch{};
    std::bitset<0x110> knownBits{};
    for(unsigned i=0xA4;i<0xA8;++i)if(known)knownBits.set(i);
    const auto value=bits(progress);
    for(unsigned i=0;i<4;++i)
        scratch[0xA4+i]=std::byte((value>>(8*i))&255u);
    return originalMovementChildCreate_30c57c(
        0x123400u,scratch,knownBits,1,3);
}
struct Owner:OriginalMovementPostStepOwner_558030 {
    std::vector<std::string> operations{};
    std::uint32_t published{};
    std::uint64_t expectedS5=0x450000u;
    std::uint32_t parentWord1CBits=0x880000u;
    std::uint64_t expectedA0=0x880000u;
    std::uint32_t read_original_parent_word1c(std::uint32_t va) override {
        assert(va==0x123400u);
        operations.emplace_back("read1c");
        return parentWord1CBits;
    }
    void call_original_314c44(
        std::uint64_t p,std::uint64_t s5,std::uint32_t mode) override {
        assert(p==expectedA0 && s5==expectedS5 && mode==3u);
        operations.emplace_back("314c44");
    }
    void store_original_parent_float70(
        std::uint32_t va,std::uint32_t raw) override {
        assert(va==0x123400u);
        published=raw;
        operations.emplace_back("store70");
    }
};
}
int main() {
    Owner o{};
    const auto finished=makeChild(1.0f);
    assert(originalMovementPostStep_558030(
        *finished,{bits(2.0f),bits(4.0f),std::nullopt},o)==2u);
    assert(o.published==bits(0.5f));
    assert(o.operations==std::vector<std::string>{"store70"});
    o.operations.clear();
    assert(originalMovementPostStep_558030(
        *makeChild(1.25f),{bits(-3.0f),bits(6.0f),std::nullopt},o)==2u);
    assert(o.published==bits(-0.5f));
    o.operations.clear();
    // The BEQZL parent+0x1C delay-slot load runs only when
    // progress <1; the original 0x314C44 call uses s5 and mode.
    assert(originalMovementPostStep_558030(
        *makeChild(0.875f),{bits(2),bits(4),0x450000u},o)==0u);
    assert(o.operations==std::vector<std::string>({"read1c","314c44"}));
    o.operations.clear();
    // Original 558070 DADDU a1,s5,zero must retain upper 32 bits.
    o.expectedS5=0x12345678450000ull;
    assert(originalMovementPostStep_558030(
        *makeChild(0.875f),{bits(2),bits(4),o.expectedS5},o)==0u);
    assert(o.operations==std::vector<std::string>({"read1c","314c44"}));
    o.expectedS5=0x450000u;
    o.operations.clear();
    // Original 558054 is LW a0,parent+1C: a negative 32-bit
    // source memory word MUST reach 314C44 sign-extended in EE a0.
    // This source field is a word read, not a host pointer.
    o.parentWord1CBits=0x81234000u;
    o.expectedA0=0xffffffff81234000ull;
    assert(originalMovementPostStep_558030(
        *makeChild(0.875f),{bits(2),bits(4),o.expectedS5},o)==0u);
    assert(o.operations==std::vector<std::string>({"read1c","314c44"}));
    o.parentWord1CBits=0x880000u;
    o.expectedA0=0x880000u;
    o.operations.clear();
    bool refused=false;
    try {(void)originalMovementPostStep_558030(
        *makeChild(0.5f),{0u,0u,std::nullopt},o);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused && o.operations.empty());
    refused=false;
    try {(void)originalMovementPostStep_558030(
        *makeChild(1.0f,false),{bits(1),bits(2),0x450000u},o);}
    catch(const std::logic_error&){refused=true;}
    assert(refused && o.operations.empty());
    // Reject wrong PS2 special COP1 semantics, not guessed IEEE.
    for(const auto input:{
            OriginalMovementPostStep_558030{bits(1),bits(3),std::nullopt},
            OriginalMovementPostStep_558030{bits(1),0u,std::nullopt},
            OriginalMovementPostStep_558030{0x7FC00000u,bits(2),std::nullopt}}) {
        refused=false;
        try{(void)originalMovementPostStep_558030(*finished,input,o);}
        catch(const std::exception&){refused=true;}
        assert(refused && o.operations.empty());
    }
    std::cout<<"ORIGINAL_558030_POST_STEP_STATUS_PASSED\n";
}
