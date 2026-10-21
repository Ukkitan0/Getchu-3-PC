#include "ae3/original_navrange_mmi_pack.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <stdexcept>
using namespace ae3;
static void ck(bool value){if(!value)throw std::runtime_error("Original navrange matrix regression");}
static OriginalNavrangeGpr128 words(OriginalVU0Vec4 v){
    return {std::bit_cast<std::uint32_t>(v.x),std::bit_cast<std::uint32_t>(v.y),
            std::bit_cast<std::uint32_t>(v.z),std::bit_cast<std::uint32_t>(v.w)};
}
int main(){
    const OriginalNavrangeGpr128 a{0x3f800000u,0x40000000u,0x40400000u,0x40800000u};
    const auto packed=originalNavrangePack_359e80(a);
    ck(words(packed.vf1)==a);
    ck(words(packed.vf2)==OriginalNavrangeGpr128({a[1],a[0],a[3],a[2]}));
    ck(words(packed.vf3)==OriginalNavrangeGpr128({a[2],a[3],a[0],a[1]}));
    ck(words(packed.vf4)==OriginalNavrangeGpr128({a[3],a[2],a[1],a[0]}));
    const auto prefix=originalNavrangePrefixFromGpr_359e80(a);
    ck(prefix.vf1.x==2.f&&prefix.vf1.y==4.f&&prefix.vf1.z==6.f&&prefix.vf1.w==8.f);
    ck(prefix.vf2.x==-2.f&&prefix.vf2.y==1.f&&prefix.vf2.z==-4.f&&prefix.vf2.w==3.f);
    ck(prefix.vf3.x==3.f&&prefix.vf3.y==-4.f&&prefix.vf3.z==-1.f&&prefix.vf3.w==2.f);
    ck(prefix.vf4.x==-4.f&&prefix.vf4.y==-3.f&&prefix.vf4.z==2.f&&prefix.vf4.w==1.f);
    ck(prefix.vf5.x==-25.f&&prefix.vf5.y==28.f&&prefix.vf5.z==-10.f&&prefix.vf5.w==0.f);
    const auto m=originalNavrangeQuaternionMatrix_359e74(a);
    ck(m[0].x==-25.f&&m[0].y==28.f&&m[0].z==-10.f&&m[0].w==0.f);
    ck(m[1].x==-20.f&&m[1].y==-19.f&&m[1].z==20.f&&m[1].w==0.f);
    ck(m[2].x==22.f&&m[2].y==4.f&&m[2].z==-9.f&&m[2].w==0.f);
    ck(m[3].x==0.f&&m[3].y==0.f&&m[3].z==0.f&&m[3].w==1.f);
    const auto eye=originalNavrangeQuaternionMatrix_359e74({0u,0u,0u,0x3f800000u});
    ck(eye[0].x==1.f&&eye[0].y==0.f&&eye[0].z==0.f);
    ck(eye[1].x==0.f&&eye[1].y==1.f&&eye[1].z==0.f);
    ck(eye[2].x==0.f&&eye[2].y==0.f&&eye[2].z==1.f&&eye[3].w==1.f);
    const auto q=originalNavrangeQuaternionMatrix_359e74(
        {0u,0u,0x3f3504f3u,0x3f3504f3u});
    ck(q[0].x>-0.001f&&q[0].x<0.001f);
    ck(q[0].y>0.999f&&q[0].y<1.001f);
    ck(q[1].x<-0.999f&&q[1].x>-1.001f);
    ck(q[1].y>-0.001f&&q[1].y<0.001f&&q[2].z==1.f);
    // MMI and QMTC2 copy all word bits, including NaN payloads and -0.
    const OriginalNavrangeGpr128 strange{0x80000000u,0x7fc0a53cu,0u,0xff800000u};
    const auto s=originalNavrangePack_359e80(strange);
    ck(words(s.vf1)==strange);
    ck(words(s.vf2)==OriginalNavrangeGpr128({strange[1],strange[0],strange[3],strange[2]}));
    ck(words(s.vf3)==OriginalNavrangeGpr128({strange[2],strange[3],strange[0],strange[1]}));
    ck(words(s.vf4)==OriginalNavrangeGpr128({strange[3],strange[2],strange[1],strange[0]}));
    for(std::uint32_t i=0;i<1000;++i){
        const OriginalNavrangeGpr128 in{i,i+1,i+2,i+3};
        const auto t=originalNavrangePack_359e80(in);
        ck(words(t.vf1)==in);
        ck(words(t.vf2)==OriginalNavrangeGpr128({in[1],in[0],in[3],in[2]}));
        ck(words(t.vf3)==OriginalNavrangeGpr128({in[2],in[3],in[0],in[1]}));
        ck(words(t.vf4)==OriginalNavrangeGpr128({in[3],in[2],in[1],in[0]}));
    }
}
