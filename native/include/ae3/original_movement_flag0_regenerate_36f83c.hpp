#pragma once
#include "ae3/original_movement_regenerate_36f8c8.hpp"
#include "ae3/original_movement_child_30c57c.hpp"
#include <array>
#include <cstdint>

namespace ae3 {
// Complete original USA 0x36F83C..0x36F8C8 untagged
// VF1/VF4/ACC polynomial. Eight source VU0 opcodes and three
// PPACW instructions are identical to original 0x36F8C8,
// but source 0x36F890 ORI sets flag bit 0, NOT bit 1.
OriginalMovementRegenerateResult_36f8c8 originalMovementRegenerateFlag0_36f83c(
    const OriginalMovementRegenerateSource_36f8c8& genuineSource);
class OriginalMovementFlag0Owner_36f83c {
public:
    virtual ~OriginalMovementFlag0Owner_36f83c()=default;
    virtual std::array<std::array<std::uint32_t,4>,8>
    read_original_vf1_through_vf8(std::uint32_t originalParentGuestVA)=0;
    virtual void publish_original_36f83c_vu0(
        const OriginalMovementRegenerateResult_36f8c8& original)=0;
};
// Original 0x558084 JAL 0x36F83C only when child+B8 bit0 is
// CLEAR. Publishes actual VF1/VF4/ACC first, then child+B8 flag.
// No guest address is ever dereferenced as a host pointer.
OriginalMovementRegenerateResult_36f8c8 originalMovementRegenerateFlag0Owned_36f83c(
    OriginalMovementChildD0_30c57c& child,
    std::uint64_t genuineChildScratchGuestVA,
    OriginalMovementFlag0Owner_36f83c& genuineOwner);
} // namespace ae3
