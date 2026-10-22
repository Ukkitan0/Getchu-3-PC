#pragma once
#include "ae3/original_actor_random_triangle.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace ae3 {
// Original 0x57BDD4/BDEC/BDFC PCPYUD source vertex extraction,
// 0x57BE8C/BEAC/BEC0 VMULx + 0x57BEC8/BECC VADD,
// then 0x57BED4 SQC2 VF3,controller+0x40 in the branch delay slot.
// The navigation workstream supplies the three genuine 0x21B458 bytes.
OriginalVU0Vec4 originalActorType2VU0WriteController40_57bdcc(
    const std::array<std::array<std::byte,16>,3>& originalSourcePolygonVertices,
    OriginalGameRandom_36f3c0& originalSharedGameRandom,
    std::array<std::byte,16>& originalControllerVector40) noexcept;
} // namespace ae3
