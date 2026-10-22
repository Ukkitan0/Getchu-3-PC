#include "ae3/original_render_packet_arena.hpp"
#include <iostream>
#include <stdexcept>
#include <cstdint>
using namespace ae3;
namespace {
void require(bool v,const char* s){if(!v)throw std::runtime_error(s);}
void run(){
    auto arena=originalRenderArenaAtCapacity(0x1000);
    require(arena.front14==0&&arena.back18==0x1000,"native render arena initialized incorrectly");
    const auto back=originalReserveRenderPacketFromBack3b3ce0(arena,4);
    require(back==0xfc0&&arena.back18==0xfc0,"source 3B3CE0 must reserve 4*16 from tail");
    require(originalReserveRenderPacketFromBack3b3ce0(arena,3)==0xf90,
            "source 3B3CE0 second rear allocation wrong");
    auto front=originalReserveRenderPacketFromFront3b3d00(arena,2);
    require(front==0&&arena.front14==32,"source front allocation returns OLD cursor");
    front=originalReserveRenderPacketFromFront3b3d00(arena,3);
    require(front==32&&arena.front14==80,"source 3B3D00 forward allocation order wrong");
    arena.front14=17;
    front=originalReserveRenderPacketFromFront3b3d00(arena,1);
    require(front==17&&arena.front14==48,"original forward alignment must include +15 and mask");
    arena.front14=0x100;arena.back18=0x120;
    require(originalReserveRenderPacketFromBack3b3ce0(arena,1)==0x110,
            "backward boundary did not align correctly");
    bool blocked=false;
    try{originalReserveRenderPacketFromBack3b3ce0(arena,2);}
    catch(const std::length_error&){blocked=true;}
    require(blocked,"original backward source allocator crossed forward cursor");
    blocked=false;
    try{originalReserveRenderPacketFromFront3b3d00(arena,2);}
    catch(const std::length_error&){blocked=true;}
    require(blocked,"original forward source allocator crossed rear cursor");
    blocked=false;
    try{originalReserveRenderPacketFromFront3b3d00(arena,UINT32_MAX);}
    catch(const std::length_error&){blocked=true;}
    require(blocked,"original source count shift overflow not rejected");
    blocked=false;
    try{originalRenderArenaAtCapacity(7);}
    catch(const std::invalid_argument&){blocked=true;}
    require(blocked,"unaligned renderer capacity accepted");
}
}
int main(){
    try{run();std::cout<<"ORIGINAL_RENDER_PACKET_ARENA_PASS: retail_back3B3CE0 front3B3D00 16byte_unit alignment collision_guard\n";return 0;}
    catch(const std::exception&e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
