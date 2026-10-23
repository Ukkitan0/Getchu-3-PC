#include "ae3/original_movement_flagged_36fb5c.hpp"
#include "ae3/original_movement_child_30c57c.hpp"
#include <bit>
#include <cmath>
#include <stdexcept>

namespace ae3 {
namespace {
float valid(std::uint32_t bits) {
    const float v=std::bit_cast<float>(bits);
    if(!std::isfinite(v) || std::fpclassify(v)==FP_SUBNORMAL)
        throw std::invalid_argument(
            "original 36FB5C needs unavailable PS2 VU0/COP1 FP special semantics");
    return v;
}
void validate(float v) {
    if(!std::isfinite(v) || std::fpclassify(v)==FP_SUBNORMAL)
        throw std::domain_error("original 36FB5C PS2 FP overflow/denormal unresolved");
}
float mul(float a,float b) {
    const volatile float v=a*b;
    validate(v);
    if(static_cast<double>(v)!=static_cast<double>(a)*static_cast<double>(b))
        throw std::domain_error("original VU0/COP1 VMUL rounding not established");
    return v;
}
float add(float a,float b) {
    const volatile float v=a+b;
    validate(v);
    if(static_cast<double>(v)!=static_cast<double>(a)+static_cast<double>(b))
        throw std::domain_error("original VU0/COP1 ADD rounding not established");
    return v;
}
float sub(float a,float b) {return add(a,-b);}
float divide(float a,float b) {
    if(b==0.0f)throw std::domain_error("source 36FBB8 divide zero not recovered");
    const volatile float v=a/b;
    validate(v);
    if(static_cast<double>(v)*static_cast<double>(b)!=static_cast<double>(a))
        throw std::domain_error("original COP1 DIV.S rounding not established");
    return v;
}
std::uint32_t bits(float x) {return std::bit_cast<std::uint32_t>(x);}
}

OriginalMovementFlaggedPass_36fb5c originalMovementFlaggedPass_36fb5c(
    const OriginalMovementFlaggedInputs_36fb5c& s) {
    if(!s.originalChildScratchGuestVA ||
       (s.originalChildScratchGuestVA>>32)!=0 ||
       (s.originalChildScratchGuestVA&3u)!=0)
        throw std::invalid_argument("original 36FB6C BBit032 tagged child requires other path");
    if((s.originalChildFlagsA8&2u)==0u)
        throw std::invalid_argument("original 36FB5C requires real child +0xA8 flag bit1");
    const float status=valid(s.originalChildStatusA4Bits);
    const float movement=valid(s.originalMovementF12Bits);
    if(!(status<1.0f) || !(movement>0.0f))
        throw std::invalid_argument("original 36FB5C requires positive nonterminal source movement");
    std::array<float,4> sq{};
    for(std::size_t i=0;i<4;++i) {
        const float f=valid(s.originalLiveVF1[i]);
        // Original 36FB70 VMUL.xyzw VF1,VF1,VF1.
        sq[i]=mul(f,f);
    }
    const std::array<float,4> y{
        add(sq[0],sq[1]),add(sq[1],sq[1]),
        sq[2],add(sq[3],sq[1])};
    OriginalMovementFlaggedPass_36fb5c out{};
    for(std::size_t i=0;i<4;++i)
        // Original 36FB80 VADDy.xyw then 36FB84 VADDz.xyzw.
        out.originalVF1After[i]=bits(add(y[i],sq[2]));
    const float lengthSquared=valid(out.originalVF1After[0]);
    // Original 36FB9C SQRT.S f3, f0 and
    // 36FBA0 MUL.S f2, f3, 0x7157C4 = 0.125f.
    const volatile float root=std::sqrt(lengthSquared);
    validate(root);
    if(static_cast<double>(root)*static_cast<double>(root)!=
       static_cast<double>(lengthSquared))
        throw std::domain_error("original 36FB9C SQRT.S rounding not established");
    const float chunk=mul(root,0.125f);
    const bool small=movement<=chunk;  // 36FBA4 C.OLE.S
    float nextStatus,remaining;
    if(small) {
        // 36FBB8 DIV.S f1,f20,f3; 36FBBC ADD.S status,f1.
        // 36FBC0 resets original f20 to +0.
        nextStatus=add(status,divide(movement,root));
        remaining=0.0f;
    } else {
        // 36FC0C SUB.S f20=f20-f2,
        // 36FC14 delay-slot ADD.S status, 0.125f.
        remaining=sub(movement,chunk);
        nextStatus=add(status,0.125f);
    }
    out.originalFlagsA8After=0; // original SW zero at 36FBE4/36FBE8
    out.originalRemainingF20Bits=bits(remaining);
    if(nextStatus>=1.0f) {
        // Source 36FBD4 BC1TL takes its 36FBD8
        // SUB.S f0, status,1 delay slot. 36FBEC clamps
        // child +0xA4 to 1; 36FBF0 MUL.S overflow,root;
        // 36FBF4 ADD.S remaining,overflow returns f0.
        out.originalStatusA4AfterBits=0x3f800000u;
        const float extra=mul(sub(nextStatus,1.0f),root);
        out.originalReturnF0Bits=bits(add(remaining,extra));
    } else {
        out.originalStatusA4AfterBits=bits(nextStatus);
        if(small) {
            // Source 36FBE0 branches to 36FB4C with f20=0.
            // C.OLT.S 0,0 is false: returns source +0.
            out.originalReturnF0Bits=0u;
        } else {
            // Source 36FBE0 loops with remaining f20 > 0,
            // flag bit1 cleared. 36FC18 calls actual 36F8C8
            // to regenerate VF1; a final result is NOT known.
            out.requiresOriginal36F8C8=true;
        }
    }
    return out;
}
void originalMovementCommitFlaggedReturn_36fb5c(
    OriginalMovementChildD0_30c57c& child,
    const OriginalMovementFlaggedInputs_36fb5c& in,
    const OriginalMovementFlaggedPass_36fb5c& pass) {
    if(pass.requiresOriginal36F8C8 || !pass.originalReturnF0Bits)
        throw std::logic_error(
            "original 36FB5C cannot commit without the source 36F8C8 continuation");
    if(child.originalWordAt(0xB4)!=in.originalChildStatusA4Bits ||
       child.originalWordAt(0xB8)!=in.originalChildFlagsA8)
        throw std::invalid_argument(
            "original movement source plan no longer matches live child status/flags");
    // The owner must not supply a fabricated status or return f0:
    // recompute the retail source pass and require full agreement
    // before modifying the original child's guest-data image.
    const auto verified=originalMovementFlaggedPass_36fb5c(in);
    if(verified.originalVF1After!=pass.originalVF1After ||
       verified.originalStatusA4AfterBits!=pass.originalStatusA4AfterBits ||
       verified.originalFlagsA8After!=pass.originalFlagsA8After ||
       verified.originalReturnF0Bits!=pass.originalReturnF0Bits ||
       verified.originalRemainingF20Bits!=pass.originalRemainingF20Bits ||
       verified.requiresOriginal36F8C8!=pass.requiresOriginal36F8C8)
        throw std::invalid_argument(
            "original 36FB5C child commit rejected an unverified source pass");
    // The source stores status+0xA4 and clears flags+0xA8
    // before returning f0. The typed child starts at +0x10.
    child.writeWord(0xB4,pass.originalStatusA4AfterBits);
    child.writeWord(0xB8,pass.originalFlagsA8After);
}
} // namespace ae3
