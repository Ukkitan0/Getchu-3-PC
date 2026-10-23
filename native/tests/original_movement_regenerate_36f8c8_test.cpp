#include "ae3/original_movement_regenerate_36f8c8.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <bit>
#include <iostream>
#include <stdexcept>
using namespace ae3;
namespace {
std::uint32_t word(float x){return std::bit_cast<std::uint32_t>(x);}
OriginalMovementRegenerateSource_36f8c8 sample() {
    OriginalMovementRegenerateSource_36f8c8 s{};
    s.originalChildScratchGuestVA=0x01fff020;
    s.originalResourcePointerRegA0=0x744dd0;
    s.originalStatusA4Bits=word(0.5f);
    s.originalFlagsA8=4u;
    for(auto& reg:s.originalVF1ThroughVF8)
        for(auto& lane:reg) lane=0;
    for(unsigned i=0;i<4;++i) {
        s.originalVF1ThroughVF8[0][i]=word(1.0f); // old VF1
        s.originalVF1ThroughVF8[1][i]=word(2.0f); // VF2
        s.originalVF1ThroughVF8[2][i]=word(3.0f); // VF3
        s.originalVF1ThroughVF8[4][i]=word(4.0f); // VF5
        s.originalVF1ThroughVF8[5][i]=word(6.0f); // VF6
        s.originalVF1ThroughVF8[6][i]=word(7.0f); // VF7
        s.originalVF1ThroughVF8[7][i]=word(8.0f); // VF8
    }
    return s;
}
}
int main() {
    auto src=sample();
    const auto r=originalMovementRegenerate_36f8c8(src);
    const std::array<std::uint32_t,4> polynomial{
        word(0.125f),word(0.25f),word(0.5f),word(1.0f)};
    assert(r.originalVF1After==polynomial);
    assert(r.originalVF4After==polynomial);
    assert(r.originalReturnV0==polynomial);
    assert(r.originalFlagsA8After==6u);
    // First ACC: old VF1*.125 + VF2*.25+VF5*.5-VF3*1.
    // Second ACC: VF8*.125+VF7*.25+VF6*.5-VF4*1.
    for(int lane=0;lane<4;++lane) {
        assert(r.originalACCFirstStage[lane]==word(-0.375f));
        assert(r.originalACCAfter[lane]==
               word(5.75f-std::bit_cast<float>(polynomial[lane])));
    }
    // At source 36F918 first VMULAx precedes 36F924 QMTC2,
    // so it must see the OLD VF1, not the packed polynomial.
    src.originalVF1ThroughVF8[0][0]=word(8.0f);
    const auto updated=originalMovementRegenerate_36f8c8(src);
    assert(updated.originalACCFirstStage[0]==word(0.5f));
    assert(updated.originalVF1After==polynomial);
    src.originalResourcePointerRegA0=0x100744dd0ull;
    bool bad=false;
    try{(void)originalMovementRegenerate_36f8c8(src);}
    catch(const std::invalid_argument&){bad=true;}
    assert(bad);
    // Genuine original 36F8EC LW t7,a0+0xA0 is sign-extended
    // into a 64-bit EE register. Both canonical negative
    // and wrongly zero-extended negative guest source pointers
    // select an unported tagged/alternate instruction path.
    src=sample();src.originalResourcePointerRegA0=0x81234000ull;
    bad=false;
    try{(void)originalMovementRegenerate_36f8c8(src);}
    catch(const std::invalid_argument&){bad=true;}
    assert(bad);
    src=sample();src.originalResourcePointerRegA0=0xffffffff81234000ull;
    bad=false;
    try{(void)originalMovementRegenerate_36f8c8(src);}
    catch(const std::invalid_argument&){bad=true;}
    assert(bad);
    src=sample();src.originalChildScratchGuestVA=0x10001fff020ull;
    bad=false;
    try{(void)originalMovementRegenerate_36f8c8(src);}
    catch(const std::invalid_argument&){bad=true;}
    assert(bad);
    // Progress 2^-35 yields polynomial cube 2^-105 and square
    // 2^-70. Double addition loses the cube, but VU0 ADD is
    // inexact: this unsupported rounding must refuse the frame.
    src=sample();src.originalStatusA4Bits=word(0x1p-35f);
    bad=false;
    try{(void)originalMovementRegenerate_36f8c8(src);}
    catch(const std::domain_error&){bad=true;}
    assert(bad);
    // 36F934 VSUBAw also must not lose a genuine tiny VF3.
    // The earlier ACC value is 2.625; subtracting 2^-70
    // is inexact even though the binary64 difference is 2.625.
    src=sample();
    src.originalVF1ThroughVF8[2][0]=word(0x1p-70f);
    bad=false;
    try{(void)originalMovementRegenerate_36f8c8(src);}
    catch(const std::domain_error&){bad=true;}
    assert(bad);
    src=sample();src.originalStatusA4Bits=0x7fc01234u;
    bad=false;
    try{(void)originalMovementRegenerate_36f8c8(src);}
    catch(const std::invalid_argument&){bad=true;}
    assert(bad);
    std::cout<<"ORIGINAL_MOVEMENT_36F8C8_ORDINARY_VU0_PASSED\n";
}
