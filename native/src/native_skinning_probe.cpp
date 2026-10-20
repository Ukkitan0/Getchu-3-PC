#include "ae3/native_skinning.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <stdexcept>
int main(int argc,char** argv){
    if(argc!=2){
        std::cerr<<"Usage: Getchu3SkinningProbe <private-model-with-I3M-tracks.glb>\n";
        return 2;
    }
    try {
        ae3::NativeSkinnedGlb model(std::filesystem::u8path(argv[1]));
        std::cout<<"NATIVE_AE3_SKELETAL_GLTF\n"
                 <<"Original named glTF animation clips: "<<model.clipCount()<<"\n"
                 <<"Original skeleton joints: "<<model.boneCount()<<"\n"
                 <<"Host-native deformable vertices: "<<model.skinnedVertexCount()<<"\n";
        for(std::size_t i=0;i<model.clipCount();++i){
            const double duration=model.clipDuration(i);
            std::cout<<"  "<<i<<": "<<model.clipNames()[i]
                     <<" duration="<<duration<<"s\n";
            const auto& half=model.sample(i,duration*.5);
            float minX=1e10f,maxX=-1e10f,minY=1e10f,maxY=-1e10f;
            for(const auto& v:half.triangles){
                minX=std::min(minX,v.x);maxX=std::max(maxX,v.x);
                minY=std::min(minY,v.y);maxY=std::max(maxY,v.y);
            }
            std::cout<<"    midclip native bounds X=["<<minX<<","<<maxX
                     <<"] Y=["<<minY<<","<<maxY<<"]\n";
        }
        if(model.clipCount()==0)
            std::cout<<"No I3M/glTF animation channels in this owner-local GLB.\n";
        std::cout<<"CPU skeletal transforms and Direct3D-compatible output; "
                    "no emulated PlayStation hardware.\n";
        return 0;
    } catch(const std::exception& e){
        std::cerr<<"NATIVE_AE3_SKELETAL_GLTF_FAILED: "<<e.what()<<"\n";
        return 1;
    }
}
