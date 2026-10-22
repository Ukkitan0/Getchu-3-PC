#pragma once
#include <array>
#include <cstdint>
#include <optional>

namespace ae3 {
// Source 0x517904..0x5179A8: the retail input manager sends a two-byte
// actuator payload through original pad library callback 0x405E30.
// The actual host haptics/motor backend is intentionally external.
struct OriginalPadActuatorPacket_517940 {
    std::uint32_t originalControlCode=1; // call a1
    std::uint8_t originalSelector=3; // supplied by pointer a2
    std::uint32_t originalByteCount=2; // call a3
    std::array<std::uint8_t,2> bytes{}; // supplied by pointer a4
};
class OriginalPadActuatorController_51790c {
public:
    // The source request has one original mode byte and one float32
    // intensity; game writes them to pad object +0x14/+0x18.
    // Returns a command only when original c.eq.s/byte comparison differs.
    // Non-finite/out-of-range float-to-int conversion is NOT specified here:
    // original R5900 FCSR behavior must be recovered before supporting it.
    std::optional<OriginalPadActuatorPacket_517940> request(
        std::uint8_t originalMode,float originalIntensity) noexcept;
    void reset() noexcept {lastMode_=0;lastIntensity_=0.0f;}
private:
    std::uint8_t lastMode_=0;
    float lastIntensity_=0.0f;
};
} // namespace ae3
