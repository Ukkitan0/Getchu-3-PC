#include "ae3/level_scene.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>
int main(int argc,char** argv){
    if(argc!=2&&argc!=3){
        std::cerr<<"Usage: Getchu3LevelSceneProbe <private-converted-GLB-folder> [placeholder.plc]\n";
        return 2;
    }
    try{
        auto level=ae3::loadNativeLevelFolder(
            std::filesystem::u8path(argv[1]),
            argc==3?std::filesystem::u8path(argv[2]):std::filesystem::path{});
        std::cout<<"NATIVE_LEVEL_SCENE_LOADED\n"
                 <<"Original GLB source files: "<<level.glbSourceFiles<<"\n"
                 <<"Placed model instances: "<<level.placedInstances<<"\n"
                 <<"Unplaced model files skipped: "<<level.unplaced<<"\n"
                 <<"Invalid model files: "<<level.modelFailures<<"\n"
                 <<"World triangles: "<<level.scene.triangles.size()/3<<"\n"
                 <<"Materials/draw batches: "<<level.scene.batches.size()<<"\n"
                 <<"Original image resources: "<<level.scene.textures.size()<<"\n"
                 <<"Scene center: "<<level.scene.bounds.center[0]<<","
                                    <<level.scene.bounds.center[1]<<","
                                    <<level.scene.bounds.center[2]<<"\n"
                 <<"Scene radius: "<<level.scene.bounds.radius<<"\n"
                 <<"Native C++ / glTF scene; NO PS2 CPU/IOP/GS runtime.\n";
        for(const auto& warning:level.warnings)
            std::cerr<<"Asset problem: "<<warning<<"\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"NATIVE_LEVEL_SCENE_FAILED: "<<e.what()<<"\n";
        return 1;
    }
}
