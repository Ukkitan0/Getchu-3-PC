#pragma once
#include "ae3/original_render_instances.hpp"
#include <cstdint>
#include <functional>
#include <vector>
namespace ae3 {
// Decoded retail owner 0x280B7C: frame counter +0x40, scheduling mode
// +0x38, interval +0x3C, owner vector at +0x34. Each vector record has
// stride 0x90 and byte flags at +0x04/+0x05.
struct OriginalRetailRenderSlot {
    OriginalRenderId id=0;
    std::uint8_t pendingValidation04=0;
    std::uint8_t active05=0;
};
struct OriginalRetailRenderOwner {
    std::uint64_t frame40=0;
    std::int32_t schedulingMode38=0;
    std::int32_t schedulingPeriod3c=1;
    std::vector<OriginalRetailRenderSlot> slots;
};
using OriginalRetailInitializeSlot=std::function<void(OriginalRetailRenderSlot&)>;
struct OriginalRetailRenderOwnerTick {
    std::uint64_t frame=0;
    std::uint32_t validated=0,active=0;
    std::vector<OriginalRenderId> scheduledIds;
};
// Source scheduling and object-activation logic ONLY, not an original GPU
// renderer. Unrecovered retail 0x51CDAC callback must be supplied when a
// record's +0x04 is zero; no substitute activation or fake assets.
OriginalRetailRenderOwnerTick advanceOriginalRetailRenderOwner(
    OriginalRetailRenderOwner& owner,
    const OriginalRetailInitializeSlot& originalValidateSlot);
} // namespace ae3
