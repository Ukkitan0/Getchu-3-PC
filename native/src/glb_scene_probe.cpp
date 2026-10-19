#include "ae3/gltf_scene.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc,char** argv){
    if(argc!=2){
        std::cerr<<"Usage: Getchu3GlbSceneProbe <locally-converted-I3D.glb>\n";
        return 2;
    }
    try{
        const auto scene=ae3::loadStaticGlb(std::filesystem::u8path(argv[1]));
        std::uint64_t compressedBytes=0;
        for(const auto& tex:scene.textures)compressedBytes+=tex.encoded.size();
        std::cout<<"NATIVE_GLB_LOADED (no PlayStation graphics interpreter)\n"
                 <<"Geometry nodes: "<<scene.sourceNodes<<"\n"
                 <<"Triangle primitives: "<<scene.sourcePrimitives<<"\n"
                 <<"Triangles: "<<scene.triangles.size()/3<<"\n"
                 <<"Material draw batches: "<<scene.batches.size()<<"\n"
                 <<"PNG/JPEG resources: "<<scene.textures.size()<<"\n"
                 <<"Original texture resource bytes: "<<compressedBytes<<"\n"
                 <<"Missing or rejected texture paths: "<<scene.missingTextures<<"\n"
                 <<"Skinning-required primitives (rest pose only): "
                    <<scene.skinnedPrimitives<<"\n"
                 <<"Unsupported primitive topology: "<<scene.unsupportedPrimitives<<"\n"
                 <<"Center: "<<scene.bounds.center[0]<<","
                               <<scene.bounds.center[1]<<","
                               <<scene.bounds.center[2]<<"\n"
                 <<"Bounding radius: "<<scene.bounds.radius<<"\n"
                 <<"Status: native mesh data decoded; game physics/AI not included\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"NATIVE_GLB_SCENE_FAILED: "<<e.what()<<"\n";
        return 1;
    }
}
