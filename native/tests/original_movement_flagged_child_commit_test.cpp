#include "ae3/original_movement_flagged_36fb5c.hpp"
#include "ae3/original_movement_child_30c57c.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <bitset>
#include <bit>
#include <cassert>
#include <iostream>
#include <stdexcept>
using namespace ae3;
namespace {
std::uint32_t raw(float f){return std::bit_cast<std::uint32_t>(f);}
std::unique_ptr<OriginalMovementChildD0_30c57c> ownedChild(
    float status,std::uint32_t flags) {
    std::array<std::byte,0x110> scratch{};
    std::bitset<0x110> actualKnown;
    for(unsigned i=0xA4;i<0xAC;++i) actualKnown.set(i);
    const auto s=raw(status);
    for(unsigned i=0;i<4;++i) {
        scratch[0xA4+i]=std::byte((s>>(8*i))&255u);
        scratch[0xA8+i]=std::byte((flags>>(8*i))&255u);
    }
    // Genuine original 0x30C57C source copy: original status
    // goes from scratch+A4 to child+B4, flags+A8 to child+B8.
    return originalMovementChildCreate_30c57c(
        0x123400u,scratch,actualKnown,1,2);
}
}
int main(){
    auto child=ownedChild(0.5f,2u);
    const OriginalMovementFlaggedInputs_36fb5c in{
        0x01FFF020u,raw(0.5f),2u,raw(0.125f),
        {raw(2.0f),0u,0u,0u}
    };
    const auto pass=originalMovementFlaggedPass_36fb5c(in);
    assert(pass.originalReturnF0Bits==0u);
    assert(pass.originalStatusA4AfterBits==raw(0.5625f));
    assert(pass.originalFlagsA8After==0u);
    // Fabricated caller plan cannot mutate the owned child.
    auto altered=pass;
    altered.originalStatusA4AfterBits=raw(0.9375f);
    bool rejected=false;
    try {originalMovementCommitFlaggedReturn_36fb5c(*child,in,altered);}
    catch(const std::invalid_argument&){rejected=true;}
    assert(rejected && child->originalWordAt(0xB4)==raw(0.5f) &&
           child->originalWordAt(0xB8)==2u);
    originalMovementCommitFlaggedReturn_36fb5c(*child,in,pass);
    assert(child->originalWordAt(0xB4)==raw(0.5625f));
    assert(child->originalWordAt(0xB8)==0u);
    // No reuse of a stale source pass after mutation.
    rejected=false;
    try {originalMovementCommitFlaggedReturn_36fb5c(*child,in,pass);}
    catch(const std::invalid_argument&){rejected=true;}
    assert(rejected && child->originalWordAt(0xB4)==raw(0.5625f));

    const auto continuation=originalMovementFlaggedPass_36fb5c({
        0x01FFF020u,raw(0.5f),2u,raw(0.5f),
        {raw(2.0f),0u,0u,0u}});
    assert(continuation.requiresOriginal36F8C8);
    assert(!continuation.originalReturnF0Bits);
    auto child2=ownedChild(0.5f,2u);
    rejected=false;
    try {originalMovementCommitFlaggedReturn_36fb5c(
        *child2,{0x01FFF020u,raw(0.5f),2u,raw(0.5f),
                 {raw(2.0f),0u,0u,0u}},continuation);}
    catch(const std::logic_error&){rejected=true;}
    assert(rejected && child2->originalWordAt(0xB4)==raw(0.5f) &&
           child2->originalWordAt(0xB8)==2u);
    std::cout<<"ORIGINAL_36FB5C_OWNED_CHILD_COMMIT_PASSED\n";
}
