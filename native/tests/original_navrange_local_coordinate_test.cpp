#include "ae3/original_navrange_local_coordinate.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <cmath>
#include <stdexcept>
using namespace ae3;
static void ck(bool v){if(!v)throw std::runtime_error("original navrange local projection wrong");}
int main(){
    const OriginalNavrangeMatrix4 identity{{
        {1.f,0.f,0.f,0.f},{0.f,1.f,0.f,0.f},
        {0.f,0.f,1.f,0.f},{0.f,0.f,0.f,1.f}}};
    auto result=originalNavrangeLocalCoordinate_21f020_21f074(
        identity,{12.f,26.f,34.f,1.f},{10.f,20.f,30.f,0.f});
    ck(result.x==2.f&&result.y==6.f&&result.z==4.f&&result.w==1.f);
    // Column-major inverse quarter-turn around Z.
    const OriginalNavrangeMatrix4 inverseZ{{
        {0.f,-1.f,0.f,0.f},{1.f,0.f,0.f,0.f},
        {0.f,0.f,1.f,0.f},{0.f,0.f,0.f,1.f}}};
    const auto rotated=originalNavrangeLocalCoordinate_21f020_21f074(
        inverseZ,{2.f,0.f,0.f,1.f},{0.f,0.f,0.f,0.f});
    ck(rotated.x==0.f&&rotated.y==-2.f&&rotated.z==0.f&&rotated.w==1.f);
    const OriginalNavrangeMatrix4 matrix{{
        {1.f,2.f,3.f,4.f},{5.f,6.f,7.f,8.f},
        {9.f,10.f,11.f,12.f},{13.f,14.f,15.f,16.f}}};
    const auto m=originalNavrangeLocalCoordinate_21f020_21f074(
        matrix,{2.f,-1.f,0.5f,1.f},{0.f,0.f,0.f,0.f});
    ck(m.x==14.5f&&m.y==17.f&&m.z==19.5f&&m.w==22.f);
    OriginalNavrangeResourceFrame frame{};
    frame.argumentA0_20={std::bit_cast<std::uint32_t>(10.f),
        std::bit_cast<std::uint32_t>(20.f),std::bit_cast<std::uint32_t>(30.f),0u};
    const auto composed=originalNavrangeLocalCentroidFromFrame_21efa4_21f074(
        frame,{12.f,26.f,34.f,1.f});
    ck(composed.x==2.f&&composed.y==6.f&&composed.z==4.f&&composed.w==1.f);
    for(int i=0;i<1000;i++){
        const float f=static_cast<float>(i)*0.125f;
        const auto p=originalNavrangeLocalCoordinate_21f020_21f074(
            identity,{f+1.f,f+2.f,f+3.f,1.f},{f,f,f,0.f});
        ck(p.x==1.f&&p.y==2.f&&p.z==3.f&&p.w==1.f);
    }
}
