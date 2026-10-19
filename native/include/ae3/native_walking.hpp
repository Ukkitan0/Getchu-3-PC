#pragma once
#include "ae3/gltf_scene.hpp"
#include <array>
#include <optional>
#include <cstddef>
#include <vector>

namespace ae3 {
// First host-native walk/jump experiment on original I3D scene geometry.
// Temporary physics and controls, NOT a recovered AE3 player controller.
// Runs on Windows x64; does not execute PS2 EE, IOP, GS or physics code.
struct WalkInput {
    float forward=0;
    float strafe=0;
    float facingRadians=0;
    bool jump=false;
    bool sprint=false;
};
struct WalkState {
    float x=0,y=0,z=0; // y is player feet
    float verticalVelocity=0;
    bool grounded=false;
    double elapsedSeconds=0;
};
class NativeWalkingController {
public:
    bool reset(const StaticScene& level);
    // Place native test player on a specific existing level collision
    // surface; useful when a recovered Lua stage supplies a spawn marker.
    bool resetAt(const StaticScene& level,float x,float z,float maxHeight=1e8f);
    void tick(const StaticScene& level,float seconds,const WalkInput& input);
    const WalkState& state() const noexcept {return player_;}
    static std::optional<float> groundHeight(
        const StaticScene& level,float x,float z,
        float highestAllowed=1e8f);
private:
    WalkState player_{};
    // Pre-filtered original I3C and GLB steep surfaces that can block
    // a finite-radius capsule, rather than sampling only ground height.
    std::vector<std::size_t> wallTriangles_;
    void prepareWalls(const StaticScene& level);
    bool collidesWithWall(const StaticScene& level,float x,float z,float feet) const;
    bool previousJump_=false;
};
} // namespace ae3
