#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace ae3 {
// Literal key/default pairs at original SCUS_975.01 0x6D64B0..0x6D6510.
// Source initializer 0x1BB9A8..0x1BBB14 calls 0x35AB88 and 0x2BB3BC
// once per pair, then 0x2BB1D0 for the final scalar.
struct OriginalAudioDescriptorField {
    std::string_view sourceName;
    std::string_view sourceDefault;
    std::size_t originalSlotOffset;
};
inline constexpr std::array<OriginalAudioDescriptorField,5>
    OriginalAudioDescriptorFields_1bb9a8{{
        {"name", "noname", 0x00},
        {"midi", "M01.mid", 0x20},
        {"reverb", "system", 0x40},
        {"vb", "M01.bd", 0x60},
        {"vh", "M01.hd", 0x80}
    }};
inline constexpr std::string_view OriginalAudioVolumeKey_1bbaf4="volume_scale";
inline constexpr float OriginalAudioVolumeDefault_1bbaf4=1.0f;
inline constexpr std::size_t OriginalAudioDescriptorStride_1bb458=0xB0;
inline constexpr std::size_t OriginalAudioVolumeFieldOffset_1bbafc=0xA0;

// Original source operations 0x2BB3BC and 0x2BB1D0 depend on a
// serialized attribute/object system still under recovery. The required
// callback exposes their requested lookup, without fabricating a
// replacement parser or pretending that native std::string is a 32-byte
// retail string object.
class OriginalAudioDescriptorSource {
public:
    virtual ~OriginalAudioDescriptorSource()=default;
    virtual std::string originalStringField_2bb3bc(
        std::string_view originalKey,std::string_view originalFallback)=0;
    virtual float originalFloatField_2bb1d0(
        std::string_view originalKey,float originalFallback)=0;
};
struct OriginalAudioDescriptorValues {
    std::array<std::string,5> sourceStrings{};
    float volumeScale=1.0f;
    std::string_view value(std::size_t slot) const {return sourceStrings.at(slot);}
};
// Source-level reproduction of the exact order and default inputs of
// original 0x1BB9A8; neither a new sound subsystem nor PS2 emulation.
inline OriginalAudioDescriptorValues originalAudioDescriptorFields_1bb9a8(
    OriginalAudioDescriptorSource& originalSource) {
    OriginalAudioDescriptorValues result;
    for(std::size_t i=0;i<OriginalAudioDescriptorFields_1bb9a8.size();++i){
        const auto& field=OriginalAudioDescriptorFields_1bb9a8[i];
        result.sourceStrings[i]=originalSource.originalStringField_2bb3bc(
            field.sourceName,field.sourceDefault);
    }
    result.volumeScale=originalSource.originalFloatField_2bb1d0(
        OriginalAudioVolumeKey_1bbaf4,OriginalAudioVolumeDefault_1bbaf4);
    return result;
}
} // namespace ae3
