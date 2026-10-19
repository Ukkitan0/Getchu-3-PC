#include "ae3/player_state.hpp"
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace {
void require(bool value,const char* desc){if(!value)throw std::runtime_error(desc);}
}
int main(){
    try{
        ae3::PlayerProgress player{};
        require(sizeof(player)==1033,"original scalar layout offset drift");
        require(player.player_life==100.f,"default player life changed");
        require(ae3::changePlayerHealth(player,-15.f)==85.f,"health damage mismatch");
        require(ae3::changePlayerHealth(player,25.f)==100.f,"health upper clamp mismatch");
        require(ae3::changePlayerHealth(player,-150.f)==0.f,"health lower clamp mismatch");
        require(ae3::changePlayerHealth(player,0.f)==0.f,"zero damage mismatch");
        player.player_energy=67.f;
        player.gmecha_assign[0]=4;
        player.capture_flag[569]=1;
        const auto* bytes=reinterpret_cast<const std::uint8_t*>(&player);
        float energy=0;
        std::memcpy(&energy,bytes+20,sizeof(energy));
        require(energy==67.f,"recovered energy address incorrect");
        require(bytes[204+569]==1,"recovered capture-flag address incorrect");
        require(player.gmecha_assign[0]==4,"native gadget assignment failed");
        std::cout<<"NATIVE_PLAYER_STATE_OK health_clamp_0_100 no_PS2_runtime=yes\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"NATIVE_PLAYER_STATE_FAILED: "<<e.what()<<"\n";
        return 1;
    }
}
