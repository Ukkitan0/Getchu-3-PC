#pragma once
#include <cstddef>
#include <cstdint>
#include <span>

namespace ae3 {
// Source-recovered scalar/store tail of original SCUS_975.01
// 0x36F5F4, specifically the call from 0x57C148.
// NOT the preceding VU0 mathematics or the complete function.
// The real EE 64-bit a0 (source stack+0x20) and original VU0
// f1 bits are supplied by the genuine game owner.
struct OriginalMovementTailSource_36f6a8 {
    std::uint64_t originalGuestA0;
    std::uint32_t originalT0;
    std::uint32_t originalF1Bits;
};
// Genuine original 0x36F6A8..0x36F6D8 writes into the same
// 0x110-byte scratch frame that original 0x30C57C consumes.
// Valid only for the original 0x57C148 caller: t0=0 and a0
// 32-bit positive aligned stack VA, so bbit132 branches are
// untaken. If these source conditions are absent, REFUSE.
// +0xA0 holds a guest VA to original dynamic 0x744DD0 data;
// it must never be treated as a native pointer.
void originalMovementScratchScalarTail_36f6a8(
    std::span<std::byte,0x110> originalScratchFrame,
    const OriginalMovementTailSource_36f6a8& source);
} // namespace ae3
