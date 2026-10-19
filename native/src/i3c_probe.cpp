#include "ae3/i3c_collision.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>
int main(int argc,char** argv){
    if(argc<2||argc>2){
        std::cerr<<"Usage: Getchu3I3cProbe <original-file.i3d|collision-directory>\n";
        return 2;
    }
    try{
        const auto path=std::filesystem::u8path(argv[1]);
        const auto data=std::filesystem::is_directory(path)?
            ae3::loadOriginalI3cDirectory(path):ae3::parseOriginalI3c(path);
        std::cout<<"ORIGINAL_AE3_I3C_COLLISION_DECODED\n"
                 <<"Collision groups: "<<data.groupNames.size()<<"\n"
                 <<"B record AABBs: "<<data.aabbs<<"\n"
                 <<"BVH nodes and leaf records: "<<data.bvhNodes<<"\n"
                 <<"Original game vertices: "<<data.originalVertices<<"\n"
                 <<"Native collision triangles: "<<data.geometry.triangles.size()/3<<"\n"
                 <<"Materials from original name array: "<<data.materialNames.size()<<"\n";
        for(const auto& material:data.materialNames)
            std::cout<<"  original collision material: "<<material<<"\n";
        std::cout<<"Coordinate conversion: original RH Z -> native LH Z\n"
                 <<"No Sony hardware or binary runtime emulated.\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"NATIVE_AE3_COLLISION_ERROR: "<<e.what()<<"\n";
        return 1;
    }
}
