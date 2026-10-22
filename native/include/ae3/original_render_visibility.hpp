#pragma once
#include <cstdint>
#include <functional>

namespace ae3 {
// First-pass early skip gate recovered from original USA SCUS_975.01
// 0x280838-0x280884. NOT the complete retail drawing decision.
// source object $s0: u64 at +0x20, u8 at +0x30/+0x35/+0x32.
// render context $s3: u64 mask at +0x58.
// If flag32 is set, 0x26C9B8 receives context+0x2C plus
// object+0x50 and object+0xC0 before allowing this pass.
struct OriginalRenderEarlyGate {
    std::uint64_t objectMask=0;
    std::uint64_t sceneMask=0;
    std::uint8_t flag30=0,flag35=0,flag32=0;
};
// Supplies original 0x26C9B8 only; without that recovered routine, fail
// rather than inventing a frustum. True means eligible for *later* gates.
bool passesOriginalRetailRenderEarlyGate(
    const OriginalRenderEarlyGate& input,
    const std::function<bool()>& originalBoundsVisible);
} // namespace ae3
