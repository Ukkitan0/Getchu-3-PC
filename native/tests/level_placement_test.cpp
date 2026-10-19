#include "ae3/level_scene.hpp"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
void u32(std::vector<std::uint8_t>& b,std::size_t at,std::uint32_t n){
    for(int i=0;i<4;++i)b.at(at+std::size_t(i))=std::uint8_t(n>>(8*i));
}
void putf(std::vector<std::uint8_t>& b,std::size_t at,float n){
    std::uint32_t v=0;std::memcpy(&v,&n,sizeof(v));u32(b,at,v);
}
void name(std::vector<std::uint8_t>& b,std::size_t at,const char* text){
    for(std::size_t i=0;text[i];++i)b.at(at+i)=std::uint8_t(text[i]);
}
}
int main(){
    try{
        std::vector<std::uint8_t> data(192+2*128);
        u32(data,0,2);
        name(data,192,"closet_door");
        putf(data,192+0x40,17.25f);
        putf(data,192+0x44,5.f);
        putf(data,192+0x48,-2.f);
        name(data,320,"broken_pos");
        putf(data,320+0x40,1e9f);
        putf(data,320+0x44,2.f);
        putf(data,320+0x48,3.f);
        auto report=ae3::parsePlacements(data);
        check(report.entries.size()==1,"original PLC simple-record count wrong");
        check(report.skipped==1,"bad PLC coordinate sanity check omitted");
        check(report.entries[0].name=="closet_door","PLC name corrupted");
        check(report.entries[0].xyz[0]==17.25f,"PLC X offset incorrect");
        check(report.entries[0].xyz[1]==5.f,"PLC Y offset incorrect");
        check(report.entries[0].xyz[2]==-2.f,"PLC Z offset incorrect");
        check(ae3::matchesPlacement("a_wod_c_closet_door","closet_door"),
              "original AE3 asset naming suffix not matched");
        check(ae3::matchesPlacement("CLOSET_DOOR","closet_door"),
              "case-insensitive same-name match failed");
        check(!ae3::matchesPlacement("long_closet_door_alt","closet_door"),
              "overlapping suffix falsely matched");
        check(!ae3::matchesPlacement("closet_door_fake","closet_door"),
              "incorrect object was placed");
        bool rejected=false;
        try{ae3::parsePlacements(std::vector<std::uint8_t>{});}catch(const std::runtime_error&){rejected=true;}
        check(rejected,"malformed PLC accepted");
        std::cout<<"NATIVE_PLC_PLACEMENT_PASSED original_world_xyz=1 matching=1\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"NATIVE_PLC_PLACEMENT_FAILED: "<<e.what()<<"\n";
        return 1;
    }
}
