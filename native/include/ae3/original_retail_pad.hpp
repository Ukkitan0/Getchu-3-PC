#pragma once
#include "ae3/original_pad_runtime.hpp"
#include <array>
#include <cstdint>

namespace ae3 {
// USA SCUS_975.01 @ 0x5179B0 loads 16 query codes from 0x6ED0C8.
// These are original PAD-LIB CHANNEL CODES, not DS2 on-wire bit offsets.
inline constexpr std::array<std::uint32_t,16> OriginalRetailButtonChannels_6ed0c8{
    0,3,4,6,7,5,12,14,15,13,10,8,1,11,9,2
};

// Original 0x5179B0 requests four axes in the order RX, RY, LX, LY.
// These are source channel numbers; a host backend implements its lookup.
inline constexpr std::array<std::uint32_t,4> OriginalRetailAxisChannels_5179b0{
    0x12,0x13,0x10,0x11
};

struct OriginalRetailAnalogVector {float x=0,y=0;};

// Source 0x517788: centered-byte conversion, source 1/120 scale, radial
// deadzone 0.375, radial rescale, and componentwise [-1,1] clamp.
OriginalRetailAnalogVector originalRetailAnalog_517788(
    std::uint8_t x,std::uint8_t y) noexcept;

// Original game calls the pad library function @ 0x4059E8 repeatedly.
// This interface preserves its *channel selection* without emulating a
// PS2 pad library or fabricating game-specific physical button bindings.
class OriginalRetailPadChannelSource {
public:
    virtual ~OriginalRetailPadChannelSource()=default;
    virtual int queryOriginalPadChannel_4059e8(std::uint32_t channel)=0;
};

// Host adapter: the original library's 0..15 channel numbers use the
// PlayStation digital button bit order; 0x10..0x13 are LX, LY, RX, RY.
// On-wire PS2 active-low buttons were already decoded by the host
// observation into positive pressedMask; no PS2 CPU/IOP emulation.
class OriginalHostPadChannels final : public OriginalRetailPadChannelSource {
public:
    explicit OriginalHostPadChannels(OriginalPadObservation snapshot) noexcept
        : snapshot_(snapshot) {}
    int queryOriginalPadChannel_4059e8(std::uint32_t channel) override;
private:
    OriginalPadObservation snapshot_;
};

// Retail 0x517880..0x517894 direct six-byte source packet:
// bytes 0..1: little-endian positive pressed bits; bytes 2..5: RX RY LX LY.
// This is the already-decoded direct-data pathway, NOT a PS2 SIO packet.
struct OriginalRetailDirectPacket {
    std::array<std::uint8_t,6> bytes{0,0,128,128,128,128};
};

struct OriginalRetailPadSample {
    bool connected=false;
    std::uint16_t originalButtonBits=0; // source 0x5179B0 bit position i
    std::uint16_t originalNewPressBits=0; // 0x517898: new & ~previous
    std::uint16_t hostReleasedBits=0; // host convenience, NOT a retail field
    std::array<std::uint8_t,4> originalAxisBytes{128,128,128,128}; // RX RY LX LY
    OriginalRetailAnalogVector right{};
    OriginalRetailAnalogVector left{};
};

// Source game tick @ 0x26EB34 writes two 0x18-byte records at
// 0x6496F8 (port 0) and 0x649710 (port 1). Each record is six u32 slots:
// held buttons, newly pressed buttons, RX, RY, LX, LY float values.
struct OriginalRetailGamePadState_26eb34 {
    std::uint32_t originalButtonBits=0;
    std::uint32_t originalNewPressBits=0;
    float rightX=0,rightY=0,leftX=0,leftY=0;
};
static_assert(sizeof(OriginalRetailGamePadState_26eb34)==0x18);
OriginalRetailGamePadState_26eb34 originalRetailGamePadState_26eb34(
    const OriginalRetailPadSample& source) noexcept;

class OriginalRetailPadReader {
public:
    // One native game input tick. Connection status is provided by the
    // original device state resolver; no guessed timeout or autoconnect.
    OriginalRetailPadSample sample(OriginalRetailPadChannelSource& source,
                                  bool connected);
    // Retail 0x517878 branch with non-null source pointer. No PAD-library
    // calls; consumes exactly the six original source bytes. Null means
    // disconnected/neutral, using the source pad state reset path.
    OriginalRetailPadSample sampleDirect(const OriginalRetailDirectPacket* packet);
    void reset() noexcept {previous_=0;}
private:
    OriginalRetailPadSample finish(OriginalRetailPadSample current) noexcept;
    std::uint16_t previous_=0;
};
// 0x26EB34 updates two ports independently before copying each six-word
// record to the game input globals 0x6496F8 and 0x649710.
struct OriginalRetailGameFrame_26eb34 {
    std::array<OriginalRetailPadSample,2> port{};
    std::array<OriginalRetailGamePadState_26eb34,2> originalGameRecords{};
};
class OriginalRetailTwoPortReader_26eb34 {
public:
    OriginalRetailGameFrame_26eb34 sampleDirect(
        const std::array<const OriginalRetailDirectPacket*,2>& packets);
    void reset() noexcept {for(auto& p:ports_)p.reset();}
private:
    std::array<OriginalRetailPadReader,2> ports_{};
};
} // namespace ae3
