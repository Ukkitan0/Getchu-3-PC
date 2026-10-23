#include "ae3/original_manager_resource_selector_317b9c.hpp"
#include <stdexcept>

namespace ae3 {
std::uint32_t originalManagerSelectResource_317b9c(
    const OriginalManagerResourceFields_317b9c& p,
    std::uint32_t originalModeA1) {
    // Source 0x317B9C: read (a0+0x10) FIRST.
    // 0x317BA0: ANDI a1,0xFF. 0x317BA4: read a0+0x40.
    const std::uint32_t mode=originalModeA1&0xffu;
    const std::uint32_t base=p.originalParentWord10;

    // Source 0x317BAC BEQ parent+0x40,1; delay slot
    // 0x317BB0 sets v0=base+0x1C0 EVEN when branch taken.
    if(p.originalParentWord40==1u)
        return base+0x1c0u;

    // Source 0x317BB4 BEQZL mode -> 0x317BCC.
    // Its 0x317BB8 delay slot executes ONLY on the taken
    // mode-zero branch; no source read of parent+0x34.
    if(mode==0u)
        return base+0x220u;

    // Source 0x317BBC reads a0+0x34 on the nonzero mode path.
    // 0x317BC0 computes base+0x120, 0x317BC4 base+0x140.
    // 0x317BC8 MOVZ chooses +0x120 if word34 is zero.
    if(!p.originalParentWord34)
        throw std::invalid_argument(
            "original 0x317B9C parent+0x34 not supplied");
    return base+(*p.originalParentWord34==0u?0x120u:0x140u);
}
} // namespace ae3
