#include "ae3/original_movement_child_30c57c.hpp"

namespace ae3 {
std::unique_ptr<OriginalMovementChildD0_30c57c>
originalMovementChildCreate_30c57c(
    std::uint32_t originalParentGuestVA,
    std::span<const std::byte,0x110> originalScratch,
    const std::bitset<0x110>& sourceKnown,
    std::uint32_t originalA2,std::uint32_t originalA3) {
    // 0x30C5A4/JAL 0x41B800: allocate EXACT retail size 0xD0.
    auto child=std::make_unique<OriginalMovementChildD0_30c57c>();
    // 0x30C5B0/B8/BC: original +4 parent, +8 byte a2,
    // source +0 vtable pointer. 0x30C5C4 writes byte a3 at +9.
    child->writeWord(0,OriginalMovementChildD0_30c57c::originalVtableVA);
    child->writeWord(4,originalParentGuestVA);
    child->writeByte(8,std::byte(originalA2&0xFFu));
    child->writeByte(9,std::byte(originalA3&0xFFu));
    // 0x30C5CC..0x30C618 loops over the first 0x80 source bytes
    // in 0x40-byte strides, then 0x30C61C..0x30C650 copies six
    // more doublewords at source +0x80..+0xA8.
    // Exactly 0xB0 bytes -> target object +0x10..+0xBF.
    for(std::size_t i=0;i<0xB0;++i) {
        // Retail physically copies each source doubleword. When
        // the original source stack byte was never initialized,
        // copying it cannot establish a known game value.
        if(sourceKnown.test(i))
            child->writeByte(0x10+i,originalScratch[i]);
    }
    // 0x30C658: original byte at +0xC0 cleared AFTER copy.
    child->writeByte(0xC0,std::byte{0});
    // Source does NOT write +0x0A..0x0F or +0xC1..0xCF;
    // retain unknown-byte status instead of claiming zeroes.
    return child;
}
std::unique_ptr<OriginalMovementChildD0_30c57c>
originalMovementChildCreate_30c57c(
    std::uint32_t originalParentGuestVA,
    std::span<const std::byte,0x110> originalScratch,
    std::uint32_t originalA2,std::uint32_t originalA3) {
    // This overload is valid ONLY when the owner has supplied and
    // established every byte of original 0x110 scratch. Keeping
    // it preserves existing genuine full-source-data call sites.
    std::bitset<0x110> allKnown;
    allKnown.set();
    return originalMovementChildCreate_30c57c(
        originalParentGuestVA,originalScratch,allKnown,
        originalA2,originalA3);
}
} // namespace ae3
