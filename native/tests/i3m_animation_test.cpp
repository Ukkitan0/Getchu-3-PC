#include "ae3/i3m_animation.hpp"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
void check(bool ok,const char* error){if(!ok)throw std::runtime_error(error);}
void word(std::vector<std::uint8_t>& b,std::size_t at,std::uint16_t n){
    b.at(at)=std::uint8_t(n);b.at(at+1)=std::uint8_t(n>>8);
}
void dword(std::vector<std::uint8_t>& b,std::size_t at,std::uint32_t n){
    word(b,at,std::uint16_t(n));word(b,at+2,std::uint16_t(n>>16));
}
void realFloat(std::vector<std::uint8_t>& b,std::size_t at,float f){
    std::uint32_t u=0;std::memcpy(&u,&f,4);dword(b,at,u);
}
std::vector<std::uint8_t> fixture(){
    std::vector<std::uint8_t> b(128);
    for(int j=0;j<7;++j)b[std::size_t(j)]=std::uint8_t("I3D_I3M"[j]);
    word(b,0x08,1);dword(b,0x0c,128);
    word(b,0x14,1);realFloat(b,0x18,2.f);
    dword(b,0x1c,40);dword(b,0x20,112);
    dword(b,40,64);word(b,44,5);word(b,46,2);dword(b,48,88);
    for(int j=0;j<8;++j)b[64+std::size_t(j)]=std::uint8_t("jnt_armL"[j]);
    word(b,88,0);word(b,90,6);word(b,92,0);word(b,94,0);word(b,96,2);
    word(b,98,20);word(b,100,6);word(b,102,0);word(b,104,1);word(b,106,2);
    word(b,112,0);word(b,114,0);word(b,116,0);word(b,118,32767);
    word(b,120,0);word(b,122,23170);word(b,124,0);word(b,126,23170);
    return b;
}
}
int main(){
    try{
        const auto anim=ae3::parseI3m(fixture());
        check(anim.tracks.size()==1,"I3M bone quaternion track missing");
        check(anim.duration==2.f,"I3M game clip duration not recovered");
        check(anim.tracks[0].name=="jnt_armL","I3M original named-track lost");
        check(anim.tracks[0].keys.size()==2,"I3M keyframe count corrupted");
        check(anim.tracks[0].keys[0].seconds==0.f,"I3M start key time wrong");
        check(anim.tracks[0].keys[1].seconds==2.f,"I3M quantized time-to-seconds wrong");
        const auto sampled=ae3::sampleTrack(anim.tracks[0],1.f);
        check(std::abs(sampled[1]-0.382683f)<.002f,
              "native quaternion shortest-arc rotation interpolation wrong");
        check(std::abs(sampled[3]-0.923879f)<.002f,
              "native interpolation changed quaternion norm");
        const auto last=ae3::sampleTrack(anim.tracks[0],99.f);
        check(std::abs(last[1]-0.707106f)<.002f,"I3M final pose missing");
        auto bad=fixture();dword(bad,0x20,125);
        bool rejected=false;
        try{ae3::parseI3m(bad);}catch(const std::runtime_error&){rejected=true;}
        check(rejected,"out-of-range original rotation-value pool was accepted");
        rejected=false;
        try{ae3::parseI3m(std::vector<std::uint8_t>{});}
        catch(const std::runtime_error&){rejected=true;}
        check(rejected,"invalid AE3 I3M magic was accepted");
        std::cout<<"NATIVE_ORIGINAL_I3M_PASSED keyframes=2 duration=2 "
                    "named_bone=1 rotation_slerp=1 no_ee_iop=1\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"NATIVE_ORIGINAL_I3M_FAILED: "<<e.what()<<"\n";
        return 1;
    }
}
