#include "ae3/original_monkey_accessors.hpp"
#include <bit>

namespace ae3 {
std::uint32_t originalMonkeyGetWord08(const OriginalMonkeySecondaryFields& data) noexcept {
    return data.word_08;
}
std::uint32_t originalMonkeyGetWord38(const OriginalMonkeySecondaryFields& data) noexcept {
    return data.word_38;
}
void originalMonkeySetFlag50(OriginalMonkeySecondaryFields& data) noexcept {
    data.flag_50 = 1;
}
float originalMonkeyAngleToRadians(const OriginalMonkeyAngleFields& data) noexcept {
    // Keep the exact original 32-bit game constant, not host double pi/180.
    constexpr float factor = std::bit_cast<float>(std::uint32_t{0x3C8EFA34});
    return data.angle_degrees_37c * factor;
}
} // namespace ae3
