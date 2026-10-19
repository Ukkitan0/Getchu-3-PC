#pragma once
#include "ae3/gltf_scene.hpp"
#include <array>
#include <optional>

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
    void tick(const StaticScene& level,float seconds,const WalkInput& input);
    const WalkState& state() const noexcept {return player_;}
    static std::optional<float> groundHeight(
        const StaticScene& level,float x,float z,
        float highestAllowed=1e8f);
private:
    WalkState player_{};
    bool previousJump_=false;
};
} // namespace ae3
