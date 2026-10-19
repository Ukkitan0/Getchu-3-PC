#include "ae3/gltf_scene.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void require(bool condition,const char* message){
    if(!condition)throw std::runtime_error(message);
}
void write32(std::vector<std::uint8_t>& bytes,std::uint32_t n){
    for(int i=0;i<4;++i)bytes.push_back(static_cast<std::uint8_t>(n>>(8*i)));
}
void appendFloat(std::vector<std::uint8_t>& bytes,float n){
    std::uint32_t bits=0;
    static_assert(sizeof(bits)==sizeof(n));
    std::memcpy(&bits,&n,sizeof(bits));
    write32(bytes,bits);
}
std::vector<std::uint8_t> sample(bool withExternalBuffer=false){
    std::vector<std::uint8_t> geometry;
    for(const float n:{0.f,0.f,0.f, 1.f,0.f,0.f, 0.f,1.f,0.f})
        appendFloat(geometry,n);
    // u16 indexed triangle, aligned BIN padding to 4 bytes.
    geometry.insert(geometry.end(),{0,0,1,0,2,0,0,0});
    const std::string json=
      std::string("{\"asset\":{\"version\":\"2.0\"},")
       +"\"buffers\":[{\"byteLength\":44"
       +(withExternalBuffer?std::string(",\"uri\":\"file:///dont-read.bin\""):std::string(""))
       +"}],"
        "\"bufferViews\":[{\"buffer\":0,\"byteOffset\":0,\"byteLength\":36,\"target\":34962},"
                          "{\"buffer\":0,\"byteOffset\":36,\"byteLength\":6,\"target\":34963}],"
        "\"accessors\":[{\"bufferView\":0,\"componentType\":5126,\"count\":3,\"type\":\"VEC3\","
                           "\"min\":[0,0,0],\"max\":[1,1,0]},"
                       "{\"bufferView\":1,\"componentType\":5123,\"count\":3,\"type\":\"SCALAR\"}],"
        "\"meshes\":[{\"primitives\":[{\"attributes\":{\"POSITION\":0},\"indices\":1,"
                "\"mode\":4,\"material\":0}]}],"
        "\"materials\":[{\"pbrMetallicRoughness\":{\"baseColorFactor\":[0.8,0.3,0.1,1]}}],"
        "\"nodes\":[{\"mesh\":0},{\"mesh\":0,\"translation\":[3,1,0]}],"
        "\"scenes\":[{\"nodes\":[0,1]}],\"scene\":0}";
    std::string padded=json;
    while(padded.size()%4)padded.push_back(' ');
    std::vector<std::uint8_t> bytes;
    write32(bytes,0x46546C67);
    write32(bytes,2);
    write32(bytes,static_cast<std::uint32_t>(12+8+padded.size()+8+geometry.size()));
    write32(bytes,static_cast<std::uint32_t>(padded.size()));
    write32(bytes,0x4E4F534A);
    bytes.insert(bytes.end(),padded.begin(),padded.end());
    write32(bytes,static_cast<std::uint32_t>(geometry.size()));
    write32(bytes,0x004E4942);
    bytes.insert(bytes.end(),geometry.begin(),geometry.end());
    return bytes;
}
void realSceneParsing(){
    const auto data=sample();
    auto scene=ae3::loadStaticGlbBytes(data);
    require(scene.sourceNodes==2,"scene graph node transforms lost");
    require(scene.sourcePrimitives==2,"triangle primitives missing");
    require(scene.triangles.size()==6,"two native indexed triangles not triangulated");
    require(scene.bounds.min[0]==0.f && scene.bounds.max[0]==4.f,
            "node world-position X offset missing");
    require(scene.bounds.min[1]==0.f && scene.bounds.max[1]==2.f,
            "node world-position Y offset missing");
    require(scene.bounds.center[0]==2.f && scene.bounds.center[1]==1.f,
            "scene bounds center incorrect");
    require(scene.skinnedPrimitives==0,"synthetic static model reported skinned");
    require(scene.unsupportedPrimitives==0,"ordinary triangles rejected");
    for(const auto& v:scene.triangles){
        require(std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z),
                "host native geometry contains NaN");
        require(v.r>0&&v.g>0&&v.b>0,"material diffuse colors lost");
        require(v.r<=1&&v.g<=1&&v.b<=1,"material RGBA exceeds native range");
    }
}
void invalidInputs(){
    bool rejected=false;
    try{ae3::loadStaticGlbBytes({0,1,2});}
    catch(const std::runtime_error&){rejected=true;}
    require(rejected,"invalid short GLB accepted");
    auto wrong=sample();wrong[0]='P';
    rejected=false;
    try{ae3::loadStaticGlbBytes(wrong);}
    catch(const std::runtime_error&){rejected=true;}
    require(rejected,"invalid binary magic accepted");
    rejected=false;
    try{ae3::loadStaticGlbBytes(sample(true));}
    catch(const std::runtime_error&){rejected=true;}
    require(rejected,"external asset URI was not refused");
}
}
int main(){
    try{
        realSceneParsing();
        invalidInputs();
        std::cout<<"NATIVE_GLB_SCENE_PASS: indexed_triangles=2 nodes=2 "
                    "host_world_transforms=1 gltf_materials=1 no_guest_gpu=1\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"NATIVE_GLB_SCENE_FAILED: "<<e.what()<<"\n";
        return 1;
    }
}
