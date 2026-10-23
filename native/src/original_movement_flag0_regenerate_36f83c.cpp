#include "ae3/original_movement_flag0_regenerate_36f83c.hpp"
#include <stdexcept>

namespace ae3 {
OriginalMovementRegenerateResult_36f8c8 originalMovementRegenerateFlag0_36f83c(
    const OriginalMovementRegenerateSource_36f8c8& source) {
    // Genuine retail 36F83C and 36F8C8 VU0 operations,
    // destination masks and sources are byte-for-byte equal.
    // Share only that proven source arithmetic, not pointer
    // tag branches or the different original status flag.
    auto result=originalMovementRegenerate_36f8c8(source);
    result.originalFlagsA8After=source.originalFlagsA8|1u;
    return result;
}
OriginalMovementRegenerateResult_36f8c8 originalMovementRegenerateFlag0Owned_36f83c(
    OriginalMovementChildD0_30c57c& child,
    std::uint64_t genuineScratchVA,
    OriginalMovementFlag0Owner_36f83c& owner) {
    // Source 557FC8 tests child+B8 bit0; only if clear does
    // 558084 call this original function with a0=child+0x10.
    const auto flags=child.originalWordAt(0xB8);
    if((flags&1u)!=0u)
        throw std::invalid_argument("original 558084 bit0 regeneration not selected");
    const auto parentVA=child.originalWordAt(4);
    const OriginalMovementRegenerateSource_36f8c8 input{
        genuineScratchVA,child.originalWordAt(0xB0),
        child.originalWordAt(0xB4),flags,
        owner.read_original_vf1_through_vf8(parentVA)};
    const auto result=originalMovementRegenerateFlag0_36f83c(input);
    // Original 36F884/898/88C..B8 COP2 side effects must be
    // published to actual owner before source SW flag+A8.
    owner.publish_original_36f83c_vu0(result);
    child.writeWord(0xB8,result.originalFlagsA8After);
    return result;
}
} // namespace ae3
