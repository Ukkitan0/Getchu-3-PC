#pragma once
#include "ae3/lua_stage.hpp"
#include "ae3/native_skinning.hpp"
#include <array>
#include <cstddef>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ae3 {
// ORIGINAL Lua-created entity descriptors become native animated 3D objects.
// This is deliberately a strict, opt-in asset binding: matching existing
// owner-extracted GLB filenames or local private binding table, never
// guessing names, inventing Sony assets or interpreting PS2 instructions.
struct NativeActor {
    std::string entityClass, name, modelName, tagName;
    std::string positionSource; // explicit Lua, exact original PLC tag, unresolved origin
    std::array<float,3> position{0,0,0}; // host-native LH world coordinates
    float yaw=0; // radians
    std::size_t firstVertex=0,vertexCount=0;
    std::size_t activeClip=0;
    double elapsed=0;
    bool hasAnimation=false;
    std::unique_ptr<NativeSkinnedGlb> model;
};
struct ActorBindingReport {
    std::size_t stageEntities=0,actorsCreated=0,animatedActors=0;
    std::size_t unboundEntities=0,modelsRejected=0;
    std::size_t matchedOriginalPlcTags=0,unresolvedSpawnPositions=0;
    std::vector<std::string> diagnostics;
};
// One combined D3D11 buffer: stationary stage geometry plus actor geometry.
// At runtime we update vertex positions via CPU skinning and native transforms.
// The original Lua simulation stays in Lua; game engine behavior is C++.
class NativeActorWorld {
public:
    explicit NativeActorWorld(StaticScene backdrop={});
    void bind(const NativeStageWorld& stage,
              const std::filesystem::path& privateGlbDirectory,
              const std::filesystem::path& optionalBindingFile={});
    void tick(double seconds);
    bool moveActor(const std::string& uniqueName,
                   float x,float y,float z,float facingRadians);
    bool setActorClip(const std::string& uniqueName,std::size_t clip);
    const NativeActor* actor(const std::string& name) const;
    const std::vector<NativeActor>& actors() const noexcept {return actors_;}
    const StaticScene& scene() const noexcept {return combined_;}
    const ActorBindingReport& report() const noexcept {return report_;}
private:
    void updateGeometry(NativeActor& actor);
    void rebuildLayout();
    StaticScene backdrop_,combined_;
    std::vector<NativeActor> actors_;
    ActorBindingReport report_;
};
} // namespace ae3
