#pragma once
#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace ae3 {
struct RotationKey {
    std::uint16_t frame=0,curve=0;
    float seconds=0;
    std::array<float,4> xyzw{0,0,0,1};
};
struct BoneRotationTrack {
    std::string name;
    std::uint16_t type=0;
    std::vector<RotationKey> keys;
};
struct OriginalAnimation {
    float duration=0;
    std::vector<BoneRotationTrack> tracks;
};
// Original PS2 asset data converted to native animation keyframes at BUILD
// or asset-load time. This is not guest PS2 CPU/VU/IOP execution.
OriginalAnimation parseI3m(const std::vector<std::uint8_t>& source);
OriginalAnimation parseI3m(const std::filesystem::path& localOwnedFile);
// Interpolates an original quantized quaternion track on the native host
// using spherical shortest-arc interpolation. Does not guess the game bone
// mapping (not yet recovered).
std::array<float,4> sampleTrack(const BoneRotationTrack& track,float gameSeconds);
}
