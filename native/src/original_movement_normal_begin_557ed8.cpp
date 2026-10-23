#include "ae3/original_movement_normal_begin_557ed8.hpp"

namespace ae3 {
void originalMovementNormalBegin_557ed8(
    OriginalMovementChildD0_30c57c& child,
    std::uint64_t originalParentWord14SignExtended,
    OriginalMovementNormalCalls_557ed8& calls) {
    // Source 0x557ED8 MTC1 zero,f20. Source 0x557EE4 SWC1
    // child+0xB4=0. 0x557EEC is 0x557EE8 JAL's delay slot,
    // clearing source child+0xB8 BEFORE 0x317B9C returns.
    // Retail 0x557EE0 reads child+0x09 BEFORE 0x557EE4
    // writes zero to +0xB4; 0x557EEC writes zero to +0xB8
    // in the 0x317B9C JAL delay slot.
    const auto mode=child.originalByteAt(0x09);
    if(!mode)
        throw std::logic_error("original movement child+0x09 mode is unknown");
    child.writeWord(0xB4,0u);
    child.writeWord(0xB8,0u);

    // Source 0x557EE8 JAL genuine 0x317B9C; its full 14-word
    // branch/selection semantics are translated natively.
    // Original 0x557EE8 JAL executes its +0xB8-zero delay slot
    // BEFORE genuine 0x317B9C's reads of original parent fields.
    const auto resourceFields=calls.read_original_parent_resource_fields(
        child.originalWordAt(4));
    const auto sourceContext=originalManagerSelectResource_317b9c(
        resourceFields,std::to_integer<unsigned>(*mode));

    // Source 0x557F00: original 0x276B68(parent+0x14,
    // sourceContext,0,0, f12=0). Original callback owns effects.
    const auto collisionState=
        calls.call_original_276b68(originalParentWord14SignExtended,sourceContext);
    if(collisionState!=0u)
        calls.call_original_2767f4(originalParentWord14SignExtended);

    // Original 0x557F20 always calls 0x2773D4(parent+0x14,0,0).
    calls.call_original_2773d4(originalParentWord14SignExtended);

    // Original 0x557F2C SB child+0xC0 = 1.
    // Original next 0x557F30 reloads child+0x09 for 0x317B34
    // before 0x36FB18; leave those future calls to the real owner.
    child.writeByte(0xC0,std::byte{1});
}
} // namespace ae3
