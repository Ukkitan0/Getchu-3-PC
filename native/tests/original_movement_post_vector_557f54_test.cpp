#include "ae3/original_movement_post_vector_557f54.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <array>
#include <bit>
#include <bitset>
#include <iostream>
#include <stdexcept>
using namespace ae3;
namespace {
std::uint32_t raw(float x){return std::bit_cast<std::uint32_t>(x);}
std::unique_ptr<OriginalMovementChildD0_30c57c> child(
    float value,unsigned mode8=1,bool known=true,unsigned flags=2,
    float secondLane=0.0f) {
    std::array<std::byte,0x110> scratch{};
    std::bitset<0x110> mask{};
    const auto bits=raw(value);
    // Original 557F64 LQ child+A0 reads ALL FOUR original lanes
    // from scratch+0x90; zero-valued lanes are still known bytes.
    for(unsigned i=0;i<16;++i)if(known)mask.set(0x90+i);
    for(unsigned i=0;i<4;++i) {
        scratch[0x90+i]=std::byte((bits>>(i*8))&255u);
        const auto yBits=raw(secondLane);
        scratch[0x94+i]=std::byte((yBits>>(i*8))&255u);
        scratch[0xA8+i]=std::byte((flags>>(i*8))&255u);
        mask.set(0xA8+i);
    }
    return originalMovementChildCreate_30c57c(
        0x234000u,scratch,mask,mode8,2);
}
}
int main() {
    // 557F64 LQ source vector (.5,0,0,0) =>
    // 557F78 genuine Q=2, VF2=(1,0,0,0);
    // 557F98 source dot=.5 < retail 0.95 => early status1.
    auto c=child(0.5f);
    auto r=originalMovementPostVector_557f54(*c,0x340000u);
    assert(r.originalVU0Q==raw(2.0f));
    assert(r.originalVF2After[0]==raw(1.0f));
    assert(r.originalVF1After[0]==raw(0.5f));
    assert(r.originalGprS3VF2X==raw(1.0f));
    assert(r.originalSlot5EarlyStatus==1u);
    r=originalMovementPostVector_557f54(*child(1.0f),0x340000u);
    assert(r.originalVU0Q==raw(1.0f));
    assert(r.originalVF1After[0]==raw(1.0f));
    assert(!r.originalSlot5EarlyStatus);
    // Source child+8==0 BEQZ skips the second source VF1
    // dot/threshold but still updates VF2 and GPR s3.
    r=originalMovementPostVector_557f54(*child(0.5f,0),0x340000u);
    assert(!r.originalSlot5EarlyStatus &&
           r.originalVF1After[0]==raw(0.5f));
    bool rejected=false;
    try{(void)originalMovementPostVector_557f54(*child(0.0f),0x340000u);}
    catch(const std::domain_error&){rejected=true;}
    assert(rejected);
    rejected=false;
    try{(void)originalMovementPostVector_557f54(*child(0.5f,1,false),0x340000u);}
    catch(const std::logic_error&){rejected=true;}
    assert(rejected);
    rejected=false;
    try{(void)originalMovementPostVector_557f54(*child(0.5f,1,true,0),0x340000u);}
    catch(const std::logic_error&){rejected=true;}
    assert(rejected);
    rejected=false;
    try{(void)originalMovementPostVector_557f54(*c,0x100340000ull);}
    catch(const std::invalid_argument&){rejected=true;}
    assert(rejected);
    // Source 557F70 VADDy: binary32 squares [1,2^-70].
    // Double comparison alone falsely certifies the addition.
    rejected=false;
    try{(void)originalMovementPostVector_557f54(
        *child(1.0f,1,true,2,0x1p-35f),0x340000u);}
    catch(const std::domain_error&){rejected=true;}
    assert(rejected);
    std::cout<<"ORIGINAL_557F54_POST_VECTOR_EARLY_STATUS_PASSED\n";
}
