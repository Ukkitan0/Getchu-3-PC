#include "ae3/original_game_random.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <vector>
using ae3::OriginalGameRandom_36f3c0;
static void check(bool ok,const char* label){if(!ok)throw std::runtime_error(label);}
static void verifySeed(std::uint32_t seed,const std::array<std::uint32_t,8>& expected){
    OriginalGameRandom_36f3c0 r(seed);
    check(r.originalCursor()==623,"Original 623 seeding twists");
    for(auto word:expected)
        check(word==r.originalTemperedWord_36f444(),"Original MT source output mismatch");
    check(r.originalCursor()==7,"Original MT cursor wrap");
}
int main(){
    verifySeed(5489,{0x6e2cee0a,0xd091bb5c,0x22ae9ef6,0xe7e1faee,
                     0xd5c31f79,0x2082352c,0xf807b7df,0xe9d30005});
    verifySeed(1,{0x18bff9da,0x6ac1f425,0xff4780eb,0xb8672f8c,
                  0xeebc1448,0x00077eff,0x20ccc389,0x4d65aacb});
    verifySeed(0,{0x477b7b2d,0x8c7f0aac,0x97c4aa2f,0xb716a675,
                  0xd821ccc0,0x9a4eb343,0xdba252fb,0x8b7d76c3});
    OriginalGameRandom_36f3c0 r;
    check(r.originalRandom31_36f4ac()==1848438282u,"Original signed 31bit random");
    r.originalSeed_36f3c0(5489);
    check(std::bit_cast<std::uint32_t>(r.originalRandomFloat_36f4d4())==0x3f5c59dcu,
          "Retail constant 0x7157A8 and cvt.s.w rounding");
    r.originalSeed_36f3c0(5489);
    check(r.originalRandomIntRange_36f508(10,25)==10+1848438282%15,
          "Original signed DIV MFHI + inclusive first endpoint");
    r.originalSeed_36f3c0(5489);
    const float x=r.originalRandomFloatRange_36f548(.8f,1.0f);
    check(x>=.8f&&x<=1.0f,"Original source random range");
    r.originalSeed_36f3c0(5489);
    bool threw=false;
    try{(void)r.originalRandomIntRange_36f508(5,5);}
    catch(const std::domain_error&){threw=true;}
    check(threw&&r.originalCursor()==623,
          "Zero-span signed MIPS DIV prevented before consuming source RNG");
    // 0x57C9BC and 0x57C9E0 must use one game-owned RNG state.
    const auto first=r.originalRandomFloat_36f4d4();
    const auto next=r.originalRandomFloatRange_36f548(.8f,1.0f);
    check(first>=0&&first<1&&next>=.8f&&next<=1,
          "Sequential original actor RNG samples");
    // Reject zero library-level random drift through full 624-step wrap.
    for(unsigned i=0;i<2000;++i)r.originalTemperedWord_36f444();
    check(r.originalCursor()==((623+2+2000)%624),"Source cursor cycles");
}
