#include "ae3/original_manager_resource_float_317b34.hpp"
#include <stdexcept>
namespace ae3 {
std::uint32_t originalManagerResourceFloatBits_317b34(
    const OriginalManagerResourceFloatFields_317b34& original,
    std::uint32_t originalA1Mode) {
    // Source 0x317B34 LW table base from parent+0x08;
    // 0x317B38 loads parent+0x40 and 0x317B40 branches if !=1.
    // Source 0x317B44 ANDI mode lowbyte executes in delay slot.
    if(original.originalParentWord40==1) {
        // 0x317B48 JR and delay-slot 0x317B4C LWC1
        // f0,table+0x374.
        if(!original.originalFloatAt374Bits)
            throw std::invalid_argument(
                "original 0x317B34 table+0x374 source COP1 word missing");
        return *original.originalFloatAt374Bits;
    }
    if((originalA1Mode&0xffu)==0u) {
        // 0x317B50 BEQZ mode -> source 0x317B94 JR
        // and delay-slot 0x317B98 LWC1 f0,table+0x3B4.
        if(!original.originalFloatAt3B4Bits)
            throw std::invalid_argument(
                "original 0x317B34 table+0x3B4 source COP1 word missing");
        return *original.originalFloatAt3B4Bits;
    }
    // Source 0x317B58 BBit032 parent pointer and
    // 0x317B60..0x317B90 original VU0/SQRT path.
    throw std::logic_error(
        "original 0x317B34 nonzero-mode VU0 resource metric unrecovered");
}
} // namespace ae3
