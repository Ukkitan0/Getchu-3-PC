#pragma once
#include <cstdint>
#include <vector>
#include <stdexcept>
namespace ae3 {
struct Texture {
    unsigned width = 0;
    unsigned height = 0;
    // True host-side RGBA8888 pixels for Windows Direct3D11.
    // This is asset format decoding, NOT PS2 graphics hardware emulation.
    std::vector<std::uint8_t> rgba;
};
bool isTim2(const std::vector<std::uint8_t>& bytes);
Texture decodeTim2(const std::vector<std::uint8_t>& bytes, unsigned picture = 0);
}
