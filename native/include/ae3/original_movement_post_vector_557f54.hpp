#pragma once
#include "ae3/original_movement_child_30c57c.hpp"
#include <array>
#include <cstdint>
#include <optional>

namespace ae3 {
// Source 0x557F54..0x557FC8, ordinary original untagged
// child and parent source pointer path. VF1 is loaded from
// genuine child +0xA0. Child+B8 bit1 must be set: otherwise
// 0x558094 must call the original 0x36F8C8 regeneration.
// Exact representable VRSQRT/VMUL only, not a VU emulator.
struct OriginalMovementPostVector_557f54 {
    std::array<std::uint32_t,4> originalVF1After;
    std::array<std::uint32_t,4> originalVF2After;
    std::uint32_t originalVU0Q;
    std::uint32_t originalGprS3VF2X;
    std::optional<std::uint32_t> originalSlot5EarlyStatus;
};
OriginalMovementPostVector_557f54 originalMovementPostVector_557f54(
    const OriginalMovementChildD0_30c57c& actualChild,
    std::uint64_t originalParentWord14GuestVA);
class OriginalMovementPostVectorOwner_557f54 {
public:
    virtual ~OriginalMovementPostVectorOwner_557f54()=default;
    // Owner preserves retail VF1/VF2/Q and GPR s3 side effects
    // before publishing source slot5 status1, if present.
    virtual void publish_original_557f54_vu0(
        const OriginalMovementPostVector_557f54& original)=0;
};
} // namespace ae3
