#include "ae3/original_render_visibility.hpp"
#include <stdexcept>

namespace ae3 {
bool passesOriginalRetailRenderEarlyGate(
    const OriginalRenderEarlyGate& input,
    const std::function<bool()>& rejectByOriginalBounds){
    // Retail ld +0x20, ld context+0x58, AND, BEQZL -> early skip.
    if((input.objectMask&input.sceneMask)==0)return false;
    // Retail lbu +0x30/+0x35; BEQZL -> early skip.
    if(!input.flag30||!input.flag35)return false;
    // Retail lbu +0x32; conditional call 0x26C9B8 -> early skip.
    if(input.flag32){
        if(!rejectByOriginalBounds)
            throw std::runtime_error("retail 0x26C9B8 bounds dependency not bound");
        if(rejectByOriginalBounds())return false;
    }
    // Reaches 0x26B550; downstream render behavior still unrecovered.
    return true;
}
} // namespace ae3
