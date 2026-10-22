#include "ae3/original_retail_pad.hpp"
#include <algorithm>
#include <cmath>

namespace ae3 {
int OriginalHostPadChannels::queryOriginalPadChannel_4059e8(
    std::uint32_t channel) {
    if(!snapshot_.connected)return -1;
    if(channel<16)return (snapshot_.pressedMask>>channel)&1u;
    if(channel<=0x13)return snapshot_.sticks[channel-0x10];
    return -1;
}
OriginalRetailAnalogVector originalRetailAnalog_517788(
    std::uint8_t x,std::uint8_t y) noexcept {
    // 0x6ED0AC = 0x3C088888 (float 1/120 approx), applied to
    // (byte-128). Retail 0x5177B4/0x5177C4 accumulate the squares.
    constexpr float scale=0.00833333283662796f;
    const float vx=static_cast<float>(int(x)-128)*scale;
    const float vy=static_cast<float>(int(y)-128)*scale;
    const float lengthSquared=vx*vx+vy*vy;
    // 0x6ED0B0: 0.140625 = 0.375^2, c.olt.s at 0x5177CC.
    if(lengthSquared<0.140625f)return {};
    // 0x5177F4: SQRT.S magnitude. 0x6ED0BC = 0.625,
    // 0x6ED0C0 = 0.375; original computes (r-.375)/(r*.625).
    const float length=std::sqrt(lengthSquared);
    const float ratio=(length-0.375f)/(length*0.625f);
    // PS2 MAX.S/MIN.S @ 0x51783C/40, 0x51784C/50, original
    // source limits each component, not entire vector magnitude.
    return {std::clamp(vx*ratio,-1.0f,1.0f),
            std::clamp(vy*ratio,-1.0f,1.0f)};
}
OriginalRetailGamePadState_26eb34 originalRetailGamePadState_26eb34(
    const OriginalRetailPadSample& source) noexcept {
    return {source.originalButtonBits,source.originalNewPressBits,
            source.right.x,source.right.y,source.left.x,source.left.y};
}
OriginalRetailPadSample OriginalRetailPadReader::sample(
    OriginalRetailPadChannelSource& source,bool connected) {
    OriginalRetailPadSample out;
    out.connected=connected;
    if(connected){
        // Source 0x5179B0: 16 queries through 0x4059E8,
        // then four queries (0x12,0x13,0x10,0x11).
        for(std::size_t i=0;i<OriginalRetailButtonChannels_6ed0c8.size();++i)
            if(source.queryOriginalPadChannel_4059e8(
                    OriginalRetailButtonChannels_6ed0c8[i])!=0)
                out.originalButtonBits|=static_cast<std::uint16_t>(1u<<i);
        for(std::size_t i=0;i<OriginalRetailAxisChannels_5179b0.size();++i)
            out.originalAxisBytes[i]=static_cast<std::uint8_t>(
                source.queryOriginalPadChannel_4059e8(
                    OriginalRetailAxisChannels_5179b0[i]));
        out.right=originalRetailAnalog_517788(
            out.originalAxisBytes[0],out.originalAxisBytes[1]);
        out.left=originalRetailAnalog_517788(
            out.originalAxisBytes[2],out.originalAxisBytes[3]);
    }
    // Retail 0x517898..0x5178AC: new press = now & ~last held;
    // on disconnect 0x5178EC..0x517900 the button mask is zero,
    // and the axis bytes are all 0x80.
    out.originalNewPressBits=static_cast<std::uint16_t>(
        out.originalButtonBits & ~previous_);
    out.hostReleasedBits=static_cast<std::uint16_t>(
        previous_ & ~out.originalButtonBits);
    previous_=out.originalButtonBits;
    return out;
}
} // namespace ae3
