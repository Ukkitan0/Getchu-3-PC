#include "ae3/original_movement_scratch_tail_36f6a8.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace ae3;
namespace {
void require(bool value,const char* why) {
    if(!value) throw std::runtime_error(why);
}
std::uint32_t word(const std::array<std::byte,0x110>& data,std::size_t off) {
    std::uint32_t result=0;
    for(std::size_t b=0;b<4;++b)
        result|=(std::to_integer<std::uint32_t>(data[off+b])<<(8u*b));
    return result;
}
void unchangedOutsideFields(const std::array<std::byte,0x110>& data) {
    for(std::size_t offset=0;offset<data.size();++offset) {
        if(offset>=0xA0 && offset<0xB0) continue;
        require(data[offset]==std::byte{0xA5},
                "original 0x36F5F4 tail modified unrelated scratch bytes");
    }
}
}
int main() {
    std::array<std::byte,0x110> frame{};
    auto run=[&](std::uint32_t originalF1Bits) {
        frame.fill(std::byte{0xA5});
        originalMovementScratchScalarTail_36f6a8(
            frame,{0x01FFF020u,0,originalF1Bits});
        require(word(frame,0xA0)==0x00744DD0u,
                "original 0x36F694 source guest-BSS VA changed");
        require(word(frame,0xA8)==0 && word(frame,0xAC)==0,
                "original t0-zero branch selector fields changed");
        unchangedOutsideFields(frame);
    };
    run(std::bit_cast<std::uint32_t>(0.5f));
    require(word(frame,0xA4)==0x3F800000u,
            "source 0x36F6C8 C.OLT.S true must write 1.0f");
    run(std::bit_cast<std::uint32_t>(1.0f));
    require(word(frame,0xA4)==0,
            "source strict C.OLT.S must reject equality to 1.0f");
    run(std::bit_cast<std::uint32_t>(2.0f));
    require(word(frame,0xA4)==0,
            "source C.OLT.S false must leave zero status");
    run(0x7FC01234u);
    require(word(frame,0xA4)==0,
            "ordered source C.OLT.S must be false for NaN");
    run(0xFF800000u);
    require(word(frame,0xA4)==0x3F800000u,
            "original negative infinity is less than source 1.0f");
    run(0x00000000u);
    require(word(frame,0xA4)==0x3F800000u,
            "original source +0 is below +1");
    run(0x80000000u);
    require(word(frame,0xA4)==0x3F800000u,
            "original source -0 is below +1");

    // The source 0x57C148 callsite sets t0 zero. Other original
    // paths must not silently take the t0-zero translated tail.
    auto reject=[&](const OriginalMovementTailSource_36f6a8& src) {
        frame.fill(std::byte{0xA5});
        bool threw=false;
        try{originalMovementScratchScalarTail_36f6a8(frame,src);}
        catch(const std::invalid_argument&){threw=true;}
        require(threw,"unsupported original callsite was accepted");
        for(const auto byte:frame)
            require(byte==std::byte{0xA5},
                    "unsupported original callsite mutated scratch");
    };
    reject({0x01FFF020u,1,0});
    reject({0x100000000ull|0x01FFF020u,0,0});
    reject({0x01FFF021u,0,0});
    reject({0,0,0});
    std::cout<<"ORIGINAL_MOVEMENT_36F6A8_SOURCE_TAIL_PASSED\n";
}
