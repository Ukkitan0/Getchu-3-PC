#include "ae3/original_movement_post_step_558030.hpp"
#include <bit>
#include <cmath>
#include <stdexcept>

namespace ae3 {
namespace {
float originalOrdinary(std::uint32_t bits) {
    const float value=std::bit_cast<float>(bits);
    if(!std::isfinite(value) || std::fpclassify(value)==FP_SUBNORMAL)
        throw std::invalid_argument(
            "original 0x558030 COP1 operand needs unavailable PS2 special semantics");
    return value;
}
}
std::uint32_t originalMovementPostStep_558030(
    const OriginalMovementChildD0_30c57c& child,
    const OriginalMovementPostStep_558030& source,
    OriginalMovementPostStepOwner_558030& owner) {
    const auto parentVA=child.originalWordAt(4);
    // Source 0x558030 LWC1 child+B4, 0x55803C C.OLE.S
    // threshold 1.0f at source 0x704B0C.
    const auto progress=originalOrdinary(child.originalWordAt(0xB4));
    if(progress>=1.0f) {
        // 0x558060 DIV.S f0=f21/f20; 0x558064 v0=2.
        // 0x558068 branches to the epilogue; its 0x55806C
        // delay slot SWC1 publishes f0 to original parent+0x70.
        const auto numerator=originalOrdinary(source.originalF21MovementReturnBits);
        const auto denominator=originalOrdinary(source.originalF20ResourceMetricBits);
        if(denominator==0.0f)
            throw std::domain_error("original 0x558060 divide-zero semantics unrecovered");
        const volatile float f0=numerator/denominator;
        if(!std::isfinite(f0) || std::fpclassify(f0)==FP_SUBNORMAL ||
           static_cast<double>(f0)*static_cast<double>(denominator)!=
               static_cast<double>(numerator))
            throw std::domain_error("original 0x558060 COP1 division rounding not verified");
        owner.store_original_parent_float70(
            parentVA,std::bit_cast<std::uint32_t>(float(f0)));
        return 2u;
    }
    // Original 0x558050 BEQZL -> 0x558070, delay slot 0x558054
    // LW a0,parent+0x1C executes only on nonterminal updates.
    if(!source.originalS5From30C0A0)
        throw std::invalid_argument("genuine original 0x30C0A0 s5 result required");
    const auto parentWord1C=owner.read_original_parent_word1c(parentVA);
    const auto mode=child.originalByteAt(9);
    if(!mode)
        throw std::logic_error("original child+0x09 mode unknown");
    // Original 0x558074 JAL 0x314C44 with child mode in
    // 0x558078 delay slot. 0x558080 sets v0=0 for return.
    // Retail 558054 LW sign extends the 32-bit parent word
    // into EE a0; the 558074 JAL passes that complete register.
    const auto originalA0=static_cast<std::uint64_t>(
        static_cast<std::int64_t>(
            static_cast<std::int32_t>(parentWord1C)));
    owner.call_original_314c44(
        originalA0,*source.originalS5From30C0A0,
        std::to_integer<std::uint32_t>(*mode));
    return 0u;
}
} // namespace ae3
