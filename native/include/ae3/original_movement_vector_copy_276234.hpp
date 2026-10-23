#pragma once
#include <array>
#include <cstdint>
#include <stdexcept>

namespace ae3 {
// Original USA 0x276234..0x276240 consists of only:
// 276234 LQ t7,0(a1)
// 276238 JR ra
// 27623C SQ t7,0x40(a0) in the JR delay slot.
// Original source-window SHA-256:
// 0f7121db3eab577e6cd5103201965a66f1a1d805730f691c4979b625e021c034
//
// This function is called from BOTH original 30C0A0 branches,
// specifically at original 30C114 and 30C14C. Unlike guest
// ADDIU, SQ's base+displacement is EE address arithmetic.
// It is not a PC pointer and must never dereference host memory.
class OriginalMovementVectorStoreOwner_276234 {
public:
    virtual ~OriginalMovementVectorStoreOwner_276234()=default;
    // Complete 128-bit original t7 copied to actual guest
    // destination 0x40(a0), including all high lanes.
    virtual void store_original_128bit_at_guest(
        std::uint64_t originalGuestDestination,
        const std::array<std::uint32_t,4>& originalT7)=0;
};
inline void originalMovementVectorCopy_276234(
    std::uint64_t originalA0GuestVA,
    const std::array<std::uint32_t,4>& originalLoadedA1,
    OriginalMovementVectorStoreOwner_276234& genuineOwner) {
    // Retail SQ demands aligned 16-byte original destination.
    // Source 30C0A0 obtains a0 from its live signed EE parent.
    // An invalid alignment cannot become a successful host write.
    const auto dest=originalA0GuestVA+0x40ull;
    if((dest&15ull)!=0ull)
        throw std::invalid_argument(
            "original 27623C SQ source guest address not 16-byte aligned");
    genuineOwner.store_original_128bit_at_guest(dest,originalLoadedA1);
}
} // namespace ae3
