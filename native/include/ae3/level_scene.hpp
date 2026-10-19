#pragma once
#include "ae3/gltf_scene.hpp"
#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace ae3 {
struct LevelPlacement {
    std::string name;
    std::array<float,3> xyz{};
};
struct PlacementFile {
    std::vector<LevelPlacement> entries;
    std::uint32_t skipped=0;
};
// Original AE3 placeholder.plc format from user-supplied Exporter.zip.
// Fixed-size simple records only: rotation/scale and extended record types
// have not been decoded and are intentionally NOT guessed.
PlacementFile parsePlacements(const std::vector<std::uint8_t>& bytes);
PlacementFile parsePlacements(const std::filesystem::path& path);
bool matchesPlacement(const std::string& fileBase,const std::string& entityName);

struct LevelLoadResult {
    StaticScene scene;
    unsigned glbSourceFiles=0,placedInstances=0,unplaced=0,modelFailures=0;
    std::vector<std::string> warnings;
};
// Aggregates private ae3-sdk GLB exports and original simple .plc
// placements into one host-native Direct3D11 scene. No game emulator.
LevelLoadResult loadNativeLevelFolder(const std::filesystem::path& glbFolder,
                                     const std::filesystem::path& plc={},
                                     unsigned maxFiles=300);
} // namespace ae3
