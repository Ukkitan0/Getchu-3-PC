#include "ae3/original_game_random.hpp"
#include <bit>
#include <stdexcept>
namespace ae3 {
OriginalGameRandom_36f3c0::OriginalGameRandom_36f3c0(std::uint32_t seed){
    originalSeed_36f3c0(seed);
}
void OriginalGameRandom_36f3c0::originalSeed_36f3c0(std::uint32_t seed){
    // 0x36F3C4..0x36F410, original multiplier 0x6C078965.
    words_[0]=seed;
    for(std::uint32_t i=1;i<624;++i){
        auto previous=words_[i-1];
        words_[i]=1812433253u*(previous^(previous>>30))+i;
    }
    cursor_=0;
    // The original 0x36F414..0x36F42C makes 623 twist calls.
    for(std::uint32_t i=0;i<623;++i)originalTwistOne_36f2dc();
}
void OriginalGameRandom_36f3c0::originalTwistOne_36f2dc() noexcept{
    const auto index=cursor_;
    const auto next=(index==623)?0u:index+1;
    const auto partner=(index<227)?index+397:index-227;
    const auto join=(words_[index]&0x80000000u)|(words_[next]&0x7fffffffu);
    words_[index]=words_[partner]^(join>>1)^((join&1)?0x9908b0dfu:0u);
    cursor_=next;
}
std::uint32_t OriginalGameRandom_36f3c0::originalTemperedWord_36f444() noexcept{
    // 0x36F460 snapshots the old word before calling 0x36F2DC.
    std::uint32_t v=words_[cursor_];
    originalTwistOne_36f2dc();
    v^=v>>11;
    v^=(v<<7)&0x9d2c5680u;
    v^=(v<<15)&0xefc60000u;
    v^=v>>18;
    return v;
}
std::uint32_t OriginalGameRandom_36f3c0::originalRandom31_36f4ac() noexcept{
    return originalTemperedWord_36f444()&0x7fffffffu;
}
float OriginalGameRandom_36f3c0::originalRandomFloat_36f4d4() noexcept{
    // 0x7157A8 holds original float32 value 2^-31, raw 0x30000000.
    const auto converted=static_cast<float>(originalRandom31_36f4ac());
    return converted*std::bit_cast<float>(std::uint32_t{0x30000000u});
}
std::int32_t OriginalGameRandom_36f3c0::originalRandomIntRange_36f508(
    std::int32_t begin,std::int32_t end){
    // Original SUBU wraps, DIV/MFHI use signed 32-bit remainder.
    const auto delta=std::bit_cast<std::int32_t>(
        std::bit_cast<std::uint32_t>(end)-std::bit_cast<std::uint32_t>(begin));
    if(delta==0)throw std::domain_error("Original 0x36F528 signed DIV by zero");
    const auto draw=static_cast<std::int32_t>(originalRandom31_36f4ac());
    const auto remainder=draw%delta;
    return std::bit_cast<std::int32_t>(
        std::bit_cast<std::uint32_t>(begin)+std::bit_cast<std::uint32_t>(remainder));
}
float OriginalGameRandom_36f3c0::originalRandomFloatRange_36f548(
    float begin,float end) noexcept{
    // The source computes end-begin, draws from 0x36F4D4, scales,
    // then adds begin; no replacement random distribution.
    const auto span=end-begin;
    const auto draw=originalRandomFloat_36f4d4();
    const auto scaled=draw*span;
    return scaled+begin;
}
} // namespace ae3
