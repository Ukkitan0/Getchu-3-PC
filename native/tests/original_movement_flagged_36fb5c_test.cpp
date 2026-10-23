#include "ae3/original_movement_flagged_36fb5c.hpp"
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
namespace {
std::uint32_t raw(float f) {return std::bit_cast<std::uint32_t>(f);}
OriginalMovementFlaggedInputs_36fb5c genuine(float status,float movement) {
    return {0x01FFF020u,raw(status),2u,raw(movement),
            {raw(2),0u,0u,0u}};
}
}
int main() {
    // Source 36FB70..36FB84: VF1=(2,0,0,0) ->
    // squared VF1=(4,0,0,0), length=2, chunk=0.25.
    auto r=originalMovementFlaggedPass_36fb5c(genuine(0.5f,0.125f));
    assert(r.originalVF1After[0]==raw(4.0f));
    assert(r.originalStatusA4AfterBits==raw(0.5625f));
    assert(r.originalFlagsA8After==0);
    assert(r.originalReturnF0Bits==0u);
    assert(r.originalRemainingF20Bits==0);
    assert(!r.requiresOriginal36F8C8);

    // Status reaches exactly 1 via small-movement 36FBB8 DIV.
    r=originalMovementFlaggedPass_36fb5c(genuine(0.875f,0.25f));
    assert(r.originalStatusA4AfterBits==raw(1.0f));
    assert(r.originalReturnF0Bits==0u);

    // 36FBD8 subtract threshold and 36FBF0 multiplies
    // overflow by the genuine VF1 length.
    r=originalMovementFlaggedPass_36fb5c(genuine(0.9375f,0.25f));
    assert(r.originalStatusA4AfterBits==raw(1.0f));
    assert(r.originalReturnF0Bits==raw(0.125f));

    // Big movement: source subtracts one 1/8 segment and
    // increments +0xA4 by precisely 1/8 before threshold gate.
    r=originalMovementFlaggedPass_36fb5c(genuine(0.875f,0.5f));
    assert(r.originalReturnF0Bits==raw(0.25f));
    assert(r.originalStatusA4AfterBits==raw(1.0f));
    assert(r.originalRemainingF20Bits==raw(0.25f));
    r=originalMovementFlaggedPass_36fb5c(genuine(0.5f,0.5f));
    assert(r.originalStatusA4AfterBits==raw(0.625f));
    assert(r.originalRemainingF20Bits==raw(0.25f));
    assert(!r.originalReturnF0Bits);
    assert(r.requiresOriginal36F8C8);

    auto bad=genuine(0.5f,0.125f);
    bad.originalChildFlagsA8=0u;
    bool threw=false;
    try {(void)originalMovementFlaggedPass_36fb5c(bad);}
    catch(const std::invalid_argument&){threw=true;}
    assert(threw);
    bad=genuine(0.5f,0.125f);
    bad.originalChildScratchGuestVA=0x100000000ull|0x1FFF020u;
    threw=false;
    try {(void)originalMovementFlaggedPass_36fb5c(bad);}
    catch(const std::invalid_argument&){threw=true;}
    assert(threw);
    bad=genuine(0.5f,0.125f);
    bad.originalLiveVF1[0]=raw(1.0f);
    bad.originalLiveVF1[1]=raw(1.0f); // sqrt(2) inexact
    threw=false;
    try {(void)originalMovementFlaggedPass_36fb5c(bad);}
    catch(const std::domain_error&){threw=true;}
    assert(threw);
    // 1 + 2^-70 rounds to 1 even in binary64. It is NOT an
    // exact VU0 VADD result and cannot enter the recovered-only path.
    bad=genuine(0.5f,0.125f);
    bad.originalLiveVF1={raw(1.0f),raw(0x1p-35f),0u,0u};
    threw=false;
    try {(void)originalMovementFlaggedPass_36fb5c(bad);}
    catch(const std::domain_error&){threw=true;}
    assert(threw);
    bad=genuine(0.5f,0.125f);
    bad.originalLiveVF1[0]=0x7fc01234u;
    threw=false;
    try {(void)originalMovementFlaggedPass_36fb5c(bad);}
    catch(const std::invalid_argument&){threw=true;}
    assert(threw);
    std::cout<<"ORIGINAL_36FB5C_FLAGGED_MOVEMENT_PASSED\n";
}
