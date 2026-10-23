#include "ae3/original_movement_regenerate_36f8c8.hpp"
#include <bit>
#include <cmath>
#include <stdexcept>

namespace ae3 {
namespace {
float checked(std::uint32_t raw) {
    const float value=std::bit_cast<float>(raw);
    if(!std::isfinite(value) || std::fpclassify(value)==FP_SUBNORMAL)
        throw std::invalid_argument(
            "original 0x36F8C8 PS2 VU0/COP1 special FP semantics unresolved");
    return value;
}
float checkedResult(float value) {
    if(!std::isfinite(value) || std::fpclassify(value)==FP_SUBNORMAL)
        throw std::domain_error("original 0x36F8C8 VU0 overflow/denormal unresolved");
    return value;
}
float mul(float x,float y) {
    const volatile float value=x*y;
    checkedResult(value);
    if(static_cast<double>(value)!=static_cast<double>(x)*static_cast<double>(y))
        throw std::domain_error("original 0x36F8C8 VU0 MUL rounding unresolved");
    return value;
}
float add(float x,float y) {
    // A binary64 comparison is not a proof of exact binary32
    // addition when the smaller nonzero operand vanishes in double.
    if(x!=0.0f && y!=0.0f &&
       std::abs(std::ilogb(x)-std::ilogb(y))>24)
        throw std::domain_error(
            "original 0x36F8C8 VU0 ADD rounding unresolved");
    const volatile float value=x+y;
    checkedResult(value);
    if(static_cast<double>(value)!=static_cast<double>(x)+static_cast<double>(y))
        throw std::domain_error("original 0x36F8C8 VU0 ADD rounding unresolved");
    return value;
}
float sub(float x,float y) {
    // VSUBAw has the same exactness condition as VADD: subtraction
    // of a nonzero far-smaller operand must not be silently lost.
    if(x!=0.0f && y!=0.0f &&
       std::abs(std::ilogb(x)-std::ilogb(y))>24)
        throw std::domain_error(
            "original 0x36F8C8 VU0 SUB rounding unresolved");
    const volatile float value=x-y;
    checkedResult(value);
    if(static_cast<double>(value)!=static_cast<double>(x)-static_cast<double>(y))
        throw std::domain_error("original 0x36F8C8 VU0 SUB rounding unresolved");
    return value;
}
std::uint32_t bits(float f) {return std::bit_cast<std::uint32_t>(f);}
}
OriginalMovementRegenerateResult_36f8c8 originalMovementRegenerate_36f8c8(
    const OriginalMovementRegenerateSource_36f8c8& src) {
    // Genuine 0x36F8D0/D8/EC/928 BBit032 and 0x36F948
    // BBit132 alternate branches are untaken for zero high-32
    // halves of both original EE guest addresses.
    if(!src.originalChildScratchGuestVA ||
       (src.originalChildScratchGuestVA>>32)!=0 ||
       (src.originalChildScratchGuestVA&0xfu)!=0 ||
       !src.originalResourcePointerRegA0 ||
       (src.originalResourcePointerRegA0>>32)!=0 ||
       // Original 36F8EC LW t7,0xA0(a0) sign-extends into
       // the 64-bit EE GPR. A high-bit low word cannot be a
       // zero-extended ordinary source for the original LQ path.
       (src.originalResourcePointerRegA0&0x80000000ull)!=0 ||
       (src.originalResourcePointerRegA0&0xfu)!=0)
        throw std::invalid_argument(
            "original 0x36F8C8 tagged/unaligned source requires another branch");

    // 0x36F8C8 LWC1 f0=status; 36F8D4 MUL.S f1=f0^2;
    // 36F8E8 MUL.S f0=f1*f0. 36F8E0/908/90C PPACW
    // together pack VF4/VF1=[status^3,status^2,status,1].
    const float progress=checked(src.originalStatusA4Bits);
    const float square=mul(progress,progress);
    const float cube=mul(square,progress);
    const std::array<float,4> poly{cube,square,progress,1.0f};
    OriginalMovementRegenerateResult_36f8c8 out{};
    for(std::size_t i=0;i<4;++i) {
        const float oldVF1=checked(src.originalVF1ThroughVF8[0][i]);
        const float vf2=checked(src.originalVF1ThroughVF8[1][i]);
        const float vf3=checked(src.originalVF1ThroughVF8[2][i]);
        const float vf5=checked(src.originalVF1ThroughVF8[4][i]);
        const float vf6=checked(src.originalVF1ThroughVF8[5][i]);
        const float vf7=checked(src.originalVF1ThroughVF8[6][i]);
        const float vf8=checked(src.originalVF1ThroughVF8[7][i]);

        // Source COP2 SPECIAL2 0x36F918 VMULAx (old VF1),
        // 0x36F92C VMADDAy, 0x36F930 VMADDAz,
        // 0x36F934 VSUBAw. 36F924 overwrites VF1 only AFTER
        // the first multiply. Preserve genuine pipeline order.
        float acc=mul(oldVF1,poly[0]);
        acc=add(acc,mul(vf2,poly[1]));
        acc=add(acc,mul(vf5,poly[2]));
        acc=sub(acc,mul(vf3,poly[3]));
        out.originalACCFirstStage[i]=bits(acc);

        // 0x36F938 VMULAx REPLACES ACC; 0x36F93C VMADDAy,
        // 0x36F940 VMADDAz, 0x36F944 VSUBAw use VF8/7/6/4.
        acc=mul(vf8,poly[0]);
        acc=add(acc,mul(vf7,poly[1]));
        acc=add(acc,mul(vf6,poly[2]));
        acc=sub(acc,mul(poly[i],poly[3]));
        out.originalACCAfter[i]=bits(acc);
        out.originalVF1After[i]=bits(poly[i]);
        out.originalVF4After[i]=bits(poly[i]);
    }
    // 36F91C ORI $t7,2 and 36F920 SW flags.
    out.originalFlagsA8After=src.originalFlagsA8|2u;
    // 36F950 DMFC2 $v0,VF1 in 36F94C JR delay slot.
    out.originalReturnV0=out.originalVF1After;
    return out;
}
} // namespace ae3
