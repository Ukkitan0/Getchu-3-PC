#include "ae3/original_manager_resource_vu0_317b34.hpp"
#include <bit>
#include <cmath>
#include <stdexcept>
#include <limits>

namespace ae3 {
namespace {
float ordinary(std::uint32_t bits) {
    float f=std::bit_cast<float>(bits);
    if (!std::isfinite(f) || std::fpclassify(f)==FP_SUBNORMAL)
        throw std::invalid_argument("original 0x317B34 requires unimplemented PS2 FP special mode");
    return f;
}
float exactMultiply(float a,float b) {
    const volatile float v=a*b;
    if (!std::isfinite(v) || std::fpclassify(v)==FP_SUBNORMAL ||
        static_cast<double>(v)!=static_cast<double>(a)*static_cast<double>(b))
        throw std::domain_error("original VU0 VMUL requires hardware rounding");
    return v;
}
float exactAdd(float a,float b) {
    // Even binary64 can discard a tiny nonzero binary32 operand.
    // When source exponents differ by over 24, a single binary32
    // ADD cannot be exact; do not certify PS2 VU0/COP1 rounding.
    if(a!=0.0f && b!=0.0f &&
       std::abs(std::ilogb(a)-std::ilogb(b))>24)
        throw std::domain_error(
            "original VU0/COP1 ADD requires hardware rounding");
    const volatile float v=a+b;
    if (!std::isfinite(v) || std::fpclassify(v)==FP_SUBNORMAL ||
        static_cast<double>(v)!=static_cast<double>(a)+static_cast<double>(b))
        throw std::domain_error("original VU0/COP1 ADD requires hardware rounding");
    return v;
}
}
OriginalResourceVU0Result_317b34 originalManagerResourceVU0_317b34(
    const OriginalManagerResourceFloatFields_317b34& p,
    std::uint32_t mode,
    const OriginalResourceVU0Inputs_317b34& source) {
    if(p.originalParentWord40==1u || (mode&255u)==0u)
        throw std::invalid_argument("original 0x317B34 VU0 branch not selected");
    // Original parent came from a 32-bit LW into a 64-bit EE
    // register. Negative low-word parent VAs sign-extend, taking
    // the original 317B58 tagged path (not this ordinary branch).
    // Refuse a caller that incorrectly zero-extends such a word.
    if(!source.originalParentGuestVA ||
       (source.originalParentGuestVA>>32)!=0u ||
       (source.originalParentGuestVA&0x80000000ull)!=0u ||
       (source.originalParentGuestVA&15u)!=0u)
        throw std::invalid_argument("original 0x317B58 tagged/unaligned parent requires other source path");
    if(!p.originalParentWord08 || (p.originalParentWord08&3u)!=0u)
        throw std::invalid_argument("original 0x317B34 parent+8 genuine resource table missing");

    // Original 0x317B5C LWC1 f2, resource+0x364 and
    // 0x317B64 LWC1 f1, resource+0x368.
    const float offset=ordinary(source.originalTable364Raw);
    const float cap=ordinary(source.originalTable368Raw);
    std::array<float,4> sq{};
    for(std::size_t i=0;i<4;++i)
        sq[i]=exactMultiply(ordinary(source.originalVF1Raw[i]),
                            ordinary(source.originalVF1Raw[i]));

    // Original 0x317B60 VMUL.xyzw VF1,VF1,VF1.
    // 0x317B68 VADDy.xyw: VU0 destination mask X,Y,W;
    // Z remains the original squared source component.
    const std::array<float,4> yadd{
        exactAdd(sq[0],sq[1]),
        exactAdd(sq[1],sq[1]),
        sq[2],
        exactAdd(sq[3],sq[1])};
    // 0x317B6C VADDz.xyzw, uses the unchanged squared Z.
    OriginalResourceVU0Result_317b34 result{};
    for(std::size_t i=0;i<4;++i)
        result.originalVF1After[i]=std::bit_cast<std::uint32_t>(
            exactAdd(yadd[i],sq[2]));
    // 0x317B70 DMFC2 VF1.x, 0x317B78 MTC1 f0.
    // 0x317B84 SQRT.S, 0x317B88 ADD.S f2, and
    // 0x317B90 MIN.S f0,f0,f1 in JR delay slot.
    const float length2=std::bit_cast<float>(result.originalVF1After[0]);
    const volatile float root=std::sqrt(length2);
    if(!std::isfinite(root) || std::fpclassify(root)==FP_SUBNORMAL ||
       static_cast<double>(root)*static_cast<double>(root)!=static_cast<double>(length2))
        throw std::domain_error("original R5900 SQRT.S requires hardware rounding");
    const float adjusted=exactAdd(root,offset);
    // MIN.S on ordinary unequal numbers. Exact equality with
    // differing sign-of-zero is not determined by this subset.
    if(adjusted==cap && std::signbit(adjusted)!=std::signbit(cap))
        throw std::domain_error("source MIN.S signed-zero tie unresolved");
    result.originalF0Raw=std::bit_cast<std::uint32_t>(
        adjusted<cap?adjusted:cap);
    return result;
}
} // namespace ae3
