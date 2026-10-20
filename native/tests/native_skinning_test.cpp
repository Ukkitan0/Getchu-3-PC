#include "ae3/native_skinning.hpp"
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void check(bool yes,const char* why){if(!yes)throw std::runtime_error(why);}
struct Fixture {
    std::vector<std::uint8_t> data;
    std::vector<std::string> views;
    int add(const std::vector<std::uint8_t>& bytes,int target=0){
        while(data.size()%4)data.push_back(0);
        const auto start=data.size();
        data.insert(data.end(),bytes.begin(),bytes.end());
        std::ostringstream o;
        o<<"{\"buffer\":0,\"byteOffset\":"<<start
         <<",\"byteLength\":"<<bytes.size();
        if(target)o<<",\"target\":"<<target;
        o<<"}";
        views.push_back(o.str());
        return static_cast<int>(views.size()-1);
    }
};
template <typename T>
std::vector<std::uint8_t> packed(std::initializer_list<T> values){
    std::vector<std::uint8_t> bytes(sizeof(T)*values.size());
    std::size_t at=0;
    for(const auto& item:values){
        std::memcpy(bytes.data()+at,&item,sizeof(T));
        at+=sizeof(T);
    }
    return bytes;
}
void put32(std::vector<std::uint8_t>& b,std::uint32_t x){
    for(int i=0;i<4;++i)b.push_back(std::uint8_t(x>>(i*8)));
}
std::vector<std::uint8_t> fixtureGlb(){
    Fixture f;
    const int pos=f.add(packed<float>({
        1,0,0, 0,1,0, 0,0,0
    }),34962);
    const int joints=f.add(packed<std::uint16_t>({
        0,0,0,0, 0,0,0,0, 0,0,0,0
    }),34962);
    const int weights=f.add(packed<float>({
        1,0,0,0, 1,0,0,0, 1,0,0,0
    }),34962);
    const int indices=f.add(packed<std::uint16_t>({0,1,2}),34963);
    const int inverse=f.add(packed<float>({
        1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1
    }));
    const int times=f.add(packed<float>({0,1}));
    const int rotation=f.add(packed<float>({
        0,0,0,1,
        0,0,0.70710678f,0.70710678f
    }));
    const int translation=f.add(packed<float>({
        0,0,0, 0,2,0
    }));
    while(f.data.size()%4)f.data.push_back(0);
    std::ostringstream json;
    json<<"{\"asset\":{\"version\":\"2.0\"},"
           "\"buffers\":[{\"byteLength\":"<<f.data.size()<<"}],"
           "\"bufferViews\":[";
    for(std::size_t i=0;i<f.views.size();++i){
        if(i)json<<",";
        json<<f.views[i];
    }
    json<<"],\"accessors\":["
         <<"{\"bufferView\":"<<pos<<",\"componentType\":5126,\"count\":3,\"type\":\"VEC3\","
           "\"min\":[0,0,0],\"max\":[1,1,0]},"
         <<"{\"bufferView\":"<<joints<<",\"componentType\":5123,\"count\":3,\"type\":\"VEC4\"},"
         <<"{\"bufferView\":"<<weights<<",\"componentType\":5126,\"count\":3,\"type\":\"VEC4\"},"
         <<"{\"bufferView\":"<<indices<<",\"componentType\":5123,\"count\":3,\"type\":\"SCALAR\"},"
         <<"{\"bufferView\":"<<inverse<<",\"componentType\":5126,\"count\":1,\"type\":\"MAT4\"},"
         <<"{\"bufferView\":"<<times<<",\"componentType\":5126,\"count\":2,\"type\":\"SCALAR\","
           "\"min\":[0],\"max\":[1]},"
         <<"{\"bufferView\":"<<rotation<<",\"componentType\":5126,\"count\":2,\"type\":\"VEC4\"},"
         <<"{\"bufferView\":"<<translation<<",\"componentType\":5126,\"count\":2,\"type\":\"VEC3\"}"
         <<"],\"meshes\":[{\"primitives\":[{\"attributes\":{\"POSITION\":0,"
           "\"JOINTS_0\":1,\"WEIGHTS_0\":2},\"indices\":3,\"mode\":4}]}],"
           "\"nodes\":[{\"name\":\"NativeSkinMesh\",\"mesh\":0,\"skin\":0},"
                       "{\"name\":\"jnt_test\",\"translation\":[0,0,0]}],"
           "\"skins\":[{\"name\":\"original_track_test\","
             "\"joints\":[1],\"inverseBindMatrices\":4,\"skeleton\":1}],"
           "\"animations\":[{\"name\":\"OriginalI3mTest\","
             "\"samplers\":[{\"input\":5,\"output\":6,\"interpolation\":\"LINEAR\"},"
                           "{\"input\":5,\"output\":7,\"interpolation\":\"LINEAR\"}],"
             "\"channels\":[{\"sampler\":0,\"target\":{\"node\":1,\"path\":\"rotation\"}},"
                            "{\"sampler\":1,\"target\":{\"node\":1,\"path\":\"translation\"}}]}],"
           "\"scenes\":[{\"nodes\":[0,1]}],\"scene\":0}";
    std::string j=json.str();
    while(j.size()%4)j.push_back(' ');
    std::vector<std::uint8_t> glb;
    put32(glb,0x46546C67);put32(glb,2);
    put32(glb,static_cast<std::uint32_t>(12+8+j.size()+8+f.data.size()));
    put32(glb,static_cast<std::uint32_t>(j.size()));
    put32(glb,0x4E4F534A);
    glb.insert(glb.end(),j.begin(),j.end());
    put32(glb,static_cast<std::uint32_t>(f.data.size()));
    put32(glb,0x004E4942);
    glb.insert(glb.end(),f.data.begin(),f.data.end());
    return glb;
}
void test(std::vector<std::uint8_t> bytes){
    ae3::NativeSkinnedGlb model(bytes);
    check(model.clipCount()==1,"named original-style clip count missing");
    check(model.clipNames()[0]=="OriginalI3mTest","original clip label missing");
    check(model.boneCount()==1,"original skeleton joint count lost");
    check(model.skinnedVertexCount()==3,"original four-weight skin vertex count lost");
    check(std::abs(model.clipDuration(0)-1)<1e-6,"clip duration wrong");
    const auto& rest=model.sample(0,0).triangles;
    check(rest.size()==3,"animated source triangle count incorrect");
    check(std::abs(rest[0].x-1)<1e-5 && std::abs(rest[0].y)<1e-5,
          "native rest pose does not preserve original joint binding");
    const auto& half=model.sample(0,.5).triangles;
    check(std::abs(half[0].x-0.70710678f)<.002f,
          "native quaternion interpolation did not rotate skinned mesh");
    check(std::abs(half[0].y-1.70710678f)<.002f,
          "native weighted skin translation not composed with rotation");
    check(std::abs(half[2].x+0.70710678f)<.002f,
          "native second original source vertex did not follow animated joint");
    check(std::abs(half[1].y-1.f)<.002f,
          "native root triangle vertex did not follow original bone translation");
    const auto& loop=model.sample(0,1.5).triangles;
    check(std::abs(loop[0].x-half[0].x)<1e-5f &&
          std::abs(loop[0].y-half[0].y)<1e-5f,
          "native animation did not loop without guest hardware runtime");
    bool rejected=false;
    try{model.sample(1,.2);}
    catch(const std::runtime_error&){rejected=true;}
    check(rejected,"out-of-range original animation clip was silently accepted");
}
}
int main(int argc,char** argv){
    try{
        const auto data=fixtureGlb();
        test(data);
        if(argc==2){
            std::filesystem::path file=std::filesystem::u8path(argv[1]);
            std::filesystem::create_directories(file.parent_path());
            std::ofstream stream(file,std::ios::binary);
            stream.write(reinterpret_cast<const char*>(data.data()),
                         static_cast<std::streamsize>(data.size()));
            check(bool(stream),"cannot publish generated host-native GLB animation fixture");
        }
        std::cout<<"NATIVE_ORIGINAL_I3M_GLB_SKINNING_PASSED "
                    "named_joint=1 inverse_bind=1 keyframe_slerp=1 "
                    "weighted_vertices=3 clip_loop=1 no_ps2_hardware=1\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"NATIVE_ORIGINAL_I3M_GLB_SKINNING_FAILED: "<<e.what()<<"\n";
        return 1;
    }
}
