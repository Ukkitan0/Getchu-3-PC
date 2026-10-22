#include "ae3/original_actor_type2_vu0_target.hpp"
#include "ae3/original_vu0_geometry.hpp"
#include <array>
#include <cstring>

namespace ae3 {
OriginalVU0Vec4 originalActorType2VU0WriteController40_57bdcc(
    const std::array<std::array<std::byte,16>,3>& originalSourcePolygonVertices,
    OriginalGameRandom_36f3c0& originalSharedGameRandom,
    std::array<std::byte,16>& originalControllerVector40) noexcept {
    // 0x57BDCC/BDDC/BDF0: original 0x21B458 returns three polygon vertices.
    // 0x57BDD4/BDEC/BDFC PCPYUD s1/s3/s2,zero,v0 preserves source
    // Z/W and clears X/Y; the original VU0 inputs contain those lanes.
    std::array<OriginalVU0Vec4,3> originalVertices{};
    for(std::size_t i=0;i<3;++i)
        std::memcpy(&originalVertices[i],originalSourcePolygonVertices[i].data(),16);

    // 0x708A24/28/2C (1,0.8,~1/15) and the separate 0x708A64/68/6C
    // are bitwise-identical original scalar constants. Original
    // 0x57BE00/BE24 draws two numbers using 0x36F4D4/0x36F548;
    // 0x57BE8C/BEAC/BEC0, 0x57BEC8/BECC execute the same source-ordered
    // three VMULx and two VADD operations as recovered 0x57C98C.
    // Calling that native function ONCE consumes exactly the correct
    // number of draws and preserves original lane/operand ordering.
    const auto selected=originalActorRandomTriangle_57c98c(
        originalVertices,originalSharedGameRandom);

    // Original 0x57BED0 branch to cleanup executes 0x57BED4
    // SQC2 VF3,0x40(s4) in its delay slot, writing ALL 16 bytes.
    // 'originalControllerVector40' is caller-owned native storage,
    // never a guest address or an invented gameplay target.
    std::memcpy(originalControllerVector40.data(),&selected,16);
    return selected;
}
} // namespace ae3
