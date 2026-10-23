#include "ae3/original_movement_vu0_scalar_36f5f4.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <cmath>
using namespace ae3;
static void put(std::array<std::byte,16>& v,std::size_t n,float f) {
    const auto raw=std::bit_cast<std::uint32_t>(f);
    for(unsigned i=0;i<4;++i)v[n*4+i]=std::byte((raw>>(8u*i))&255u);
}
static bool rejects(const OriginalMovementVU0ScalarSource_36f5f4& v) {
    try{(void)originalMovementVU0Scalars_36f5f4(v);}catch(const std::invalid_argument&){return true;}
    return false;
}
int main() {
    OriginalMovementVU0ScalarSource_36f5f4 source{};
    // Exact source VSUB/VMUL/VADDy/VADDz. First squared=9,
    // second=0.25, so source 0x36F640 first comparison false,
    // and source 0x36F698 second comparison true.
    put(source.originalA1,0,1);put(source.originalA1,1,2);put(source.originalA1,2,3);
    put(source.originalA2,0,3);put(source.originalA2,1,4);put(source.originalA2,2,4);
    put(source.originalA3,0,1.5f);put(source.originalA3,1,2);put(source.originalA3,2,3);
    auto output=originalMovementVU0Scalars_36f5f4(source);
    assert(std::bit_cast<float>(output.originalFirstLengthSquaredBits)==9);
    assert(std::bit_cast<float>(output.originalSecondLengthSquaredBits)==0.25f);
    // W isn't included in the original resulting VF*.x reduction.
    put(source.originalA1,3,250);put(source.originalA2,3,-100);put(source.originalA3,3,9000);
    output=originalMovementVU0Scalars_36f5f4(source);
    assert(std::bit_cast<float>(output.originalFirstLengthSquaredBits)==9);
    assert(std::bit_cast<float>(output.originalSecondLengthSquaredBits)==0.25f);
    // The scalar written to f1 at source 0x36F664 must use
    // VF2 (A3-A1), not the first length A2-A1.
    put(source.originalA3,0,2); // len²=1, comparison is false
    output=originalMovementVU0Scalars_36f5f4(source);
    assert(std::bit_cast<float>(output.originalSecondLengthSquaredBits)==1);
    // Rounded stage data, source branch handles exact zero.
    put(source.originalA3,0,1);
    output=originalMovementVU0Scalars_36f5f4(source);
    assert(output.originalSecondLengthSquaredBits==0);
    // Preserve source strong-failure behavior for unrecovered
    // PS2 VU0 nonordinary FP inputs.
    put(source.originalA3,0,std::numeric_limits<float>::quiet_NaN());
    assert(rejects(source));
    put(source.originalA3,0,std::numeric_limits<float>::infinity());
    assert(rejects(source));
    put(source.originalA3,0,std::numeric_limits<float>::denorm_min());
    assert(rejects(source));
    // Nonfinite W is not consumed in the source XYZ scalar path.
    put(source.originalA3,0,2);
    put(source.originalA3,3,std::numeric_limits<float>::quiet_NaN());
    assert(std::bit_cast<float>(
        originalMovementVU0Scalars_36f5f4(source).originalSecondLengthSquaredBits)==1);
    std::cout << "ORIGINAL_MOVEMENT_VU0_36F5F4_SCALARS_PASSED\n";
}
