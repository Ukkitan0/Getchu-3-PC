#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
namespace ae3 {
// Native decoding of original 0x27C614 render source pointer-pair
// boundaries; does NOT decode the five payload record formats.
struct OriginalRenderModelStream {
    std::uint32_t originalBegin=0;
    std::uint32_t originalEnd=0;
    std::uint32_t recordStride=0;
    std::uint32_t recordCount=0;
    std::uint8_t sourceBeginField=0;
    std::uint8_t sourceEndField=0;
};
struct OriginalRenderModelStreamTable {
    std::array<OriginalRenderModelStream,5> streams{};
};
// Reads source root fields with 32-bit LE guest token semantics; never
// casts source guest pointers into host addresses.
OriginalRenderModelStreamTable originalDecodeModelStreams27c614(
    std::span<const std::uint8_t> originalModelRoot);
// Optional original actor/model +0x54,+0x58 pair iterated at 0x27C5CC.
OriginalRenderModelStream originalDecodeActorSupplement27c520(
    std::span<const std::uint8_t> originalModelRoot);
} // namespace ae3
