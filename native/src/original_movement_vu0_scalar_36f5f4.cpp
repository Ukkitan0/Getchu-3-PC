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
