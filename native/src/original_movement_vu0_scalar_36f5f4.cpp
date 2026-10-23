#include "ae3/original_movement_vu0_scalar_36f5f4.hpp"
#include "ae3/original_movement_scratch_tail_36f6a8.hpp"
#include <bit>
#include <cmath>
#include <stdexcept>
#include <limits>

namespace ae3 {
namespace {
float lane(const std::array<std::byte,16>& originalEE,
           std::size_t index) noexcept {
    std::uint32_t bits=0;
    for (std::size_t i=0;i<4;++i)
        bits|=std::uint32_t(std::to_integer<unsigned>(
            originalEE[index*4+i]))<<(8*i);
    return std::bit_cast<float>(bits);
}
void requireOrdinaryVU0(float value) {
    // Genuine EE VU0 is not fully IEEE-754-compliant. Until the
    // source's overflow/denormal modes are recovered, refuse them.
    if (!std::isfinite(value) ||
        (std::fpclassify(value)==FP_SUBNORMAL))
        throw std::invalid_argument(
            "source VU0 packed operand requires unrecovered PS2 FP semantics");
}
float originalSquaredLengthXYZ(
    const std::array<std::byte,16>& sourceA,
    const std::array<std::byte,16>& sourceB) {
    float squared[3]{};
    for (std::size_t component=0;component<3;++component) {
        const float x=lane(sourceA,component);
        const float y=lane(sourceB,component);
        requireOrdinaryVU0(x);
        requireOrdinaryVU0(y);
        // Source 0x36F608/0x36F610 VSUB.xyzw followed by
        // source 0x36F618/0x36F61C VMUL.xyzw. Do not FMA.
        const volatile float difference=y-x;
        requireOrdinaryVU0(difference);
        const volatile float square=difference*difference;
        requireOrdinaryVU0(square);
        squared[component]=square;
    }
    // 0x36F620/0x36F624 VADDy.xyw creates VF*.x=x²+y².
    // 0x36F628/0x36F62C VADDz.xyzw then adds the original
    // squared Z lane. This source order matters for rounding.
    const volatile float xy=squared[0]+squared[1];
    requireOrdinaryVU0(xy);
    const volatile float xyz=xy+squared[2];
    requireOrdinaryVU0(xyz);
    return xyz;
}
}
OriginalMovementVU0ScalarResult_36f5f4 originalMovementVU0Scalars_36f5f4(
    const OriginalMovementVU0ScalarSource_36f5f4& original) {
    // VF3=A1, VF4=A2, VF6=A3. Two independent VU0 vector
    // lengths. The original 0x36F64C normalization is a
    // separate side effect, not a replacement for this scalar.
    const float first=originalSquaredLengthXYZ(
        original.originalA1,original.originalA2);
    const float second=originalSquaredLengthXYZ(
        original.originalA1,original.originalA3);
    return {std::bit_cast<std::uint32_t>(first),
            std::bit_cast<std::uint32_t>(second)};
}
OriginalMovementVU0VectorFrame_36f5f4 originalMovementVU0VectorFrame_36f5f4(
    const OriginalMovementVU0ScalarSource_36f5f4& source) {
    const auto scalars=originalMovementVU0Scalars_36f5f4(source);
    const bool small=std::bit_cast<float>(scalars.originalFirstLengthSquaredBits)<1.0f;
    float a[4]{},b[4]{},c[4]{};
    for(std::size_t i=0;i<4;++i) {
        a[i]=lane(source.originalA1,i);
        b[i]=lane(source.originalA2,i);
        c[i]=lane(source.originalA3,i);
        requireOrdinaryVU0(a[i]);
        requireOrdinaryVU0(b[i]);
        requireOrdinaryVU0(c[i]);
    }
    OriginalMovementVU0VectorFrame_36f5f4 result{};
    result.scalar=scalars;
    result.originalFirstLengthUnderOne=small;
    for(std::size_t i=0;i<4;++i) {
        // 0x36F650..0x36F65C source branch:
        // QMTC2 VF1.x=0.5, VMULx VF2=VF3*0.5,
        // VMULx VF1=VF6*0.5, VADD VF4=VF2+VF1.
        // Otherwise VF4 retains original EE A2.
        float sourceMiddle=b[i];
        if(small) {
            const volatile float left=a[i]*0.5f;
            const volatile float right=c[i]*0.5f;
            sourceMiddle=left+right;
            requireOrdinaryVU0(sourceMiddle);
        }
        // 0x36F668/0x36F66C source PPACW zero t5 twice,
        // followed by 0x36F674 QMTC2 VF7 = all zero.
        // 0x36F67C: VF3=VF4-VF3 (original source A1)
        // 0x36F684: VF1=VF5-VF4 (VF5 is original A3)
        // 0x36F68C: VF2=VF7-VF5 = 0-original A3.
        const volatile float vf3=sourceMiddle-a[i];
        const volatile float vf1=c[i]-sourceMiddle;
        const volatile float vf2=0.0f-c[i];
        requireOrdinaryVU0(vf3);
        requireOrdinaryVU0(vf1);
        requireOrdinaryVU0(vf2);
        result.originalVF4[i]=std::bit_cast<std::uint32_t>(sourceMiddle);
        result.originalVF3[i]=std::bit_cast<std::uint32_t>(float(vf3));
        result.originalVF1[i]=std::bit_cast<std::uint32_t>(float(vf1));
        result.originalVF2[i]=std::bit_cast<std::uint32_t>(float(vf2));
    }
    return result;
}
void originalMovementScratchFromVU0Scalar_36f5f4(
    std::span<std::byte,0x110> sourceScratch,
    std::uint64_t originalGuestStackA0,
    const OriginalMovementVU0ScalarSource_36f5f4& genuineSource) {
    const auto result=originalMovementVU0Scalars_36f5f4(genuineSource);
    // Retail 0x57C13C fixes t0=0. Only the second squared
    // length survives COP1 f1 at 0x36F664 and C.OLT.S at 0x36F698.
    originalMovementScratchScalarTail_36f6a8(sourceScratch,
        {originalGuestStackA0,0,result.originalSecondLengthSquaredBits});
}
} // namespace ae3
