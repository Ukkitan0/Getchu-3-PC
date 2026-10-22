#include "ae3/original_actor_random_triangle.hpp"
#include <bit>
#include <cstring>
namespace ae3 {
OriginalActorTriangleWeights_57c98c originalActorTriangleWeights_57c98c(
    OriginalGameRandom_36f3c0& originalRandom) noexcept {
    // 0x57C9BC: RNG[global 0x6A8E48] / source 0x36F4D4
    const float first=originalRandom.originalRandomFloat_36f4d4();
    // 0x57C9D4: global source 0x708A64=1.0f
    const float remainingAfterFirst=1.0f-first;
    // 0x57C9E0: original source 0x36F548(0.0f, 1.0f-first)
    const float second=originalRandom.originalRandomFloatRange_36f548(
        0.0f,remainingAfterFirst);
    // 0x57C9EC: remainder=(1-first)-second, not a new random draw.
    const float remainder=remainingAfterFirst-second;
    constexpr float sourceScale=std::bit_cast<float>(std::uint32_t{0x3f4ccccd});
    constexpr float sourceBase=std::bit_cast<float>(std::uint32_t{0x3d888888});
    const float firstWeight=(first*sourceScale)+sourceBase;
    const float secondWeight=(second*sourceScale)+sourceBase;
    const float thirdWeight=(remainder*sourceScale)+sourceBase;
    return {first,second,remainder,{firstWeight,secondWeight,thirdWeight}};
}
OriginalVU0Vec4 originalActorRandomTriangle_57c98c(
    const std::array<OriginalVU0Vec4,3>& src,
    OriginalGameRandom_36f3c0& originalRandom) noexcept {
    const auto weights=originalActorTriangleWeights_57c98c(originalRandom);
    const auto blend=[&](float x,float y,float z) {
        // 0x57CA28 VMULx, 0x57CA40 VMULx, 0x57CA54 VMULx;
        // 0x57CA5C VADD, then 0x57CA64 VADD in this order.
        const float a=x*weights.originalWeightedCoefficients[0];
        const float b=y*weights.originalWeightedCoefficients[1];
        const float c=z*weights.originalWeightedCoefficients[2];
        return (a+b)+c;
    };
    // 0x57C994/57C9A8/57C9B8 PCPYUD zero,vertex retain ONLY
    // the vertex's upper doubleword in the EE register. QMTC2
    // copies all 128 bits, so source VF.x/y are zero, not the
    // original vertex x/y. 0x57CA28/40/54 and 5C/64 combine z/w.
    return {0.0f,0.0f,
            blend(src[0].z,src[1].z,src[2].z),
            blend(src[0].w,src[1].w,src[2].w)};
}
std::uint64_t originalActorRandomTriangleLow64_57c98c(
    const std::array<std::array<std::byte,16>,3>& bytes,
    OriginalGameRandom_36f3c0& originalRandom) noexcept {
    std::array<OriginalVU0Vec4,3> source{};
    for(std::size_t i=0;i<3;++i)
        std::memcpy(&source[i],bytes[i].data(),16);
    const auto result=originalActorRandomTriangle_57c98c(source,originalRandom);
    return std::uint64_t(std::bit_cast<std::uint32_t>(result.x)) |
         (std::uint64_t(std::bit_cast<std::uint32_t>(result.y))<<32);
}
} // namespace ae3
