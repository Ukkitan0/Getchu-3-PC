#pragma once
#include "ae3/gltf_scene.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace ae3 {
// Host-native animation of original I3M motion already converted OFFLINE
// by MIT ae3-sdk into named glTF skeleton joints and animation channels.
// This parses glTF 2.0, not PlayStation machine instructions or VU programs.
// CPU skinning produces the same SceneVertex layout as our D3D11 renderer.
class NativeSkinnedGlb {
public:
    explicit NativeSkinnedGlb(const std::filesystem::path& glb);
    NativeSkinnedGlb(const std::vector<std::uint8_t>& glbBytes);
    std::size_t clipCount() const noexcept {return clips_.size();}
    std::size_t boneCount() const noexcept {return totalJointCount_;}
    std::size_t skinnedVertexCount() const noexcept {return skinnedVertexCount_;}
    const std::vector<std::string>& clipNames() const noexcept {return clipNames_;}
    double clipDuration(std::size_t index) const;
    const StaticScene& restScene() const noexcept {return scene_;}
    // Samples animation at absolute seconds, looping the selected clip.
    // Validates all channels, joint arrays and index ranges at load time.
    // Only LINEAR / STEP glTF channels currently supported; unsupported
    // CUBICSPLINE is explicitly rejected rather than silently misplayed.
    const StaticScene& sample(std::size_t clip,double seconds);
private:
    using V3=std::array<float,3>;
    using V4=std::array<float,4>;
    using Mat=std::array<float,16>;
    struct Node {
        int parent=-1;
        V3 position{0,0,0},scale{1,1,1};
        V4 rotation{0,0,0,1};
        Mat explicitMatrix{};
        bool hasMatrix=false;
    };
    struct Skin {
        std::vector<int> nodes;
        std::vector<Mat> inverseBind;
    };
    struct Binding {
        std::uint32_t vertex=0;
        V3 local{};
        int node=-1,skin=-1;
        std::array<unsigned,4> joint{};
        std::array<float,4> weights{};
    };
    enum class Path {position,rotation,scale};
    struct Channel {
        int node=-1;
        Path path=Path::position;
        bool step=false;
        std::vector<float> times;
        std::vector<V4> values;
    };
    struct Clip {
        double duration=0;
        std::vector<Channel> channels;
    };
    void load(const std::vector<std::uint8_t>& glbBytes);
    StaticScene scene_;
    std::vector<Node> nodes_;
    std::vector<Skin> skins_;
    std::vector<Binding> bindings_;
    std::vector<Clip> clips_;
    std::vector<std::string> clipNames_;
    std::size_t totalJointCount_=0,skinnedVertexCount_=0;
};
} // namespace ae3
