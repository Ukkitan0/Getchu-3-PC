#include "ae3/original_actor_controller_vector_compare.hpp"
#include <cstring>
namespace ae3 {
OriginalActorControllerCompare_57c1d4 originalActorControllerCompare_57c1d4(
    bool variant30,
    const std::array<std::byte,16>& controller20,
    const std::array<std::byte,16>& controller40) noexcept {
    // 0x57C1D4 LBU controller+0x30; 0x57C1D8 BEQZ goes to
    // 0x57C200, returning 2 regardless of the two vectors.
    if(!variant30)return {false,2};
    // 0x57C1E0/E4: LQ t6,+0x20, LQ t7,+0x40
    // 0x57C1E8 PCEQW t7,t7,t6 -> four 0xffffffff or 0 lanes
    // 0x57C1EC PPACH t7,zero,t7 -> lower packed half carries
    // the low 16 bits of all four comparison lanes.
    // 0x57C1F0 NOR t7,zero,t7; 0x57C1F4 SLTIU t7,t7,1
    // -> true iff all source comparison words matched EXACTLY.
    // 0x57C1F8 BEQZL + 0x57C210 MOVZ produce return code
    // 2 on equal data, 1 on unequal data. IEEE float == is WRONG:
    // source PCEQW is a raw 32-bit integer comparison per lane.
    const bool eq=std::memcmp(controller20.data(),controller40.data(),16)==0;
    return {eq,eq?2u:1u};
}
} // namespace ae3
