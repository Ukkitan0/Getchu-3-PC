#include "ae3/original_movement_child_early_36f954.hpp"
#include <bit>
#include <cmath>
#include <cstdint>
#include <stdexcept>
namespace ae3 {
bool originalMovementChildTerminal36F954(
    const OriginalMovementChildD0_30c57c& child) {
    // Retail child update passes original a0=child+0x10.
    // Source 0x36F96C LWC1(a0+0xA4) therefore reads child+0xB4.
    const float status=std::bit_cast<float>(child.originalWordAt(0xB4));
    // For original 0x557EC8 caller, f12=0x704B04=+2.0:
    // the source 0x36F970 "f1<=0 && f12<0" early gate is
    // impossible. The second source gate at 0x36F994 is
    // C.OLE.S(1.0,f1) then 0x36F9A0 C.OLT.S(0,f12).
    // When both are true 0x36F9AC returns source v0=1.
    // Anything else enters still-unrecovered original gameplay.
    if (!std::isfinite(status))
        throw std::invalid_argument(
            "original 0x36F954 status needs missing PS2 FP semantics");
    return status>=1.0f;
}
} // namespace ae3
