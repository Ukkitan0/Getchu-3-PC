#include "ae3/native_walking.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
namespace {
void require(bool pass,const char* message){if(!pass)throw std::runtime_error(message);}
ae3::StaticScene floorScene(){
    ae3::StaticScene scene;
    auto point=[](float x,float y,float z)->ae3::SceneVertex {
        return {x,y,z,1,1,1,1,0,0};
    };
    scene.triangles={
        point(-10,0,-10),point(10,0,-10),point(10,0,10),
        point(-10,0,-10),point(10,0,10),point(-10,0,10)
    };
    scene.sourcePrimitives=1;
    return scene;
}
}
int main(){
    try{
        auto level=floorScene();
        auto ground=ae3::NativeWalkingController::groundHeight(level,0,0);
        require(ground && std::abs(*ground)<1e-6f,
                "no real triangle-ground contact at stage center");
        require(!ae3::NativeWalkingController::groundHeight(level,100,100),
                "native collision gave a floor outside imported geometry");
        ae3::NativeWalkingController player;
        require(player.reset(level),"cannot initialize native player on original-shape geometry");
        require(player.state().grounded,"native spawn should be on walkable ground");
        const float startingZ=player.state().z;
        for(int i=0;i<20;++i)
            player.tick(level,.05f,{1.f,0.f,0.f,false,false});
        require(player.state().z>startingZ+3.5f,
                "native WASD player did not move across mesh floor");
        require(player.state().grounded,"walking on geometry immediately fell through it");
        require(std::abs(player.state().y)<.02f,
                "grounded native height diverged from triangle geometry");
        const float foot=player.state().y;
        player.tick(level,.016f,{0,0,0,true,false});
        require(!player.state().grounded && player.state().y>foot,
                "native player jump failed to leave ground");
        for(int i=0;i<110;++i)
            player.tick(level,.016f,{0,0,0,false,false});
        require(player.state().grounded,
                "native gravity did not return player to original mesh floor");
        require(std::abs(player.state().y)<.02f,"native landing penetrated original geometry");
        require(player.state().elapsedSeconds>2,
                "simulation elapsed time did not advance");
        bool rejected=false;
        try{player.tick(level,-.1f,{ });}
        catch(const std::runtime_error&){rejected=true;}
        require(rejected,"invalid host frame time accepted");
        require(!ae3::NativeWalkingController{}.reset(ae3::StaticScene{}),
                "empty scene incorrectly created a playable character");
        std::cout<<"NATIVE_MOVEMENT_TEST_PASSED mesh_floor=1"
                    " gravity=1 jump=1 wasd=1 landing=1 no_console_physics=1\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"NATIVE_MOVEMENT_TEST_FAILED: "<<e.what()<<"\n";
        return 1;
    }
}
