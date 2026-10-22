#include "ae3/original_actor_fallback_vu0_random.hpp"
namespace ae3 {
void originalFallbackVu0RinitVF0w_57ca68(OriginalActorFallbackVu0R_57ca68& state) noexcept {
    // Genuine VU0 constant VF0.w=1.0f. RINIT retains the low 23 mantissa bits
    // and enforces the 0x3F800000 exponent.
    state.rawR=0x3f800000u;
    ++state.sourceVRINITCalls;
}
void originalFallbackVu0RxorVF0x_57ca68(OriginalActorFallbackVu0R_57ca68& state) noexcept {
    // VF0.x=0.0f, so source VRXOR does not change the random mantissa.
    state.rawR=0x3f800000u|(state.rawR&0x007fffffu);
    ++state.sourceVRXORCalls;
}
void originalFallbackVu0RandomCandidate_57ca68(OriginalActorFallbackVu0R_57ca68& state) noexcept {
    // Both original source paths execute precisely two pairs per candidate.
    originalFallbackVu0RinitVF0w_57ca68(state);
    originalFallbackVu0RxorVF0x_57ca68(state);
    originalFallbackVu0RinitVF0w_57ca68(state);
    originalFallbackVu0RxorVF0x_57ca68(state);
}
} // namespace ae3
