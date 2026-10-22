#include "ae3/original_pad_actuator.hpp"
#include <cmath>
#include <cstdint>

namespace ae3 {
std::optional<OriginalPadActuatorPacket_517940>
OriginalPadActuatorController_51790c::request(
    std::uint8_t originalMode,float originalIntensity) noexcept {
    // Retail 0x51790C..0x517938: skip if both byte and float compare equal.
    // Initial state is zero from retail initializer 0x517C1C.
    if(lastMode_==originalMode && lastIntensity_==originalIntensity)
        return std::nullopt;
    // R5900 CVT.W.S invalid operation response and FCSR rounding overrides
    // are still unknown. Do not pretend these exceptional values are valid.
    const float scaled=originalIntensity*255.0f; // 0x6ED0C4
    if(!std::isfinite(scaled) || scaled< -2147483648.0f ||
       static_cast<double>(scaled)>=2147483648.0)
        return std::nullopt;
    const double rounded=std::nearbyint(static_cast<double>(scaled));
    const auto signedWord=static_cast<std::int32_t>(rounded);
    const std::uint32_t bits=static_cast<std::uint32_t>(signedWord);
    // Retail 0x517988..0x5179A4: first byte is (w<<1)&255,
    // OR 1 if mode!=0. Second byte is (w>>7)&255. Both truncated to u8.
    OriginalPadActuatorPacket_517940 out;
    out.bytes[0]=static_cast<std::uint8_t>(((bits<<1) & 0xffu) |
                                                 (originalMode!=0 ? 1u : 0u));
    out.bytes[1]=static_cast<std::uint8_t>((bits>>7)&0xffu);
    lastMode_=originalMode;
    lastIntensity_=originalIntensity;
    return out;
}
} // namespace ae3
