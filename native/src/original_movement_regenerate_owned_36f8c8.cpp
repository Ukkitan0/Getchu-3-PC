#include "ae3/original_movement_regenerate_owned_36f8c8.hpp"
#include <stdexcept>

namespace ae3 {
OriginalMovementRegenerateResult_36f8c8 originalMovementRegenerateOwned_36f8c8(
    OriginalMovementChildD0_30c57c& child,
    std::uint64_t originalScratchVA,
    OriginalMovementRegenerationOwner_36f8c8& owner) {
    // Genuine 36FC18 source branch only when child+B8 bit1
    // is clear. Source +A0/+A4/+A8 correspond to 0x30C57C
    // copied data at child +B0/+B4/+B8, respectively.
    const auto pointer=child.originalWordAt(0xB0);
    const auto status=child.originalWordAt(0xB4);
    const auto flags=child.originalWordAt(0xB8);
    if((flags&2u)!=0u)
        throw std::invalid_argument(
            "original 0x36FC18 regeneration requires source bit1 clear");
    const auto parentVA=child.originalWordAt(4);
    const OriginalMovementRegenerateSource_36f8c8 input{
        originalScratchVA,pointer,status,flags,
        owner.read_original_vf1_through_vf8(parentVA)};
    // Calculate and validate all source operations before
    // making any observable native child-state writes.
    const auto result=originalMovementRegenerate_36f8c8(input);
    // Retail 0x36F910/0x36F924 sets VF4/VF1 and SPECIAL2
    // changes ACC. Owner must publish the COMPLETE result.
    owner.publish_original_36f8c8_vu0(result);
    // Retail 0x36F91C ORI bit1, 0x36F920 SW flags.
    // The typed child+0xB8 is scratch+0xA8.
    child.writeWord(0xB8,result.originalFlagsA8After);
    return result;
}
} // namespace ae3
