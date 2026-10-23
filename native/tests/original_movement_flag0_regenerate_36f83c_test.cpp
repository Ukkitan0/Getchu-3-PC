#include "ae3/original_movement_flag0_regenerate_36f83c.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <bit>
#include <bitset>
#include <iostream>
#include <stdexcept>
using namespace ae3;
namespace {
std::uint32_t raw(float f){return std::bit_cast<std::uint32_t>(f);}
struct Owner:OriginalMovementFlag0Owner_36f83c {
    int reads=0,publishes=0;
    bool failPublish=false;
    std::array<std::array<std::uint32_t,4>,8>
    read_original_vf1_through_vf8(std::uint32_t va) override {
        assert(va==0x234000u);
        ++reads;
        std::array<std::array<std::uint32_t,4>,8> vf{};
        for(auto& v:vf)for(auto& l:v)l=raw(1.0f);
        return vf;
    }
    void publish_original_36f83c_vu0(
        const OriginalMovementRegenerateResult_36f8c8& result) override {
        assert(result.originalFlagsA8After==3u);
        assert(result.originalVF1After[0]==raw(0.125f));
        if(failPublish)
            throw std::runtime_error("original VF1/VF4/ACC game owner unavailable");
        ++publishes;
    }
};
std::unique_ptr<OriginalMovementChildD0_30c57c> child(bool known=true) {
    std::array<std::byte,0x110> scratch{};
    std::bitset<0x110> provenance;
    for(unsigned i=0xA0;i<0xB0;++i)provenance.set(i);
    if(!known)for(unsigned i=0xA4;i<0xA8;++i)provenance.reset(i);
    const auto pointer=0x744DD0u,progress=raw(0.5f),flags=2u;
    for(unsigned i=0;i<4;++i) {
        scratch[0xA0+i]=std::byte((pointer>>(i*8))&255u);
        scratch[0xA4+i]=std::byte((progress>>(i*8))&255u);
        scratch[0xA8+i]=std::byte((flags>>(i*8))&255u);
    }
    return originalMovementChildCreate_30c57c(
        0x234000u,scratch,provenance,1,2);
}
}
int main() {
    Owner owner{};
    auto c=child();
    // Original source +B8 bit1 is already present; source
    // 557FC8 bit0 CLEAR -> 558084 JAL 36F83C sets bit0
    // while preserving the existing bit1, so flags become 3.
    const auto result=originalMovementRegenerateFlag0Owned_36f83c(
        *c,0x01FFF020ull,owner);
    assert(owner.reads==1 && owner.publishes==1);
    assert(c->originalWordAt(0xB8)==3u);
    assert(result.originalFlagsA8After==3u);
    assert(result.originalVF1After[0]==raw(0.125f));
    assert(result.originalReturnV0==result.originalVF1After);
    bool refused=false;
    try{(void)originalMovementRegenerateFlag0Owned_36f83c(
        *c,0x01FFF020ull,owner);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused && owner.reads==1);
    c=child();owner.failPublish=true;
    refused=false;
    try{(void)originalMovementRegenerateFlag0Owned_36f83c(
        *c,0x01FFF020ull,owner);}
    catch(const std::runtime_error&){refused=true;}
    assert(refused && c->originalWordAt(0xB8)==2u);
    c=child(false);owner.failPublish=false;
    refused=false;
    try{(void)originalMovementRegenerateFlag0Owned_36f83c(
        *c,0x01FFF020ull,owner);}
    catch(const std::logic_error&){refused=true;}
    assert(refused && c->originalWordAt(0xB8)==2u);
    std::cout<<"ORIGINAL_36F83C_FLAG0_REGENERATION_PASSED\n";
}
