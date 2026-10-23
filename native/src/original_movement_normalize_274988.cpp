#include "ae3/original_movement_normalize_274988.hpp"
#include <bit>
#include <cmath>
#include <stdexcept>

namespace ae3 {
namespace {
float originalOrdinary(std::uint32_t raw) {
    const float value=std::bit_cast<float>(raw);
    // Signed zero, denormals and exceptional operands have
    // hardware-specific VU0/COP1 flags not translated here.
    if(!std::isfinite(value) || std::fpclassify(value)==FP_SUBNORMAL ||
       (value==0.0f && std::signbit(value)))
        throw std::invalid_argument(
            "original 274988 VU0 special operand mode unresolved");
    return value;
}
void checked(float value) {
    if(!std::isfinite(value) || std::fpclassify(value)==FP_SUBNORMAL ||
       (value==0.0f && std::signbit(value)))
        throw std::domain_error(
            "original 274988 VU0/COP1 result needs special floating semantics");
}
float mulExact(float a,float b) {
    const volatile float result=a*b;
    checked(result);
    if(static_cast<double>(result)!=static_cast<double>(a)*static_cast<double>(b))
        throw std::domain_error(
            "original 274988 VMUL/VMULx requires inexact PS2 rounding");
    return result;
}
float addExact(float a,float b) {
    // The binary64 arithmetic comparison below alone can
    // round away a nonzero operand separated by >53 bits.
    if(a!=0.0f && b!=0.0f &&
       std::abs(std::ilogb(a)-std::ilogb(b))>24)
        throw std::domain_error(
            "original 274988 VADD requires inexact PS2 rounding");
    const volatile float result=a+b;
    checked(result);
    if(static_cast<double>(result)!=static_cast<double>(a)+static_cast<double>(b))
        throw std::domain_error(
            "original 274988 VADD requires inexact PS2 rounding");
    return result;
}
std::uint32_t raw(float x) {return std::bit_cast<std::uint32_t>(x);}
}
OriginalMovementNormalizeResult_274988 originalMovementNormalizeExact_274988(
    const OriginalMovementNormalizeInputs_274988& source) {
    // Source 274988 LQ t7,0(a1), zero t6, then:
    // 274990 PEXTLW t6,t6,t7 -> [a1.x,0,a1.y,0]
    // 274994 PCPYUD t7,t7,t7 -> [a1.z,a1.w,a1.z,a1.w]
    // 274998 PCPYLD t6,t7,t6 -> [a1.x,0,a1.z,a1.w].
    // 27499C DMTC2 t6,VF2; source explicitly DROPS a1.y.
    const std::array<float,4> pack{
        originalOrdinary(source.originalEEStackA1[0]),0.0f,
        originalOrdinary(source.originalEEStackA1[2]),
        originalOrdinary(source.originalEEStackA1[3])};
    // Ensure the unused source word has actual original data
    // provenance as an EE register lane, even though the source
    // MMI sequence drops it without doing floating operations.
    const auto ignoredSourceY=source.originalEEStackA1[1];
    (void)ignoredSourceY;

    OriginalMovementNormalizeResult_274988 result{};
    for(std::size_t i=0;i<4;++i)
        result.originalVF2Packed[i]=raw(pack[i]);
    std::array<float,4> prior{};
    for(std::size_t i=0;i<4;++i)
        prior[i]=originalOrdinary(source.originalLiveVF1Before[i]);

    std::array<float,4> squared{};
    for(std::size_t i=0;i<4;++i)
        // Source 2749A0 VMUL.xyzw VF2,VF2,VF2.
        squared[i]=mulExact(pack[i],pack[i]);

    // Source 2749A4 VADDy.xyw VF1,VF2,VF2:
    // destination Z is NOT written (preserves live original
    // VF1.z), all other destination lanes use squared VF2.y.
    std::array<float,4> vaddY{
        addExact(squared[0],squared[1]),
        addExact(squared[1],squared[1]),
        prior[2],
        addExact(squared[3],squared[1])};
    // Source 2749A8 VADDz.xyzw VF1,VF1,VF2,
    // including the original pre-existing VF1 Z component.
    for(std::size_t i=0;i<4;++i)
        result.originalVF1BeforeThreshold[i]=raw(
            addExact(vaddY[i],squared[2]));

    // Original 2749AC DMFC2 VF1.x; 2749B4 MTC1 f0.
    const float lengthSquared=originalOrdinary(
        result.originalVF1BeforeThreshold[0]);
    result.originalLengthSquaredBits=raw(lengthSquared);
    // Source 2749C8 SQRT.S f1,f0 is executed before the
    // source epsilon comparison, even if normalization skips.
    const volatile float root=std::sqrt(lengthSquared);
    checked(root);
    if(static_cast<double>(root)*static_cast<double>(root)!=
       static_cast<double>(lengthSquared))
        throw std::domain_error(
            "original 2749C8 SQRT.S requires PS2 hardware rounding");
    result.originalLengthBits=raw(root);

    // Original 2749CC LWC1 0x6ED7C0 = 0x2EDBE6FF,
    // original 2749D0 C.OLT.S epsilon, root.
    constexpr std::uint32_t originalEpsilonBits=0x2EDBE6FFu;
    const auto epsilon=std::bit_cast<float>(originalEpsilonBits);
    if(!(epsilon<root)) {
        result.originalPerformedNormalize=false;
        result.originalVF1After=result.originalVF1BeforeThreshold;
        result.originalReciprocalBits=0;
        return result;
    }

    // Original 2749E0 LWC1 0x6ED7C4 = 1.0;
    // 2749EC DIV.S f2,f0,f1, 2749F0 MFC1 f2,
    // 2749F4 DMTC2 into VF1.x, 2749F8
    // VMULx.xyzw VF1,VF2,VF1.x.
    const volatile float reciprocal=1.0f/root;
    checked(reciprocal);
    if(static_cast<double>(reciprocal)*static_cast<double>(root)!=1.0)
        throw std::domain_error(
            "original 2749EC DIV.S requires PS2 hardware rounding");
    result.originalPerformedNormalize=true;
    result.originalReciprocalBits=raw(reciprocal);
    for(std::size_t i=0;i<4;++i)
        result.originalVF1After[i]=raw(mulExact(pack[i],reciprocal));
    return result;
}
} // namespace ae3
