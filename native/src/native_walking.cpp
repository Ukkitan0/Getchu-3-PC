#include "ae3/native_walking.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace ae3 {
namespace {
struct Vec {float x,y,z;};
Vec at(const SceneVertex& v){return {v.x,v.y,v.z};}
Vec subtract(Vec a,Vec b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
Vec cross(Vec a,Vec b){
    return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};
}
float signedAreaXZ(Vec a,Vec b,Vec c){
    return (b.x-a.x)*(c.z-a.z)-(c.x-a.x)*(b.z-a.z);
}
// Cast a vertical ray onto every sufficiently walkable triangle; this is
// host-native geometric collision, NOT a PS2 collision engine interpreter.
// Accurate original I3C collision topology remains to be recovered.
std::optional<float> project(const SceneVertex& av,const SceneVertex& bv,
                              const SceneVertex& cv,float x,float z) {
    Vec a=at(av),b=at(bv),c=at(cv);
    const auto n=cross(subtract(b,a),subtract(c,a));
    const float nlen=std::sqrt(n.x*n.x+n.y*n.y+n.z*n.z);
    if(nlen<1e-6f || std::abs(n.y/nlen)<0.65f)return {};
    if(x<std::min({a.x,b.x,c.x})-.001f ||
       x>std::max({a.x,b.x,c.x})+.001f ||
       z<std::min({a.z,b.z,c.z})-.001f ||
       z>std::max({a.z,b.z,c.z})+.001f)return {};
    const float area=signedAreaXZ(a,b,c);
    if(std::abs(area)<1e-7f)return {};
    const float w0=signedAreaXZ(b,c,{x,0,z})/area;
    const float w1=signedAreaXZ(c,a,{x,0,z})/area;
    const float w2=1.f-w0-w1;
    if(w0<-.0001f||w1<-.0001f||w2<-.0001f)return {};
    return a.y*w0+b.y*w1+c.y*w2;
}
}
std::optional<float> NativeWalkingController::groundHeight(
    const StaticScene& scene,float x,float z,float maxHeight){
    if(!std::isfinite(x)||!std::isfinite(z)||!std::isfinite(maxHeight))
        return {};
    std::optional<float> highest;
    for(std::size_t i=0;i+2<scene.triangles.size();i+=3){
        const auto elevation=project(scene.triangles[i],
                                     scene.triangles[i+1],
                                     scene.triangles[i+2],x,z);
        if(!elevation||*elevation>maxHeight+.0001f)continue;
        if(!highest||*elevation>*highest)highest=*elevation;
    }
    return highest;
}
bool NativeWalkingController::reset(const StaticScene& scene){
    if(scene.triangles.empty())return false;
    float widest=-1.f;
    float spawnX=0,spawnZ=0;
    for(std::size_t i=0;i+2<scene.triangles.size();i+=3){
        const auto& a=scene.triangles[i];
        const auto& b=scene.triangles[i+1];
        const auto& c=scene.triangles[i+2];
        const float area=std::abs((b.x-a.x)*(c.z-a.z)-(c.x-a.x)*(b.z-a.z));
        const auto n=cross(subtract(at(b),at(a)),subtract(at(c),at(a)));
        const float length=std::sqrt(n.x*n.x+n.y*n.y+n.z*n.z);
        if(length<1e-6f||std::abs(n.y/length)<0.65f||area<=widest)continue;
        widest=area;
        spawnX=(a.x+b.x+c.x)/3.f;
        spawnZ=(a.z+b.z+c.z)/3.f;
    }
    if(widest<=0)return false;
    player_={};
    player_.x=spawnX;
    player_.z=spawnZ;
    const auto ground=groundHeight(scene,spawnX,spawnZ);
    if(!ground)return false;
    player_.y=*ground;
    player_.grounded=true;
    previousJump_=false;
    return true;
}
void NativeWalkingController::tick(const StaticScene& scene,float dt,
                                    const WalkInput& input){
    if(!std::isfinite(dt)||dt<0||dt>0.1f)
        throw std::runtime_error("native frame delta outside walk integration bounds");
    if(!std::isfinite(input.facingRadians) || !std::isfinite(input.forward)||
       !std::isfinite(input.strafe))
        throw std::runtime_error("native walking input contains invalid values");
    player_.elapsedSeconds+=dt;
    constexpr float gravity=24.f,jumpSpeed=7.5f;
    if(player_.grounded && input.jump && !previousJump_){
        player_.grounded=false;
        player_.verticalVelocity=jumpSpeed;
    }
    previousJump_=input.jump;
    const float localZ=std::clamp(input.forward,-1.f,1.f);
    const float localX=std::clamp(input.strafe,-1.f,1.f);
    const float magnitude=std::sqrt(localX*localX+localZ*localZ);
    const float normalizer=magnitude>1.f?1.f/magnitude:1.f;
    const float speed=input.sprint?8.25f:4.5f;
    const float forwardX=std::sin(input.facingRadians);
    const float forwardZ=std::cos(input.facingRadians);
    const float dx=(localZ*forwardX+localX*forwardZ)*normalizer*speed*dt;
    const float dz=(localZ*forwardZ-localX*forwardX)*normalizer*speed*dt;
    float nextX=player_.x+dx,nextZ=player_.z+dz;
    constexpr float maxStep=0.5f;
    const auto nextFloor=groundHeight(scene,nextX,nextZ,player_.y+maxStep);
    if(player_.grounded && nextFloor && *nextFloor>player_.y+maxStep){
        nextX=player_.x;nextZ=player_.z;
    }
    player_.x=nextX;player_.z=nextZ;
    player_.verticalVelocity-=gravity*dt;
    player_.y+=player_.verticalVelocity*dt;
    const auto floor=groundHeight(scene,player_.x,player_.z,
                                  player_.y+std::max(maxStep,gravity*dt*dt));
    if(floor && player_.verticalVelocity<=0 && player_.y<=*floor+0.12f){
        player_.y=*floor;
        player_.verticalVelocity=0;
        player_.grounded=true;
    }else player_.grounded=false;
}
} // namespace ae3
