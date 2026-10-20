#include "ae3/original_player_mode.hpp"
namespace ae3 {
bool dispatchOriginalPlayerMode(std::uint32_t mode,OriginalPlayerModeCalls& game){
    // 12-case jump table @ 0x7122FC, method 0x34A57C, USA retail ELF.
    switch(mode){
    case 0: game.call_33fce4(1); return true;
    case 1: game.call_33fce4(5); return true;
    case 2: game.call_33fce4(10); return true;
    case 3: return false;
    case 4: game.call_34675c(20.0f); return true; // original 0x7122F4
    case 5: game.call_34675c(100.0f); return true; // original 0x7122F8
    case 6: return false;
    case 7: game.call_3509ac(1,1); return true;
    case 8: game.call_3509ac(2,1); return true;
    case 9: game.call_33fd90(); return true;
    case 10: game.call_324260(false); return true;
    case 11: game.call_324260(true); return true;
    default: return false;
    }
}
} // namespace ae3


#include <bit>
namespace ae3 {
OriginalChipGainCore originalChipGainBeforeClamp(
    std::int32_t chipCount, std::int32_t gained,
    bool limitedTo100, bool survivalMode) noexcept {
    // Original R5900 0x33FD04..0x33FD40 uses signed compares, but its
    // addu/subu integer arithmetic wraps at 32 bits without C++ UB.
    const int threshold=survivalMode?1000:(limitedTo100?100:-1);
    const auto pending=std::bit_cast<std::int32_t>(
        std::uint32_t(chipCount)+std::uint32_t(gained));
    if(threshold>0 && pending>=threshold){
        return {std::bit_cast<std::int32_t>(
            std::uint32_t(pending)-std::uint32_t(threshold)),threshold,true};
    }
    return {pending,threshold,false};
}
}
