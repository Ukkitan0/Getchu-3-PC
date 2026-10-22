#include "ae3/original_actor_fallback_vu0_random.hpp"
#include <stdexcept>
using namespace ae3;
static void ck(bool b){if(!b)throw std::runtime_error("Source VU0 R semantics mismatch");}
int main(){
    for(auto input:{0u,0x3f8abcdeu,0x3f800000u,0xffffffffu}){
        OriginalActorFallbackVu0R_57ca68 state{input};
        originalFallbackVu0RinitVF0w_57ca68(state);
        ck(state.rawR==0x3f800000u&&state.sourceVRINITCalls==1&&state.sourceVRXORCalls==0);
        originalFallbackVu0RxorVF0x_57ca68(state);
        ck(state.rawR==0x3f800000u&&state.sourceVRXORCalls==1);
        for(unsigned sample=0;sample<6;++sample){
            originalFallbackVu0RandomCandidate_57ca68(state);
            ck(state.rawR==0x3f800000u);
            ck(state.sourceVRINITCalls==1+2*(sample+1));
            ck(state.sourceVRXORCalls==1+2*(sample+1));
        }
    }
}
