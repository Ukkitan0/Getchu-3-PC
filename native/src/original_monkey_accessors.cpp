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
void originalMonkeyAdvanceSlotIndex(OriginalMonkeySlotFields& object) noexcept {
    // MIPS 0x571BA0: xori t7,t5,7 (test ==7)
    // MIPS 0x571BA8: addiu t5,t5,1 (wraps on 32-bit overflow)
    // MIPS 0x571BAC: movz t5,zero,t7 (only if old value == 7)
    // MIPS 0x571BB4: sw t5,0x2f0(s1)
    const auto before=object.slot_index_2f0;
    object.slot_index_2f0=before==7 ? 0 : before+std::uint32_t{1};
}
} // namespace ae3
