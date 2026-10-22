#include "ae3/original_pad_actuator.hpp"
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
void check(bool ok,const char* what){if(!ok)throw std::runtime_error(what);}
void sourceActuatorControl(){
    ae3::OriginalPadActuatorController_51790c pad;
    check(!pad.request(0,0.0f),"retail initial zero should be silent");
    auto packet=pad.request(1,1.0f);
    check(packet && packet->originalControlCode==1 &&
          packet->originalSelector==3 && packet->originalByteCount==2,
          "retail 0x405E30 argument layout differs");
    check(packet->bytes[0]==0xff && packet->bytes[1]==1,
          "255.0f * source double-byte encoding changed");
    check(!pad.request(1,1.0f),"duplicate actuator command emitted");
    packet=pad.request(0,1.0f);
    check(packet && packet->bytes[0]==0xfe && packet->bytes[1]==1,
          "motor mode low bit not preserved");
    packet=pad.request(0,0.5f);
    check(packet && packet->bytes[0]==0 && packet->bytes[1]==1,
          "half-scale float conversion/rounding changed");
    packet=pad.request(1,0.5f);
    check(packet && packet->bytes[0]==1 && packet->bytes[1]==1,
          "mode-only change should still send command");
    packet=pad.request(1,0.f);
    check(packet && packet->bytes[0]==1 && packet->bytes[1]==0,
          "zero intensity with mode flag did not generate source command");
    check(!pad.request(1,std::numeric_limits<float>::quiet_NaN()),
          "source-unsupported nonfinite CVT.W.S was silently encoded");
    check(!pad.request(1,0.f),"invalid float altered preserved actuator state");
    pad.reset();
    packet=pad.request(1,1.f);
    check(packet && packet->bytes[0]==255,"original pad reset did not reset actuator state");
}
}
int main(){try{sourceActuatorControl();
    std::cout<<"ORIGINAL_RETAIL_ACTUATOR_51790C_405E30_PASSED\n";
    return 0;
}catch(const std::exception& e){
    std::cerr<<"ORIGINAL_RETAIL_ACTUATOR_FAILED: "<<e.what()<<'\n';
    return 1;
}}
