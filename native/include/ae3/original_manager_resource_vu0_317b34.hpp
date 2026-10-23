#pragma once
#include "ae3/original_manager_resource_float_317b34.hpp"
#include <array>
#include <cstdint>

namespace ae3 {
// Source 0x317B58..0x317B90: ordinary EXACT finite-number subset of
// original PS2 VU0 VMUL/VADDy/VADDz + EE COP1 SQRT/ADD/MIN.
// All four VF1 lanes and both table floats MUST originate from the
// original live game owner; none are inferred from host transforms.
struct OriginalResourceVU0Inputs_317b34 {
    std::uint64_t originalParentGuestVA;
    std::array<std::uint32_t,4> originalVF1Raw;
    std::uint32_t originalTable364Raw;
    std::uint32_t originalTable368Raw;
};
struct OriginalResourceVU0Result_317b34 {
    std::uint32_t originalF0Raw;
    std::array<std::uint32_t,4> originalVF1After;
};

// Call only for original parent+0x40 !=1, mode low8 !=0.
// Source parent guest VA must be untagged and 16-byte aligned,
// so the 0x317B58 BBit032 alternate entry is not selected.
// PS2 VU/COP1 non-IEEE special cases and INEXACT arithmetic
// are rejected until the original rounding semantics are recovered.
// Returns original COP1 f0 and all real VU0 VF1 resulting lanes.
// Never reinterpret guest VA as a Windows pointer.
OriginalResourceVU0Result_317b34 originalManagerResourceVU0_317b34(
    const OriginalManagerResourceFloatFields_317b34& parent,
    std::uint32_t mode,
    const OriginalResourceVU0Inputs_317b34& originalSource);
} // namespace ae3
