#include "ae3/original_movement_child_30c57c.hpp"
#include "ae3/original_movement_scratch_tail_36f6a8.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <utility>

using namespace ae3;
namespace {
void require(bool value,const char* why) {
    if(!value) throw std::runtime_error(why);
}
}
int main() {
    std::array<std::byte,0x110> scratch{};
    for(std::size_t i=0;i<scratch.size();++i)
        scratch[i]=std::byte((i*37u+11u)&255u);
    // Source 0x57C148 call: verified straight-through 0x36F6A8
    // tail writes scalar state into genuine original stack +0x20.
    originalMovementScratchScalarTail_36f6a8(
        scratch,{0x01FFF020u,0,std::bit_cast<std::uint32_t>(0.5f)});
    // Source 0x57C15C: original a0 parent, a1 scratch,
    // a2=1, a3 original controller+0x1d low byte.
    auto child=originalMovementChildCreate_30c57c(
        0x12345000u,scratch,0xFE01u,0x57AAu);
    require(child!=nullptr,"retail 0x30C57C host allocation missing");
    require(child->originalWordAt(0)==0x006B0B20u,
            "original 0x30C57C vtable changed");
    require(child->originalWordAt(4)==0x12345000u,
            "original +4 parent guest VA was lost");
    require(child->originalByteAt(8)==std::byte{1} &&
            child->originalByteAt(9)==std::byte{0xAA},
            "retail original low-byte mode arguments changed");
    for(std::size_t i=0;i<0xB0;++i) {
        require(child->originalByteAt(0x10+i)==scratch[i],
                "retail 0x30C57C 0xB0 byte scratch copy mismatch");
    }
    require(child->originalWordAt(0xB0)==0x00744DD0u,
            "original +0xA0 BSS source pointer not copied");
    require(child->originalWordAt(0xB4)==0x3F800000u,
            "original 0x36F6A8 ordered scalar state not copied");
    require(child->originalWordAt(0xB8)==0 &&
            child->originalWordAt(0xBC)==0,
            "original 0x36F6A8 status/selector not copied");
    require(child->originalByteAt(0xC0)==std::byte{0},
            "retail 0x30C658 terminal flag was not cleared");
    // 0x30C57C does not initialize the holes +0xA..+0xF or
    // trailing bytes +0xC1..+0xCF. Do not invent host zero state.
    for(std::size_t i=0x0A;i<0x10;++i)
        require(!child->originalByteAt(i).has_value(),
                "original unknown header byte was invented");
    for(std::size_t i=0xC1;i<0xD0;++i)
        require(!child->originalByteAt(i).has_value(),
                "original uninitialized tail byte was invented");
    bool rejected=false;
    try{(void)child->originalWordAt(0x0A);}
    catch(const std::logic_error&){rejected=true;}
    require(rejected,"reading unknown source field did not fail");
    rejected=false;
    try{(void)child->originalByteAt(0xD0);}
    catch(const std::out_of_range&){rejected=true;}
    require(rejected,"outside original 0xD0 child accepted");

    // The typed native actor allocation owns its source copy.
    // Releasing/reusing the source 0x110 scratch frame does not
    // invalidate authentic initialized controller child fields.
    scratch.fill(std::byte{0xFF});
    require(child->originalWordAt(0xB0)==0x00744DD0u &&
            child->originalWordAt(0xB4)==0x3F800000u &&
            child->originalWordAt(4)==0x12345000u,
            "retail source child referenced temporary scratch input");

    // C.OLT.S unordered false => 0 at source scratch+0xA4,
    // carried exactly to original object+0xB4.
    scratch.fill(std::byte{0x11});
    originalMovementScratchScalarTail_36f6a8(
        scratch,{0x01FFF020u,0,0x7FC01234u});
    const auto nanCase=originalMovementChildCreate_30c57c(
        0x00123400u,scratch,0xFFFFFFFFu,0u);
    require(nanCase->originalWordAt(0xB4)==0 &&
            nanCase->originalWordAt(0xB0)==0x00744DD0u &&
            nanCase->originalByteAt(8)==std::byte{0xFF},
            "source NaN/low-byte variant changed original child data");

    std::cout<<"ORIGINAL_MOVEMENT_CHILD_30C57C_SOURCE_COPY_PASSED\n";
}
