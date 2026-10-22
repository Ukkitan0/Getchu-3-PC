#pragma once
#include <cstddef>
#include <cstdint>
namespace ae3 {
// Native translation of the VF0-specific VU0 R side effects of
// original USA 0x57CA68. NOT a generic VU interpreter.
struct OriginalActorFallbackVu0R_57ca68 {
    std::uint32_t rawR=0; // original owner-provided VU0 VI[R]
    std::size_t sourceVRINITCalls=0;
    std::size_t sourceVRXORCalls=0;
};
// Source first candidate: 0x57CB90/0x57CB94 and 0x57CBB0/0x57CBB4.
// Every repeated candidate: 0x57CC88/0x57CC8C and 0x57CCA8/0x57CCAC.
// Source VRINIT VF0.w (always 1.0f), then VRXOR VF0.x (always 0.0f).
void originalFallbackVu0RinitVF0w_57ca68(OriginalActorFallbackVu0R_57ca68&) noexcept;
void originalFallbackVu0RxorVF0x_57ca68(OriginalActorFallbackVu0R_57ca68&) noexcept;
void originalFallbackVu0RandomCandidate_57ca68(OriginalActorFallbackVu0R_57ca68&) noexcept;
} // namespace ae3
