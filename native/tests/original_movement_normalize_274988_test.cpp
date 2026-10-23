#include "ae3/original_movement_normalize_274988.hpp"
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
std::uint32_t bits(float x) {return std::bit_cast<std::uint32_t>(x);}
OriginalMovementNormalizeInputs_274988 original(
    std::array<std::uint32_t,4> source,
    std::array<std::uint32_t,4> prior={
        bits(1.0f),bits(2.0f),bits(3.0f),bits(4.0f)}) {
    return {source,prior};
}
}
int main() {
    // Real 274990 PEXTLW + 274994 PCPYUD + 274998 PCPYLD:
    // discard input Y, force +zero VF2.y, preserve X/Z/W.
    // Original 2749A4 VADDy.xyw does NOT update VF1.z;
    // its old physical lane is then consumed by 2749A8 VADDz.
    auto source=original({
        bits(2.0f),bits(37.0f),bits(0.0f),bits(1.0f)});
    auto r=originalMovementNormalizeExact_274988(source);
    assert(r.originalVF2Packed==
           (std::array<std::uint32_t,4>{
               bits(2.0f),0u,0u,bits(1.0f)}));
    assert(r.originalVF1BeforeThreshold==
           (std::array<std::uint32_t,4>{
               bits(4.0f),0u,bits(3.0f),bits(1.0f)}));
    assert(r.originalLengthSquaredBits==bits(4.0f));
    assert(r.originalLengthBits==bits(2.0f));
    assert(r.originalPerformedNormalize &&
           r.originalReciprocalBits==bits(0.5f));
    assert(r.originalVF1After==
           (std::array<std::uint32_t,4>{
               bits(1.0f),0u,0u,bits(0.5f)}));

    // The original source never performs FP arithmetic on the
    // MMI-dropped input Y lane, even if it is a NaN bit pattern.
    source.originalEEStackA1[1]=0x7fc01234u;
    r=originalMovementNormalizeExact_274988(source);
    assert(r.originalPerformedNormalize &&
           r.originalVF1After[0]==bits(1.0f));

    // Original epsilon table at 6ED7C0: 0x2EDBE6FF (~1e-10).
    // If length <= epsilon, skip division/VMULx. The source
    // VADDz VF1.z remains prior VF1.z even when X/Z are zero.
    source=original({0u,0u,0u,0u});
    r=originalMovementNormalizeExact_274988(source);
    assert(!r.originalPerformedNormalize &&
           r.originalLengthBits==0u &&
           r.originalVF1After==
           (std::array<std::uint32_t,4>{
               0u,0u,bits(3.0f),0u}));
    source=original({bits(0x1p-35f),0u,0u,bits(1.0f)},
                    {0u,0u,0u,0u});
    r=originalMovementNormalizeExact_274988(source);
    assert(!r.originalPerformedNormalize &&
           r.originalLengthBits==bits(0x1p-35f));

    // Genuine XZ=(0,2) is normalized to +Z; the original
    // ignored Y must not contribute to the length.
    source=original({0u,bits(42.0f),bits(2.0f),bits(1.0f)},
                    {bits(1.0f),0u,bits(0.5f),0u});
    r=originalMovementNormalizeExact_274988(source);
    assert(r.originalPerformedNormalize &&
           r.originalVF1BeforeThreshold[2]==bits(4.5f) &&
           r.originalVF1After==
           (std::array<std::uint32_t,4>{
               0u,0u,bits(1.0f),bits(0.5f)}));

    // Original 2749C8 SQRT.S sqrt(2) requires genuine
    // PS2 rounding, never a host approximation.
    bool refused=false;
    source=original({bits(1.0f),0u,bits(1.0f),bits(1.0f)},
                    {0u,0u,0u,0u});
    try{(void)originalMovementNormalizeExact_274988(source);}
    catch(const std::domain_error&){refused=true;}
    assert(refused);

    // sqrt(3^2+4^2)=5 exact, but 2749EC DIV.S 1/5 is
    // inexact and cannot pass the native exact-only path.
    refused=false;
    source=original({bits(3.0f),0u,bits(4.0f),bits(1.0f)},
                    {0u,0u,0u,0u});
    try{(void)originalMovementNormalizeExact_274988(source);}
    catch(const std::domain_error&){refused=true;}
    assert(refused);

    // When binary64 also discards a much smaller VU0 operand,
    // the source VADD cannot be falsely declared exact.
    refused=false;
    source=original({bits(1.0f),0u,bits(0x1p-35f),bits(1.0f)},
                    {0u,0u,0u,0u});
    try{(void)originalMovementNormalizeExact_274988(source);}
    catch(const std::domain_error&){refused=true;}
    assert(refused);

    // Unresolved VU0 NaN and signed-zero source semantics
    // must never silently become fake original state.
    refused=false;
    source=original({0x7fc00001u,0u,0u,bits(1.0f)});
    try{(void)originalMovementNormalizeExact_274988(source);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused);
    refused=false;
    source=original({0x80000000u,0u,0u,bits(1.0f)});
    try{(void)originalMovementNormalizeExact_274988(source);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused);
    std::cout<<"ORIGINAL_274988_NATIVE_XZ_NORMALIZATION_EXACT_PASSED\n";
}
