#include "ae3/original_render_instances.hpp"
#include "ae3/original_render_owner_schedule.hpp"
#include "ae3/original_render_pass.hpp"
#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

namespace {
void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
std::shared_ptr<ae3::StaticScene> testGeometry(){
    // Game-free mathematical fixture only. Production sources MUST be
    // owner-extracted original stage I3D mesh and original I3M clip data.
    auto p=std::make_shared<ae3::StaticScene>();
    p->triangles={{0,0,0,0.5f,0.7f,0.8f,1,0,0},
                  {1,0,0,0.5f,0.7f,0.8f,1,1,0},
                  {0,1,0,0.5f,0.7f,0.8f,1,0,1}};
    p->batches={{0,3,0}};
    p->textures.push_back({{0x42,0x4d,0x00}});
    p->sourceNodes=1;p->sourcePrimitives=1;
    return p;
}
void run(){
    auto source=testGeometry();
    ae3::OriginalRenderWorld world;
    ae3::OriginalRenderObject first{};
    first.id=101;first.sourceName="private_original_stadium_asset";
    first.world=ae3::gamePlacementMatrix({1,2,3});
    world.addStatic(first,source);
    ae3::OriginalRenderObject actor{};
    actor.id=102;actor.sourceName="private_original_actor_clip";
    actor.world=ae3::gamePlacementMatrix({5,0,-1});
    std::size_t sampleCalls=0;
    world.addAnimated(actor,[&](std::size_t clip,double time)->const ae3::StaticScene&{
        check(clip==3&&time==0.75,"actor animation timeline not preserved");
        ++sampleCalls;
        return *source;
    });
    auto* current=world.find(102);
    check(current,"original actor 102 was not registered");
    current->clip=3;current->animationSeconds=.75;
    ae3::OriginalRetailRenderOwner owner{};
    owner.frame40=UINT64_MAX;
    owner.schedulingMode38=1;owner.schedulingPeriod3c=2;
    owner.slots={{101,1,1},{102,1,1}};
    ae3::OriginalRetailDrawObject stage{};
    stage.id=101;stage.groupMask20=1;stage.flag30=1;stage.flag35=1;
    ae3::OriginalRetailDrawObject character=stage;
    character.id=102;character.flag32=1;character.flag33=1;
    const std::vector<ae3::OriginalRetailDrawObject> actors{stage,character};
    unsigned culled=0;
    ae3::OriginalRetailDrawProviders providers{
        [&](ae3::OriginalRenderId id){check(id==101||id==102,"unexpected spatial-cull actor");++culled;return true;},
        {}
    };
    const auto next=[&]()->ae3::OriginalRenderFrame{
        const auto tick=ae3::advanceOriginalRetailRenderOwner(owner,[](ae3::OriginalRetailRenderSlot& slot){
            check(slot.active05==1,"source test validator unexpectedly disabled slot");
        });
        std::vector<ae3::OriginalRetailDrawObject> selected;
        for(const auto id:tick.scheduledIds){
            for(const auto& a:actors)if(a.id==id)selected.push_back(a);
        }
        const auto pass=ae3::traceOriginalPrimaryRenderPass(selected,1,providers);
        check(pass.acceptedIds==tick.scheduledIds,"source admission changed retail owner order");
        return world.composeFrameInOriginalOrder(pass.acceptedIds);
    };
    auto frame=next();
    check(owner.frame40==0&&frame.visibleObjects==1&&frame.submissions[0].id==101,
          "original owner frame 0 must draw source-mesh 101 only");
    check(frame.scene.triangles[0].x==1&&frame.scene.triangles[0].z==-3,
          "original coordinate placement lost in D3D frame");
    check(frame.scene.triangles[0].r==0.5f&&frame.scene.textures.size()==1,
          "original asset colors or material references changed");
    frame=next();
    check(owner.frame40==1&&frame.visibleObjects==1&&frame.animatedObjects==1&&
          frame.submissions[0].id==102,"original owner frame 1 must select actor 102");
    check(sampleCalls==1&&culled==1,"original animation/cull callbacks skipped");
    check(frame.scene.triangles[0].x==5&&frame.scene.triangles[0].z==1,
          "original animated model placement lost");
    auto pass=ae3::traceOriginalSecondaryRenderPass({stage,character},providers);
    check(pass.acceptedIds==std::vector<ae3::OriginalRenderId>({101,102}),
          "retail secondary pass should independently admit both actors");
    check(culled==3,"secondary pass must always spatial-check both objects");
    frame=world.composeFrameInOriginalOrder(pass.acceptedIds);
    check(frame.visibleObjects==2&&frame.scene.triangles.size()==6&&
          frame.scene.batches.size()==2&&frame.scene.textures.size()==1&&
          frame.animatedObjects==1,"genuine per-object drawing bridge corrupted mesh topology or materials");
    check(frame.submissions[0].id==101&&frame.submissions[1].id==102,
          "original two-pass actor order not preserved");
}
}
int main(){
    try{run();std::cout<<"ORIGINAL_RENDER_PIPELINE_PASS: owner_schedule retail_two_pass visibility original_order meshes animated_actor material_resource\n";return 0;}
    catch(const std::exception& e){std::cerr<<"ORIGINAL_RENDER_PIPELINE_FAIL: "<<e.what()<<"\n";return 1;}
}
