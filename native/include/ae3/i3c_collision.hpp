#pragma once
#include "ae3/gltf_scene.hpp"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace ae3 {
// Native decoding of ORIGINAL Ape Escape 3 I3D_I3C collision meshes.
// The triangle data is converted once to native CPU geometry. At runtime
// there is NO PlayStation CPU, IOP, VU, GS, DMA or collision engine interpreter.
struct OriginalCollision {
    StaticScene geometry;
    std::vector<std::string> groupNames;
    std::vector<std::string> materialNames;
    std::size_t aabbs = 0, bvhNodes = 0, originalVertices = 0;
};
OriginalCollision parseOriginalI3c(const std::vector<std::uint8_t>& bytes);
OriginalCollision parseOriginalI3c(const std::filesystem::path& source);

// Load genuine I3D_I3C members inside a level extraction tree. Files may
// have an .i3c extension or an .i3d extension; always verify the full tag.
// Does not infer collision material from filename.
OriginalCollision loadOriginalI3cDirectory(
    const std::filesystem::path& folder, std::size_t maxFiles=100);
} // namespace ae3
