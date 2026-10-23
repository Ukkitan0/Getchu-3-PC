#include "ae3/original_movement_step_fast_36fb18.hpp"
#include <bit>
#include <cmath>
#include <stdexcept>

namespace ae3 {
std::optional<std::uint32_t> originalMovementStepFast_36fb18(
    const OriginalMovementChildD0_30c57c& child,
    std::uint32_t inputBits) {
    // Original source a0=child+0x10. Source 0x36FB3C reads
    // f1 from a0+0xA4, which is child+0xB4.
    const auto status=std::bit_cast<float>(child.originalWordAt(0xB4));
    const auto f12=std::bit_cast<float>(inputBits);
    if(!std::isfinite(status) || !std::isfinite(f12) ||
       std::fpclassify(status)==FP_SUBNORMAL ||
       std::fpclassify(f12)==FP_SUBNORMAL)
        throw std::invalid_argument("source 36FB18 requires unresolved PS2 COP1 special value support");
    // Original 0x7157C0 constant is 1.0f.
    // 0x36FB40 C.OLE.S 1,status; BC1T 0x36FBF8.
    // The 0x36FB48 delay slot MOV.S f0,f12, so if status >=1
    // original method returns f12, with its exact original bits.
    if(status>=1.0f)
        return inputBits;
    // 0x36FB4C MTC1 zero,f0; 0x36FB50 C.OLT.S 0,f20;
    // 0x36FB54 BC1FL 0x36FBFC, leaving f0=positive zero.
    // Signed -0.0 input is not >0, so returns +0.0 as source.
    if(!(f12>0.0f))
        return 0u;
    // Positive movement below threshold needs source bit flags,
    // 0x36F8C8, VU0 collision and accumulation. No fake result.
    return std::nullopt;
}
} // namespace ae3
