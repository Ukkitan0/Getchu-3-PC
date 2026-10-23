#include "ae3/original_movement_regenerate_owned_36f8c8.hpp"
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
std::uint32_t word(float f){return std::bit_cast<std::uint32_t>(f);}
struct OriginalOwner final:OriginalMovementRegenerationOwner_36f8c8 {
    OriginalMovementChildD0_30c57c* child{};
    bool failRead=false,failPublish=false;
    int reads=0,published=0;
    std::array<std::array<std::uint32_t,4>,8>
    read_original_vf1_through_vf8(std::uint32_t parent) override {
        assert(parent==0x234000u);
        ++reads;
        if(failRead) throw std::runtime_error("genuine original VF source missing");
        std::array<std::array<std::uint32_t,4>,8> vf{};
        for(auto& reg:vf)for(auto& lane:reg)lane=word(1.0f);
        return vf;
    }
    void publish_original_36f8c8_vu0(
        const OriginalMovementRegenerateResult_36f8c8& value) override {
        assert(child->originalWordAt(0xB8)==0);
        assert(value.originalVF1After[0]==word(0.125f));
        assert(value.originalVF4After[3]==word(1.0f));
        if(failPublish)
            throw std::runtime_error("original VU0 publishing failed");
        ++published;
    }
};
std::unique_ptr<OriginalMovementChildD0_30c57c> create(bool known=true) {
    std::array<std::byte,0x110> scratch{};
    std::bitset<0x110> proven{};
    for(unsigned i=0xA0;i<0xB0;++i)proven.set(i);
    if(!known)for(unsigned i=0xA4;i<0xA8;++i)proven.reset(i);
    const auto pointer=0x00744DD0u;
    const auto status=word(0.5f);
    for(unsigned j=0;j<4;++j) {
        scratch[0xA0+j]=std::byte((pointer>>(8*j))&255u);
        scratch[0xA4+j]=std::byte((status>>(8*j))&255u);
    }
    return originalMovementChildCreate_30c57c(
        0x234000u,scratch,proven,1,2);
}
}
int main() {
    OriginalOwner owner{};
    auto child=create();owner.child=child.get();
    const auto result=originalMovementRegenerateOwned_36f8c8(
        *child,0x01FFF020ull,owner);
    assert(owner.reads==1 && owner.published==1);
    assert(child->originalWordAt(0xB8)==2u);
    assert(result.originalReturnV0[0]==word(0.125f));

    // Original 36FC18 branch cannot regenerate a second time
    // once retail flag bit1 is set.
    bool rejected=false;
    try{(void)originalMovementRegenerateOwned_36f8c8(
        *child,0x01FFF020ull,owner);}
    catch(const std::invalid_argument&){rejected=true;}
    assert(rejected && owner.reads==1 && owner.published==1);

    child=create();owner.child=child.get();owner.failPublish=true;
    rejected=false;
    try{(void)originalMovementRegenerateOwned_36f8c8(
        *child,0x01FFF020ull,owner);}
    catch(const std::runtime_error&){rejected=true;}
    assert(rejected && child->originalWordAt(0xB8)==0u);
    owner.failPublish=false;

    child=create(false);owner.child=child.get();
    rejected=false;
    try{(void)originalMovementRegenerateOwned_36f8c8(
        *child,0x01FFF020ull,owner);}
    catch(const std::logic_error&){rejected=true;}
    assert(rejected && owner.published==1 &&
           child->originalWordAt(0xB8)==0u);
    std::cout<<"ORIGINAL_36F8C8_OWNED_CHILD_REGEN_PASSED\n";
}
