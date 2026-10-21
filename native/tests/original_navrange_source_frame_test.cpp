#include "ae3/original_navrange_source_frame.hpp"
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <stdexcept>
using namespace ae3;
static void ck(bool ok){if(!ok)throw std::runtime_error("navrange original source frame failed");}
static void w32(std::span<std::byte> bytes,std::size_t at,std::uint32_t word){
    for(unsigned i=0;i<4;++i)bytes[at+i]=std::byte((word>>(i*8))&255u);
}
int main(){
    std::array<std::byte,0x60> header{};
    for(unsigned i=0;i<4;++i){
        w32(header,0x20+i*4,0x3f000000u+i);
        w32(header,0x30+i*4,0u);
        w32(header,0x40+i*4,0x80000000u+i);
    }
    w32(header,0x3c,0x3f800000u);
    const auto frame=originalNavrangeSourceFrame_4e4110(header);
    for(unsigned i=0;i<4;i++){
        ck(frame.argumentA0_20[i]==0x3f000000u+i);
        ck(frame.argumentA1_40[i]==0x80000000u+i);
    }
    ck(frame.quaternionA2_30[0]==0u&&frame.quaternionA2_30[3]==0x3f800000u);
    const auto inv=originalNavrangeFrameInverse_21ef9c_21efdc(frame);
    ck(inv[0].x==1.f&&inv[1].y==1.f&&inv[2].z==1.f&&inv[3].w==1.f);
    ck(inv[0].y==0.f&&inv[1].z==0.f&&inv[2].x==0.f);
    w32(header,0x20,0x7fc01234u);
    w32(header,0x24,0x80000000u);
    w32(header,0x44,0xff800000u);
    const auto special=originalNavrangeSourceFrame_4e4110(header);
    ck(special.argumentA0_20[0]==0x7fc01234u);
    ck(special.argumentA0_20[1]==0x80000000u);
    ck(special.argumentA1_40[1]==0xff800000u);
    bool rejected=false;
    try{(void)originalNavrangeSourceFrame_4e4110(std::span<const std::byte>(header).first(0x4f));}
    catch(const std::out_of_range&){rejected=true;}
    ck(rejected);
    for(unsigned n=0;n<0x100u;++n){
        for(unsigned c=0;c<3;++c){
            const unsigned off=0x20+c*16;
            for(unsigned j=0;j<4;++j)w32(header,off+j*4,0xabcdef00u+n*17u+c*4u+j);
        }
        const auto row=originalNavrangeSourceFrame_4e4110(header);
        for(unsigned j=0;j<4;j++){
            ck(row.argumentA0_20[j]==0xabcdef00u+n*17u+j);
            ck(row.quaternionA2_30[j]==0xabcdef00u+n*17u+4u+j);
            ck(row.argumentA1_40[j]==0xabcdef00u+n*17u+8u+j);
        }
    }
}
