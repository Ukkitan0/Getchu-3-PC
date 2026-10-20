#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace ae3 {
// The offsets and word sizes in these recovered *original* game substructures
// come directly from SCUS_975.01. Unknown fields are intentionally not named
// or fabricated. Native host code may own these C++ value types directly;
// there is no PS2 machine-state or CPU emulation involved.
struct OriginalMonkeySecondaryFields {
    std::array<std::uint8_t, 0x08> unknown_00{};
    std::uint32_t word_08 = 0;
    std::array<std::uint8_t, 0x2c> unknown_0c{};
    std::uint32_t word_38 = 0;
    std::array<std::uint8_t, 0x14> unknown_3c{};
    std::uint8_t flag_50 = 0;
};
static_assert(offsetof(OriginalMonkeySecondaryFields,word_08)==0x08);
static_assert(offsetof(OriginalMonkeySecondaryFields,word_38)==0x38);
static_assert(offsetof(OriginalMonkeySecondaryFields,flag_50)==0x50);

struct OriginalMonkeyAngleFields {
    std::array<std::uint8_t,0x37c> unknown_000{};
    float angle_degrees_37c = 0.0f;
};
static_assert(offsetof(OriginalMonkeyAngleFields,angle_degrees_37c)==0x37c);

// Actual original Monkey secondary vtable slots 0x6B14D0[0], [1], and [5].
// The actual original functions first resolve a separate secondary-object
// pointer chain; this translation operates on that resolved game subobject.
std::uint32_t originalMonkeyGetWord08(const OriginalMonkeySecondaryFields&) noexcept; // 0x571648
std::uint32_t originalMonkeyGetWord38(const OriginalMonkeySecondaryFields&) noexcept; // 0x571658
void originalMonkeySetFlag50(OriginalMonkeySecondaryFields&) noexcept;              // 0x5716C4

// Original 0x571688 -> 0x316928: after following the *real* original
// +0x20 -> +0x00 -> +0x00 pointer chain, reads float +0x37C and
// multiplies by float constant at 0x708278 (0x3C8EFA34, pi / 180).
float originalMonkeyAngleToRadians(const OriginalMonkeyAngleFields&) noexcept;
} // namespace ae3
