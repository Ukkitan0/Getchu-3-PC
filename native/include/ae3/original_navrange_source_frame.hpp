#pragma once
#include "ae3/original_navrange_matrix_inverse.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>

namespace ae3 {
// The three unmodified 128-bit arguments loaded by 0x4E4110,
// 0x4E4118, and 0x4E4120 from the same navrange resource header.
// The 0x20-byte group key remains a separate keyed-map identifier.
struct OriginalNavrangeResourceFrame {
    OriginalNavrangeGpr128 argumentA0_20{};
    OriginalNavrangeGpr128 quaternionA2_30{};
    OriginalNavrangeGpr128 argumentA1_40{};
};
inline OriginalNavrangeResourceFrame originalNavrangeSourceFrame_4e4110(
    std::span<const std::byte> originalResourceHeader) {
    if(originalResourceHeader.size()<0x50)
        throw std::out_of_range("Original navrange resource is missing +0x20..+0x4F source vectors");
    const auto quad=[&](std::size_t offset) {
        OriginalNavrangeGpr128 raw{};
        for(std::size_t lane=0;lane<4;lane++) {
            const std::size_t p=offset+lane*4;
            raw[lane]=std::uint32_t(std::to_integer<unsigned char>(originalResourceHeader[p]))|
                (std::uint32_t(std::to_integer<unsigned char>(originalResourceHeader[p+1]))<<8)|
                (std::uint32_t(std::to_integer<unsigned char>(originalResourceHeader[p+2]))<<16)|
                (std::uint32_t(std::to_integer<unsigned char>(originalResourceHeader[p+3]))<<24);
        }
        return raw;
    };
    return {quad(0x20),quad(0x30),quad(0x40)};
}
// The source 0x21EF9C feeds A2 (header+0x30) to 0x359E74;
// 0x21EFDC then inverts its matrix via 0x359F44. Existing
// inverse is mathematical cofactor code, not bit-exact VU0.
inline OriginalNavrangeMatrix4 originalNavrangeFrameInverse_21ef9c_21efdc(
    const OriginalNavrangeResourceFrame& frame) noexcept {
    return originalNavrangeInverseQuaternion_21efa4_21efdc(frame.quaternionA2_30);
}
} // namespace ae3
