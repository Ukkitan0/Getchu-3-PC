#include "ae3/original_navrange_matrix_inverse.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <stdexcept>
using namespace ae3;
static void ck(bool ok){if(!ok)throw std::runtime_error("Retail inverse 0x359F44 regression");}
static bool close(float a,float b,float eps=0.0003f){return std::fabs(a-b)<=eps;}
static float at(const OriginalNavrangeMatrix4& a,int row,int col){
    const auto& v=a[static_cast<std::size_t>(col)];
    return row==0?v.x:row==1?v.y:row==2?v.z:v.w;
}
static void checkInverse(const OriginalNavrangeMatrix4& matrix){
    const auto inv=originalNavrangeInverseCofactors_359f44(matrix);
    for(int r=0;r<4;r++)for(int c=0;c<4;c++){
        float product=0.f;
        for(int k=0;k<4;k++)product+=at(matrix,r,k)*at(inv,k,c);
        ck(close(product,r==c?1.f:0.f,0.002f));
    }
}
int main(){
    const OriginalNavrangeMatrix4 ident{{
        {1.f,0.f,0.f,0.f},{0.f,1.f,0.f,0.f},
        {0.f,0.f,1.f,0.f},{0.f,0.f,0.f,1.f}}};
    checkInverse(ident);
    auto inv=originalNavrangeInverseCofactors_359f44(ident);
    for(int r=0;r<4;r++)for(int c=0;c<4;c++)
        ck(close(at(inv,r,c),at(ident,r,c)));
    const OriginalNavrangeMatrix4 diagonal{{
        {2.f,0.f,0.f,0.f},{0.f,-4.f,0.f,0.f},
        {0.f,0.f,0.5f,0.f},{0.f,0.f,0.f,3.f}}};
    checkInverse(diagonal);
    auto d=originalNavrangeInverseCofactors_359f44(diagonal);
    ck(close(d[0].x,0.5f)&&close(d[1].y,-0.25f)&&
       close(d[2].z,2.f)&&close(d[3].w,1.f/3.f));
    const OriginalNavrangeMatrix4 affine{{
        {2.f,0.f,0.f,0.f},{0.5f,3.f,0.f,0.f},
        {0.25f,0.75f,4.f,0.f},{5.f,-2.f,11.f,1.f}}};
    checkInverse(affine);
    auto ai=originalNavrangeInverseCofactors_359f44(affine);
    ck(close(ai[3].x,-2.4947917f,0.001f));
    for(float w: {1.f,0.9238795f,0.70710678f}){
        float z=std::sqrt(1.f-w*w);
        const auto rotation=originalNavrangeQuaternionMatrix_359e74(
          {0u,0u,std::bit_cast<std::uint32_t>(z),std::bit_cast<std::uint32_t>(w)});
        checkInverse(rotation);
        const auto transpose=originalNavrangeInverseCofactors_359f44(rotation);
        for(int r=0;r<3;r++)for(int c=0;c<3;c++)
            ck(close(at(transpose,r,c),at(rotation,c,r),0.001f));
    }
    const OriginalNavrangeGpr128 quaternion{0u,0u,
        std::bit_cast<std::uint32_t>(0.382683432f),
        std::bit_cast<std::uint32_t>(0.923879533f)};
    const auto composed=originalNavrangeInverseQuaternion_21efa4_21efdc(quaternion);
    const auto direct=originalNavrangeInverseCofactors_359f44(
        originalNavrangeQuaternionMatrix_359e74(quaternion));
    for(int r=0;r<4;r++)for(int c=0;c<4;c++)
        ck(close(at(composed,r,c),at(direct,r,c)));
    // Source has no singular-case branch or identity fallback.
    OriginalNavrangeMatrix4 singular=ident;
    singular[1]=singular[0];
    const auto s=originalNavrangeInverseCofactors_359f44(singular);
    bool anyNonfinite=false;
    for(int r=0;r<4;r++)for(int c=0;c<4;c++)
        anyNonfinite|=!std::isfinite(at(s,r,c));
    ck(anyNonfinite);
}
