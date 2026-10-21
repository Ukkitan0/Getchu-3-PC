#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
namespace ae3 {
// Source-native value-typed version of USA 0x36F3C0 game RNG.
// Retail object: 624 u32 words and cursor at byte +0x9C0.
// 623 source twists precede the first draw. Unlike std::mt19937,
// original 0x36F444 reads the cursor word BEFORE advancing it.
class OriginalGameRandom_36f3c0 {
public:
    static constexpr std::size_t kStateCount=624;
    static constexpr std::uint32_t kRetailDefaultSeed=0x1571;
    explicit OriginalGameRandom_36f3c0(std::uint32_t seed=kRetailDefaultSeed);
    void originalSeed_36f3c0(std::uint32_t seed);
    void originalTwistOne_36f2dc() noexcept;
    std::uint32_t originalTemperedWord_36f444() noexcept;
    std::uint32_t originalRandom31_36f4ac() noexcept;
    float originalRandomFloat_36f4d4() noexcept;
    std::int32_t originalRandomIntRange_36f508(std::int32_t begin,std::int32_t end);
    float originalRandomFloatRange_36f548(float begin,float end) noexcept;
    std::uint32_t originalCursor() const noexcept {return cursor_;}
    const std::array<std::uint32_t,kStateCount>& originalWords() const noexcept {return words_;}
private:
    std::array<std::uint32_t,kStateCount> words_{};
    std::uint32_t cursor_=0;
};
} // namespace ae3
