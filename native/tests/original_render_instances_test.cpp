#include "ae3/original_render_instances.hpp"
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace {
void expect(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
std::shared_ptr<ae3::StaticScene> model(){
    auto scene=std::make_shared<ae3::StaticScene>();
    // Pure structural test fixture. No replacement original-game model.
    scene->triangles={{0,0,0,1,1,1,1,0,0},{1,0,0,1,1,1,1,1,0},{0,1,0,1,1,1,1,0,1}};
    scene->textures.push_back({{1,2,3}});
    scene->batches={{0,3,0}};
    scene->sourceNodes=1;scene->sourcePrimitives=1;
    return scene;
}
void run(){
    const auto shared=model();
    ae3::OriginalRenderWorld world;
    ae3::OriginalRenderObject a{};
    a.id=42;a.sourceName="owner_original_mesh";
    a.world=ae3::gamePlacementMatrix({10,2,3});
    world.addStatic(a,shared);
    auto b=a;b.id=43;b.world=ae3::gamePlacementMatrix({-4,0,-2});
    world.addStatic(b,shared);
    auto frame=world.composeFrame();
    expect(frame.scene.triangles.size()==6,"two independent original model instances required");
    expect(frame.scene.textures.size()==1,"shared source texture unnecessarily duplicated");
    expect(frame.scene.batches.size()==2&&frame.submissions[0].id==42&&
           frame.submissions[1].id==43,"material/identity/order lost");
    expect(frame.scene.triangles[0].x==10&&frame.scene.triangles[0].z==-3,
           "original->native coordinate conversion incorrect");
    expect(frame.scene.triangles[3].x==-4&&frame.scene.triangles[3].z==2,
           "second instance inherits first actor transform");
    expect(frame.scene.bounds.max[0]==11&&frame.scene.bounds.min[0]==-4,
           "scene bounds not recomputed");
    a.id=44;a.renderLayer=-10;
    std::size_t calls=0,clip=0;double seconds=0;
    world.addAnimated(a,[&](std::size_t c,double s)->const ae3::StaticScene&{
        ++calls;clip=c;seconds=s;
        return *shared;
    });
    auto* pose=world.find(44);
    expect(pose!=nullptr,"animated identity missing");
    pose->clip=3;pose->animationSeconds=0.75;
    frame=world.composeFrame();
    expect(calls==1&&clip==3&&seconds==0.75,"per-actor clip/time not forwarded");
    expect(frame.submissions.front().id==44&&frame.animatedObjects==1,
           "animated draw layer/source identity lost");
    expect(frame.scene.textures.size()==1,"animated/static source texture duplicated");
    world.find(42)->visible=false;
    frame=world.composeFrame();
    expect(frame.visibleObjects==2&&frame.scene.triangles.size()==6,
           "game-supplied actor visibility ignored");
    expect(world.remove(43)&&!world.remove(43),"remove/identity state wrong");
    bool rejected=false;
    try{world.addStatic(a,shared);}catch(const std::invalid_argument&){rejected=true;}
    expect(rejected,"duplicate identity accepted");
    auto bad=ae3::identityRenderMatrix();bad[12]=INFINITY;
    a.id=55;a.world=bad;rejected=false;
    try{world.addStatic(a,shared);}catch(const std::invalid_argument&){rejected=true;}
    expect(rejected,"non-finite world transform accepted");
    auto corrupt=model();corrupt->batches[0].textureIndex=22;
    a.world=ae3::identityRenderMatrix();rejected=false;
    try{world.addStatic(a,corrupt);}catch(const std::runtime_error&){rejected=true;}
    expect(rejected,"invalid material texture index accepted");
    world.clear();expect(world.composeFrame().scene.triangles.empty(),"empty frame invalid");
}
}
int main(){
    try{
        run();
        std::cout<<"ORIGINAL_RENDER_INSTANCES_PASS: instancing texture_dedup actor_identity animation visibility transforms layers validation\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"ORIGINAL_RENDER_INSTANCES_FAIL: "<<e.what()<<"\n";
        return 1;
    }
}
