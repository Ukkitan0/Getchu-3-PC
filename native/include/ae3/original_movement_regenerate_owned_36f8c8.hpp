#pragma once
#include "ae3/original_movement_regenerate_36f8c8.hpp"
#include "ae3/original_movement_child_30c57c.hpp"
#include <array>
#include <cstdint>

namespace ae3 {
class OriginalMovementRegenerationOwner_36f8c8 {
public:
    virtual ~OriginalMovementRegenerationOwner_36f8c8()=default;
    virtual std::array<std::array<std::uint32_t,4>,8>
    read_original_vf1_through_vf8(std::uint32_t genuineParentGuestVA)=0;
    // Publish ALL source VF1,VF4,ACC and return-v0 register state.
    // Called before source child+B8 flag becomes visible to C++.
    virtual void publish_original_36f8c8_vu0(
        const OriginalMovementRegenerateResult_36f8c8& original)=0;
};
// Actual 36FC18 JAL 36F8C8, only for source child+B8 bit1
// CLEAR. True game-owner interface, NOT alternative steering.
// Returns original 128-bit GPR v0; downstream 36FB70 actor
// movement and collision are separate unfinished operations.
OriginalMovementRegenerateResult_36f8c8 originalMovementRegenerateOwned_36f8c8(
    OriginalMovementChildD0_30c57c& child,
    std::uint64_t genuineChildScratchGuestVA,
    OriginalMovementRegenerationOwner_36f8c8& owner);
} // namespace ae3
