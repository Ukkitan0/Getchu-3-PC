#include "ae3/original_movement_post_vector_557f54.hpp"
#include <bit>
#include <cmath>
#include <stdexcept>

namespace ae3 {
namespace {
float lane(std::uint32_t bits) {
    const float v=std::bit_cast<float>(bits);
    if(!std::isfinite(v)||std::fpclassify(v)==FP_SUBNORMAL)
        throw std::invalid_argument(
            "source 0x557F54 PS2 VU0 special float unsupported");
    return v;
}
float checked(float v) {
    if(!std::isfinite(v)||std::fpclassify(v)==FP_SUBNORMAL)
        throw std::domain_error("source 0x557F54 original VU0 result special");
    return v;
}
float multiply(float a,float b) {
    const volatile float x=a*b;
    checked(x);
    if(static_cast<double>(x)!=static_cast<double>(a)*static_cast<double>(b))
        throw std::domain_error("original 0x557F54 VMUL rounding unproven");
    return x;
}
float add(float a,float b) {
    // Binary64 can also lose an extreme nonzero binary32 lane.
    // Exponent gaps over 24 cannot yield exact VU0 additions.
    if(a!=0.0f && b!=0.0f &&
       std::abs(std::ilogb(a)-std::ilogb(b))>24)
        throw std::domain_error("original 0x557F54 VADD rounding unproven");
    const volatile float x=a+b;
    checked(x);
    if(static_cast<double>(x)!=static_cast<double>(a)+static_cast<double>(b))
        throw std::domain_error("original 0x557F54 VADD rounding unproven");
    return x;
}
std::uint32_t bits(float f) {return std::bit_cast<std::uint32_t>(f);}
}
OriginalMovementPostVector_557f54 originalMovementPostVector_557f54(
    const OriginalMovementChildD0_30c57c& child,
    std::uint64_t originalParentWord14GuestVA) {
    // 0x557F54 child+B8 bit1 zero branches to actual 0x36F8C8.
    if((child.originalWordAt(0xB8)&2u)==0u)
        throw std::logic_error(
            "original 0x557F5C must regenerate via 36F8C8 first");
    // 0x557F64 LQ child+A0; 557F68 DMTC2 -> VF1.
    std::array<float,4> source{},squared{};
    for(std::size_t i=0;i<4;++i) {
        source[i]=lane(child.originalWordAt(0xA0+i*4));
        squared[i]=multiply(source[i],source[i]);
    }
    // 557F6C VF2=VF1^2; VADDy.xyw, VADDz.xyzw.
    const std::array<float,4> xy{
        add(squared[0],squared[1]),add(squared[1],squared[1]),
        squared[2],add(squared[3],squared[1])};
    const std::array<float,4> lengthSq{
        add(xy[0],squared[2]),add(xy[1],squared[2]),
        add(xy[2],squared[2]),add(xy[3],squared[2])};
    // 557F78 VRSQRT Q,VF0.w,VF2.x; 557F7C VWAITQ.
    const volatile float length=std::sqrt(lengthSq[0]);
    if(!(length>0.0f)||!std::isfinite(length)||
       static_cast<double>(length)*static_cast<double>(length)!=
           static_cast<double>(lengthSq[0]))
        throw std::domain_error("source 0x557F78 VRSQRT SQRT rounding/zero unsupported");
    const volatile float q=1.0f/length;
    checked(q);
    if(static_cast<double>(q)*static_cast<double>(length)!=1.0)
        throw std::domain_error("source 0x557F78 VRSQRT reciprocal rounding unproven");
    // 0x557F80 VMULq VF2,VF1,Q.
    OriginalMovementPostVector_557f54 result{};
    result.originalVU0Q=bits(q);
    for(std::size_t i=0;i<4;++i) {
        result.originalVF2After[i]=bits(multiply(source[i],q));
        result.originalVF1After[i]=bits(source[i]);
    }
    // Source 557F88 BEQZ child+8, unconditional delay slot
    // 557F8C DMFC2 s3,VF2.x.
    result.originalGprS3VF2X=result.originalVF2After[0];
    const auto mode8=child.originalByteAt(8);
    if(!mode8)
        throw std::logic_error("original 0x30C57C child mode+8 unknown");
    if(*mode8==std::byte{0})return result;
    // Source 557F90 BBit032 parent+14 chooses alternate path.
    if((originalParentWord14GuestVA>>32)!=0 ||
       (originalParentWord14GuestVA&3u)!=0)
        throw std::invalid_argument("original 557F90 tagged parent branch unrecovered");
    // 0x557F98 VF1=VF1*VF2; VADDy, VADDz.
    std::array<float,4> products{};
    for(std::size_t i=0;i<4;++i)
        products[i]=multiply(source[i],
                            std::bit_cast<float>(result.originalVF2After[i]));
    const std::array<float,4> intermediate{
        add(products[0],products[1]),add(products[1],products[1]),
        products[2],add(products[3],products[1])};
    for(std::size_t i=0;i<4;++i)
        result.originalVF1After[i]=bits(add(intermediate[i],products[2]));
    // 557FBC C.OLT.S VF1.x, 0x704B08=0x3F733333
    // (0.95f); 557FC0 BC1TL enters original slot5
    // epilogue with source t6=1.
    if(std::bit_cast<float>(result.originalVF1After[0])<
       std::bit_cast<float>(0x3F733333u))
        result.originalSlot5EarlyStatus=1u;
    return result;
}
} // namespace ae3
