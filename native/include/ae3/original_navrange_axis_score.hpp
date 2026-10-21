#pragma once
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
namespace ae3 {
// Recovered scalar FPU subgraph of retail 0x21EF3C. These arguments
// are the local VF-derived coordinates *after* the original setup,
// not raw navrange resource fields. The setup remains untranslated.
// A positive result excludes a group polygon at retail 0x21F75C.
inline float originalNavrangeAxisScore_21f20c(
    const std::array<float,3>& coordinate,
    const std::array<float,3>& halfExtent) noexcept {
    float score=0.0f; // 0x21F1AC: MTC1 zero, F20
    for(std::size_t i=0;i<3;++i){
        const float x=coordinate[i], r=halfExtent[i];
        const float negativeR=-r; // 0x21F208 and corresponding axes
        // 0x21F21C / 0x21F2C8 / 0x21F374: C.OLT.S F0,-F1
        if(x<negativeR) {
            const float delta=x+r;
            const float square=delta*delta;
            score=score+square;
        } else if(r<x) {
            // 0x21F52C / 0x21F4B0 / 0x21F434: C.OLT.S F1,F0
            const float delta=x-r;
            const float square=delta*delta;
            score=score+square;
        }
    }
    return score; // 0x21F3FC MOV.S F0,F20
}

// Scalar accessors at retail 0x44BD5C and 0x44BD80 use original
// variable quadword shifting (MTSAB + QFSRV) and bit masking to
// read/replace a 32-bit lane. No host numerical float conversion.
using OriginalNavrangePacked4 = std::array<std::uint32_t,4>;
inline float originalNavrangeLaneGet_44bd5c(
    const OriginalNavrangePacked4& source,std::size_t index){
    if(index>=4)throw std::out_of_range("Original navrange lane index");
    return std::bit_cast<float>(source[index]);
}
inline OriginalNavrangePacked4 originalNavrangeLaneSet_44bd80(
    OriginalNavrangePacked4 source,std::size_t index,float newValue){
    if(index>=4)throw std::out_of_range("Original navrange lane index");
    source[index]=std::bit_cast<std::uint32_t>(newValue);
    return source;
}
} // namespace ae3
