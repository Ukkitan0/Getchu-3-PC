#include "ae3/original_player_mode.hpp"
#include <cstdint>
#include <stdexcept>
static void check(bool b){if(!b)throw std::runtime_error("Original 0x26FB24 Data writes diverged from R5900 MMI");}
int main(){
    ae3::PlayerProgress p{};
    p.chip_count=98;p.max_chip_count=98;
    auto event=ae3::applyOriginalChipGainDataWrites(p,5,true,false);
    check(event.crossedThreshold && p.chip_count==3 && p.max_chip_count==98);
    p.chip_count=98;p.max_chip_count=98;
    event=ae3::applyOriginalChipGainDataWrites(p,1,true,false);
    check(!event.crossedThreshold && p.chip_count==99 && p.max_chip_count==99);
    p.chip_count=999;p.max_chip_count=999;
    event=ae3::applyOriginalChipGainDataWrites(p,5,true,true);
    check(event.crossedThreshold && p.chip_count==4 && p.max_chip_count==999);
    p.chip_count=1;p.max_chip_count=300;
    event=ae3::applyOriginalChipGainDataWrites(p,15000,false,false);
    check(p.chip_count==9999 && p.max_chip_count==15001);
    p.chip_count=1;p.max_chip_count=7;
    event=ae3::applyOriginalChipGainDataWrites(p,-12,false,false);
    check(p.chip_count==0 && p.max_chip_count==7);
    p.chip_count=INT32_MAX;p.max_chip_count=5;
    event=ae3::applyOriginalChipGainDataWrites(p,1,true,false);
    check(p.chip_count==0 && p.max_chip_count==5);
}
