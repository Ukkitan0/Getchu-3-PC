#include "ae3/original_render_owner_schedule.hpp"
#include <stdexcept>
#include <unordered_set>
namespace ae3 {
OriginalRetailRenderOwnerTick advanceOriginalRetailRenderOwner(
    OriginalRetailRenderOwner& owner,
    const OriginalRetailInitializeSlot& validateSlot){
    if(owner.slots.size()>100000)
        throw std::runtime_error("retail render owner slot budget exceeded");
    OriginalRetailRenderOwnerTick out;
    // Retail 0x280BB0-0x280BC0: ld/daddiu/sd at owner +0x40.
    owner.frame40+=1; // std::uint64_t wraps just like retail daddiu.
    out.frame=owner.frame40;
    // Retail 0x280BB4/0x280BC8-0x280BD8: signed period clamped to 1.
    const std::uint64_t period=owner.schedulingPeriod3c<1?
        1:static_cast<std::uint32_t>(owner.schedulingPeriod3c);
    std::uint64_t ordinal=0;
    std::unordered_set<OriginalRenderId> seen;
    for(auto& slot:owner.slots){
        if(!slot.id||!seen.insert(slot.id).second)
            throw std::runtime_error("invalid/duplicate original render owner ID");
        // Retail 0x280BE8-0x280C04: call 0x51CDAC only if byte4
        // equals zero, then always clear byte4 in the branch delay slot.
        if(!slot.pendingValidation04){
            if(!validateSlot)
                throw std::runtime_error("unrecovered 0x51CDAC validator unavailable");
            validateSlot(slot);
            ++out.validated;
        }
        slot.pendingValidation04=0;
        if(!slot.active05)continue;
        ++out.active;
        // Retail 0x280C1C-0x280C4C: mode==1 uses 0x4233F8,
        // the 64-bit remainder helper, to select matching active ordinal.
        // Other scheduling modes include every active record.
        const bool selected=owner.schedulingMode38!=1 ||
                            (out.frame%period)==ordinal;
        ++ordinal; // 0x280C3C/0x280C6C: increment for each active record.
        if(selected)out.scheduledIds.push_back(slot.id);
    }
    return out;
}
} // namespace ae3
