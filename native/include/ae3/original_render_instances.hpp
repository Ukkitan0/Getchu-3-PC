#pragma once
#include "ae3/gltf_scene.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace ae3 {
// Renderer bridge for genuine owner-extracted scene meshes and model samplers.
// No replacement models, scene fabrication, guest GPU or gameplay emulation.
using OriginalRenderId=std::uint64_t;
using HostRenderMatrix=std::array<float,16>; // Column-major, host LH
HostRenderMatrix identityRenderMatrix() noexcept;
HostRenderMatrix gamePlacementMatrix(const std::array<float,3>& originalXYZ);

struct OriginalRenderObject {
    OriginalRenderId id=0;
    std::string sourceName;
    HostRenderMatrix world=identityRenderMatrix();
    bool visible=true;
    int renderLayer=0; // Provided by original-game owner, not inferred.
    std::size_t clip=0;
    double animationSeconds=0;
};
// A separate NativeSkinnedGlb per animated actor can supply this callback.
// The returned source must stay valid until the callback finishes; the
// bridge copies each source immediately, preserving independent actor poses.
using OriginalSceneSampler=std::function<const StaticScene&(std::size_t,double)>;
struct OriginalRenderSubmission {
    OriginalRenderId id=0;
    std::string sourceName;
    std::uint32_t firstVertex=0,vertexCount=0;
    int textureIndex=-1;
    int renderLayer=0;
};
struct OriginalRenderFrame {
    StaticScene scene; // Directly consumable by the existing D3D11 uploader.
    std::vector<OriginalRenderSubmission> submissions;
    std::size_t visibleObjects=0, animatedObjects=0;
};
class OriginalRenderWorld {
public:
    void addStatic(OriginalRenderObject object,std::shared_ptr<const StaticScene> source);
    void addAnimated(OriginalRenderObject object,OriginalSceneSampler sampler);
    bool remove(OriginalRenderId id);
    OriginalRenderObject* find(OriginalRenderId id) noexcept;
    const OriginalRenderObject* find(OriginalRenderId id) const noexcept;
    std::size_t size() const noexcept {return entries_.size();}
    void clear() noexcept {entries_.clear();}
    OriginalRenderFrame composeFrame() const;
private:
    struct Entry {
        OriginalRenderObject object;
        std::shared_ptr<const StaticScene> staticScene;
        OriginalSceneSampler sampler;
    };
    std::vector<Entry> entries_;
};
} // namespace ae3
