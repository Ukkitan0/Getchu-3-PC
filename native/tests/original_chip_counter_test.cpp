#include "ae3/original_player_mode.hpp"
#include <cstdint>
#include <stdexcept>
static void check(bool yes){
    if(!yes)throw std::runtime_error("Original retail chip gain math diverged");
}
int main(){
    using ae3::originalChipGainBeforeClamp;
    auto a=originalChipGainBeforeClamp(98,5,true,false);
    check(a.crossedThreshold && a.threshold==100 && a.pendingBeforeFinalClamp==3);
    a=originalChipGainBeforeClamp(98,1,true,false);
    check(!a.crossedThreshold && a.pendingBeforeFinalClamp==99);
    a=originalChipGainBeforeClamp(999,5,true,true);
    check(a.crossedThreshold && a.threshold==1000 && a.pendingBeforeFinalClamp==4);
    a=originalChipGainBeforeClamp(99,1,true,false);
    check(a.crossedThreshold && a.pendingBeforeFinalClamp==0);
    a=originalChipGainBeforeClamp(1000,2505,false,true);
    check(a.crossedThreshold && a.pendingBeforeFinalClamp==2505);
    a=originalChipGainBeforeClamp(100,10,false,false);
    check(!a.crossedThreshold && a.threshold==-1 && a.pendingBeforeFinalClamp==110);
    // Preserve original 32-bit MIPS addu behavior without signed overflow.
    a=originalChipGainBeforeClamp(INT32_MAX,1,true,false);
    check(!a.crossedThreshold && a.pendingBeforeFinalClamp==INT32_MIN);
}
