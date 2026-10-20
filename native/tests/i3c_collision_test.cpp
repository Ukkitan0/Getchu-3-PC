#include "ae3/i3c_collision.hpp"
#include "ae3/native_walking.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void check(bool yes,const char* why){if(!yes)throw std::runtime_error(why);}
void u16(std::vector<std::uint8_t>& b,std::size_t at,std::uint16_t n){
    b.at(at)=std::uint8_t(n);b.at(at+1)=std::uint8_t(n>>8);
}
void u32(std::vector<std::uint8_t>& b,std::size_t at,std::uint32_t n){
    u16(b,at,std::uint16_t(n));u16(b,at+2,std::uint16_t(n>>16));
}
void f32(std::vector<std::uint8_t>& b,std::size_t at,float n){
    std::uint32_t bits=0;std::memcpy(&bits,&n,sizeof(bits));u32(b,at,bits);
}
void str(std::vector<std::uint8_t>& b,std::size_t at,const char* name){
    for(std::size_t i=0;name[i];++i)b.at(at+i)=std::uint8_t(name[i]);
}
std::vector<std::uint8_t> cube(){
    constexpr unsigned bytes=0x300;
    std::vector<std::uint8_t> b(bytes);
    str(b,0,"I3D_I3C"); // eighth byte remains NUL
    u32(b,0x08,0x00030000);u32(b,0x0c,bytes);
    b[0x10]=1;
    u16(b,0x14,1);u16(b,0x16,1);
    u32(b,0x18,0x40);u32(b,0x1c,0x50);
    u32(b,0x40,0x60);u16(b,0x44,1);u32(b,0x48,0x80);
    u32(b,0x50,0x70);
    str(b,0x60,"ROOTNODE");
    str(b,0x70,"coll_wall");
    f32(b,0x80+0,0);f32(b,0x80+4,0);f32(b,0x80+8,0);
    f32(b,0x80+12,1.f);
    f32(b,0x90,.5f);f32(b,0x94,.5f);f32(b,0x98,.5f);
    u32(b,0x80+0x28,0x180);u32(b,0x80+0x2c,0x100);
    const float vertices[8][3]={
        {-.5f,-.5f,-.5f},{.5f,-.5f,-.5f},
        {.5f,-.5f,.5f},{-.5f,-.5f,.5f},
        {-.5f,.5f,-.5f},{.5f,.5f,-.5f},
        {.5f,.5f,.5f},{-.5f,.5f,.5f}
    };
    for(int i=0;i<8;++i)
        for(int j=0;j<3;++j)
            f32(b,0x100+std::size_t(i)*16+std::size_t(j)*4,vertices[i][j]);
    u16(b,0x180,12);u32(b,0x188,0x190);
    const std::uint16_t tri[12][3]={
       {0,2,1},{0,3,2},{4,5,6},{4,6,7},
       {0,1,5},{0,5,4},{1,2,6},{1,6,5},
       {2,3,7},{2,7,6},{3,0,4},{3,4,7}
    };
    for(int i=0;i<12;++i){
        const auto at=0x190+std::size_t(i)*12;
        u16(b,at,0x8001);
        u32(b,at+8,static_cast<std::uint32_t>(0x220+i*8));
        const auto leaf=0x220+std::size_t(i)*8;
        for(int j=0;j<3;++j)u16(b,leaf+std::size_t(j)*2,tri[i][j]);
        u16(b,leaf+6,static_cast<std::uint16_t>(i));
    }
    return b;
}
void fullNativeCollision(){
    const auto original=cube();
    const auto data=ae3::parseOriginalI3c(original);
    check(data.aabbs==1 && data.groupNames.size()==1,"actual I3C A record lost");
    check(data.groupNames[0]=="ROOTNODE","actual I3C group name lost");
    check(data.materialNames.size()==1 && data.materialNames[0]=="coll_wall",
          "original I3C surface material was guessed or not read");
    check(data.bvhNodes==13,"original BVH leaf recursion lost");
    check(data.originalVertices==8,"I3C vertex count must derive from BVH minus vertex pointer");
    check(data.geometry.triangles.size()==36,"original I3C cube did not decode 12 triangles");
    check(data.geometry.bounds.min[0]==-.5f && data.geometry.bounds.max[2]==.5f,
          "native coordinate transform changed original cube dimension");
    auto top=ae3::NativeWalkingController::groundHeight(data.geometry,0,0);
    check(top && std::abs(*top-.5f)<1e-6f,
          "native walking did not use TRUE I3C collision cube top surface");
    ae3::NativeWalkingController walking;
    check(walking.reset(data.geometry),"native walking did not enter original I3C collision");
    bool landed=false;
    for(int i=0;i<200;++i){
        walking.tick(data.geometry,0.016f,{});
        landed=landed||walking.state().grounded;
    }
    check(landed,"walking against actual I3C cube could not find walkable ground");
}
void corruptions(){
    auto invalid=cube();
    u16(invalid,0x220,99);
    bool rejected=false;
    try{ae3::parseOriginalI3c(invalid);}catch(const std::runtime_error&){rejected=true;}
    check(rejected,"I3C bad original indexed vertex accepted");
    invalid=cube();u32(invalid,0x188,0xffffff00);
    rejected=false;
    try{ae3::parseOriginalI3c(invalid);}catch(const std::runtime_error&){rejected=true;}
    check(rejected,"I3C out-of-bounds BVH child accepted");
    invalid=cube();invalid[0x10]=0;
    rejected=false;
    try{ae3::parseOriginalI3c(invalid);}catch(const std::runtime_error&){rejected=true;}
    check(rejected,"I3C unsupported pointer fixup state accepted");
    invalid=cube();u32(invalid,12,0xffffff);
    rejected=false;
    try{ae3::parseOriginalI3c(invalid);}catch(const std::runtime_error&){rejected=true;}
    check(rejected,"I3C file size mismatch accepted");
}
void originalPckCollisionDiscovery(){
    namespace fs=std::filesystem;
    // PCK storage uses the authentic 16-byte member table. The I3C data
    // inside is a test-only collision geometry fixture (no retail assets).
    const auto embedded=cube();
    std::vector<std::uint8_t> pack(128+embedded.size(),0);
    str(pack,0,"PCK");
    str(pack,16,"collision");
    u32(pack,4,64);u32(pack,8,1);
    u32(pack,64,0);u32(pack,68,0);
    u32(pack,72,128);u32(pack,76,
        static_cast<std::uint32_t>(embedded.size()));
    std::copy(embedded.begin(),embedded.end(),pack.begin()+128);
    const fs::path root=fs::temp_directory_path()/"getchu3_native_pck_collision";
    fs::remove_all(root);
    fs::create_directories(root);
    try{
        std::ofstream file(root/"bg.pck",std::ios::binary);
        file.write(reinterpret_cast<const char*>(pack.data()),
                   static_cast<std::streamsize>(pack.size()));
        file.close();
        const auto collision=ae3::loadOriginalI3cDirectory(root);
        check(collision.geometry.triangles.size()==36 &&
              collision.originalVertices==8 && collision.bvhNodes==13,
              "real PCK member indexing failed to feed original-format I3C native collision");
        fs::remove_all(root);
    }catch(...){fs::remove_all(root);throw;}
}
void originalFileDiscovery(){
    namespace fs=std::filesystem;
    const fs::path root=fs::temp_directory_path()/"getchu3_native_true_i3c_fixture";
    fs::remove_all(root);
    fs::create_directories(root);
    try{
        const auto bytes=cube();
        std::ofstream file(root/"original_collision.i3d",std::ios::binary);
        file.write(reinterpret_cast<const char*>(bytes.data()),
                   static_cast<std::streamsize>(bytes.size()));
        file.close();
        std::ofstream junk(root/"ordinary_visual.i3d",std::ios::binary);
        junk<<"I3D_BIN -- not a collision file";
        junk.close();
        const auto loaded=ae3::loadOriginalI3cDirectory(root);
        check(loaded.bvhNodes==13 && loaded.geometry.triangles.size()==36,
              "native extracted-folder scanner failed to distinguish I3D_I3C and I3D_BIN");
        fs::remove_all(root);
    }catch(...){fs::remove_all(root);throw;}
}
}
int main(){
    try{
        fullNativeCollision();
        corruptions();
        originalFileDiscovery();
        originalPckCollisionDiscovery();
        std::cout<<"NATIVE_REAL_I3C_PASS original_vertices=8 original_triangles=12 "
                    "collision_ground=1 bvh=13 no_ps2_runtime=1\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"NATIVE_REAL_I3C_FAILED: "<<e.what()<<"\n";
        return 1;
    }
}
