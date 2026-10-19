#pragma once
#include <array>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace ae3 {
// Original I3D geometry exported to Khronos GLB by optional MIT ae3-sdk.
// This is a *native mesh source*, not PS2 GPU packet interpretation.
// All geometry is transformed into host-native LH triangles in world space.
struct SceneVertex {
    float x=0,y=0,z=0;
    float r=1,g=1,b=1,a=1;
    float u=0,v=0;
};
static_assert(sizeof(SceneVertex)==9*sizeof(float));
struct SceneTexture {std::vector<std::uint8_t> encoded;};
struct SceneBatch {std::uint32_t firstVertex=0, vertexCount=0; int textureIndex=-1;};
struct SceneBounds {
    std::array<float,3> min{},max{};
    std::array<float,3> center{};
    float radius=0;
};
struct StaticScene {
    std::vector<SceneVertex> triangles;
    std::vector<SceneBatch> batches;
    std::vector<SceneTexture> textures;
    SceneBounds bounds{};
    std::uint32_t sourceNodes=0,sourcePrimitives=0,skinnedPrimitives=0;
    std::uint32_t unsupportedPrimitives=0,missingTextures=0;
};
// Supports embedded GLB 2.0 triangle primitives. Node world transforms,
// vertex colors and material base factors are baked into native vertices.
// Static rest-pose meshes only: skinned animations and textures remain TODO.
// Rejects external URIs so loading an untrusted GLB cannot read arbitrary files.
StaticScene loadStaticGlb(const std::filesystem::path& path);
StaticScene loadStaticGlbBytes(const std::vector<std::uint8_t>& bytes);
} // namespace ae3
