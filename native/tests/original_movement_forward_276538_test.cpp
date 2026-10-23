#include "ae3/original_movement_forward_276538.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
using namespace ae3;

namespace {
struct RealTargetOwner final:OriginalMovementForwardOwner_274988 {
    int calls=0;
    bool fail=false;
    std::uint64_t target=0;
    std::array<std::uint32_t,4> observed{};
    void call_original_274988(
        std::uint64_t originalA0,
        const std::array<std::uint32_t,4>& originalStackVector) override {
        ++calls;
        target=originalA0;
        observed=originalStackVector;
        if(fail)
            throw std::runtime_error(
                "real original 274988 game owner refused source VU0 state");
    }
};
}
int main() {
    RealTargetOwner owner;
    // Complete 128-bit EE a1 SQ into source stack, no low 64-bit
    // narrowing or host pointer reinterpretation.
    const std::array<std::uint32_t,4> originalS3{
        0x3f800000u,0xbf800000u,0x81234567u,0xabcdef01u};
    originalMovementForward_276538(0x340000u,originalS3,owner);
    assert(owner.calls==1 && owner.target==0x340050u &&
           owner.observed==originalS3);
    // Source 557E68 LW s4 sign-extends, then 27653C ADDIU
    // preserves a canonical negative EE pointer after +0x50.
    originalMovementForward_276538(
        0xffffffff81234000ull,originalS3,owner);
    assert(owner.calls==2 && owner.target==0xffffffff81234050ull &&
           owner.observed==originalS3);
    // ADDIU is 32-bit wrap, NOT 64-bit host pointer addition.
    originalMovementForward_276538(
        0x7fffffc0ull,originalS3,owner);
    assert(owner.calls==3 && owner.target==0xffffffff80000010ull);
    originalMovementForward_276538(
        0xffffffffffffffd0ull,originalS3,owner);
    assert(owner.calls==4 && owner.target==0x20ull);
    // A zero-extended high-bit parent is impossible from retail
    // source 557E68 LW, so reject before calling 274988.
    bool refused=false;
    try{originalMovementForward_276538(
        0x81234000ull,originalS3,owner);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused && owner.calls==4);
    refused=false;
    try{originalMovementForward_276538(
        0x100340000ull,originalS3,owner);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused && owner.calls==4);
    // The underlying original 274988 owner still owns its
    // effects. Its failure must propagate, never masquerade
    // as a successful original vector update.
    owner.fail=true;
    refused=false;
    try{originalMovementForward_276538(
        0x340000ull,originalS3,owner);}
    catch(const std::runtime_error&){refused=true;}
    assert(refused && owner.calls==5);
    std::cout<<"ORIGINAL_276538_SOURCE_VECTOR_FORWARD_PASSED\n";
}
