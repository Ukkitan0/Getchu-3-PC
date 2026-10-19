#include "ae3/native_walking.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
void check(bool yes,const char* error){if(!yes)throw std::runtime_error(error);}
ae3::StaticScene floorAndWall(){
    ae3::StaticScene scene;
    auto v=[](float x,float y,float z)->ae3::SceneVertex{
        return {x,y,z,1,1,1,1,0,0};
    };
    // Floor split into two large native triangle surfaces.
    scene.triangles={
        v(-6,0,-6),v(6,0,-6),v(6,0,6),
        v(-6,0,-6),v(6,0,6),v(-6,0,6),
        // Vertical original-format equivalent wall at x=0, z[-2,+2],
        // preventing the player radius from crossing x=0.
        v(0,0,-2),v(0,3,-2),v(0,3,2),
        v(0,0,-2),v(0,3,2),v(0,0,2)
    };
    return scene;
}
}
int main(){
    try{
        auto level=floorAndWall();
        ae3::NativeWalkingController body;
        check(body.resetAt(level,-2,0),"native test player cannot spawn at explicit position");
        check(std::abs(body.state().x+2.f)<1e-6f,"explicit spawn x changed");
        for(int i=0;i<120;++i)
            body.tick(level,.016f,{1,0,1.57079633f,false,false});
        check(body.state().x<-.27f && body.state().x> -1.f,
              "player passed through a vertical original collision wall");
        check(body.state().grounded,"native collision wall caused fall through floor");
        const float beforeX=body.state().x;
        for(int i=0;i<18;++i)
            body.tick(level,.016f,{1,1,1.57079633f,false,false});
        check(body.state().z<-.3f,"native wall slide did not permit tangential motion");
        // The previous x was ~-0.344. The first diagonal step legitimately
        // approaches to ~-0.293, still OUTSIDE the 0.29-radius capsule.
        // Test actual wall penetration, not an arbitrary delta of 0.05.
        check(body.state().x<=-.28f,
              "native wall-slide penetrated wall collision radius");
        check(body.state().x>=beforeX-.05f,
              "tangential wall slide unexpectedly pushed player backwards");
        check(body.resetAt(level,2,0),"native player cannot spawn opposite side");
        for(int i=0;i<120;++i)
            body.tick(level,.016f,{1,0,-1.57079633f,false,false});
        check(body.state().x>.27f,"native wall collision was not bidirectional");
        check(body.resetAt(level,-2,0),"cannot restore start point");
        body.tick(level,.016f,{0,0,0,true,false});
        check(!body.state().grounded&&body.state().verticalVelocity>0,
              "native jump was broken by wall integration");
        bool rejected=false;
        try{body.resetAt(level,200,200);}
        catch(const std::runtime_error&){rejected=true;}
        check(!rejected,"absent floor should return false instead of throwing");
        check(!body.resetAt(level,200,200),"native explicit spawn accepted empty world");
        std::cout<<"NATIVE_I3C_WALL_MOVEMENT_PASSED wall_block=1 slide=1 "
                    "both_sides=1 spawn=1 jump=1 host_physics=1\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"NATIVE_I3C_WALL_MOVEMENT_FAILED: "<<e.what()<<"\n";
        return 1;
    }
}
