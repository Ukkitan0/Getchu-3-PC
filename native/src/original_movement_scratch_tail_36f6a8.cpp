#include "ae3/original_movement_scratch_tail_36f6a8.hpp"
#include <bit>
#include <cstring>
#include <stdexcept>

namespace ae3 {
namespace {
void storeWord(std::span<std::byte,0x110> frame,
               std::size_t offset,std::uint32_t value) noexcept {
    // The genuine retail EE uses 32-bit little-endian SW/SWC1.
    for(std::size_t b=0;b<4;++b)
        frame[offset+b]=std::byte((value>>(8u*b))&255u);
}
}
void originalMovementScratchScalarTail_36f6a8(
    std::span<std::byte,0x110> originalScratchFrame,
    const OriginalMovementTailSource_36f6a8& source) {
    // Caller 0x57C13C executes move t0,zero, followed by source
    // COP2 packing and 0x57C148 JAL. At 0x36F678 t3=2, then
    // 0x36F680 MOVZ t3,zero,t0 => t3=0 for this real callsite.
    // Original source a0=sp+0x20, positive 32-bit stack address.
    // High-32 BBit132 tests 36F6A0/A4/B4/B8/BC are all false.
    if(source.originalT0!=0 ||
       (source.originalGuestA0>>32)!=0 ||
       (source.originalGuestA0&15u)!=0 ||
       source.originalGuestA0==0)
        throw std::invalid_argument(
            "0x36F6A8 tail requires authentic 0x57C148 stack VA and zero t0");

    // 36F6A8: SW zero,+0xA8
    storeWord(originalScratchFrame,0xA8,0);
    // 36F6AC: SW t5,+0xA4; t5 set zero at 36F660.
    storeWord(originalScratchFrame,0xA4,0);
    // 36F6B0: SW t4,+0xA0; 36F68C-69C computes
    // 0x744DD0 + (t3<<7). Here original t3 is exactly zero.
    // This is a source game-data VA in original .bss,
    // NOT an ELF-backed static table or Windows pointer.
    storeWord(originalScratchFrame,0xA0,0x00744DD0u);
    // Source 36F6C4: SW t3,+0xAC
    storeWord(originalScratchFrame,0xAC,0);
    // Retail 36F698: C.OLT.S f1,f0, with original f0 from
    // 0x7157AC = 1.0f. 36F6C8 BC1F skips the SWC1 otherwise.
    // C.OLT.S is ordered: NaN is false. No approximate epsilon.
    const float originalF1=std::bit_cast<float>(source.originalF1Bits);
    if(originalF1<1.0f)
        storeWord(originalScratchFrame,0xA4,0x3F800000u);
}
} // namespace ae3
