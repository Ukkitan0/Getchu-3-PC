#include "ae3/original_monkey_accessors.hpp"
#include <bit>
#include <cstdint>
#include <limits>
#include <stdexcept>

static void check(bool yes,const char* description){
    if(!yes)throw std::runtime_error(description);
}
int main(){
    ae3::OriginalMonkeySecondaryFields obj{};
    obj.word_08=0xBAD00D42;
    obj.word_38=0xAABBCCDD;
    obj.flag_50=0xFF;
    check(ae3::originalMonkeyGetWord08(obj)==0xBAD00D42,"original 0x571648 +8 load differs");
    check(ae3::originalMonkeyGetWord38(obj)==0xAABBCCDD,"original 0x571658 +0x38 load differs");
    ae3::originalMonkeySetFlag50(obj);
    check(obj.flag_50==1,"original 0x5716C4 store differs");
    obj.word_08=0;obj.word_38=0;
    check(ae3::originalMonkeyGetWord08(obj)==0 && ae3::originalMonkeyGetWord38(obj)==0,
          "original secondary methods do not preserve zero");
    ae3::OriginalMonkeyAngleFields pose{};
    constexpr float originalFactor = std::bit_cast<float>(std::uint32_t{0x3C8EFA34});
    for(float angle:{0.0f,45.0f,90.0f,180.0f,-90.0f,360.0f}){
        pose.angle_degrees_37c=angle;
        const auto result=ae3::originalMonkeyAngleToRadians(pose);
        check(std::bit_cast<std::uint32_t>(result)==
              std::bit_cast<std::uint32_t>(angle*originalFactor),
              "original 0x316928 floating-point multiplication differs");
    }
    pose.angle_degrees_37c=std::numeric_limits<float>::infinity();
    check(ae3::originalMonkeyAngleToRadians(pose)==std::numeric_limits<float>::infinity(),
          "original float input infinity must be preserved");
}
