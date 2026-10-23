#include "ae3/original_movement_vu0_scalar_36f5f4.hpp"
#include "ae3/original_movement_child_30c57c.hpp"
#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace ae3;

namespace {
void put(std::array<std::byte,16>& bytes,unsigned lane,float value) {
    auto raw=std::bit_cast<std::uint32_t>(value);
    for(unsigned i=0;i<4;++i)
        bytes[4*lane+i]=std::byte((raw>>(8*i))&0xffu);
}
float asFloat(std::uint32_t raw){return std::bit_cast<float>(raw);}
}
int main() {
    OriginalMovementVU0ScalarSource_36f5f4 original{};
    put(original.originalA1,0,1.0f);
    put(original.originalA1,1,2.0f);
    put(original.originalA1,2,3.0f);
    put(original.originalA1,3,4.0f);
    put(original.originalA2,0,3.0f);
    put(original.originalA2,1,2.0f);
    put(original.originalA2,2,3.0f);
    put(original.originalA2,3,4.0f);
    put(original.originalA3,0,1.5f);
    put(original.originalA3,1,2.0f);
    put(original.originalA3,2,3.0f);
    put(original.originalA3,3,4.0f);

    std::array<std::byte,0x110> originalScratch{};
    originalScratch.fill(std::byte{0xA5});
    std::bitset<0x110> known{};
    // One independent source byte known before this function.
    originalScratch[5]=std::byte{0x34};
    known.set(5);
    const auto frame=originalMovementVU0FirstReturn_36f5f4(
        originalScratch,known,0x01FFF020ull,original);
    assert(asFloat(frame.scalar.originalFirstLengthSquaredBits)==4.0f);
    assert(asFloat(frame.scalar.originalSecondLengthSquaredBits)==0.25f);
    assert(!frame.originalFirstLengthUnderOne);
    assert(asFloat(frame.originalVF4[0])==3.0f);
    assert(asFloat(frame.originalVF3[0])==2.0f);
    assert(asFloat(frame.originalVF1[0])==-1.5f);
    assert(asFloat(frame.originalVF2[0])==-1.5f);

    for(std::size_t i=0;i<known.size();++i) {
        assert(known.test(i)==(i==5||(i>=0xA0 && i<0xB0)));
        if(i!=5 && (i<0xA0||i>=0xB0))
            assert(originalScratch[i]==std::byte{0xA5});
    }
    auto child=originalMovementChildCreate_30c57c(
        0x00543210u,originalScratch,known,1,0x18);
    assert(child->originalWordAt(0)==0x006B0B20u);
    assert(child->originalWordAt(4)==0x00543210u);
    assert(child->originalByteAt(8)==std::byte{1});
    assert(child->originalByteAt(9)==std::byte{0x18});
    assert(child->originalByteAt(0x15)==std::byte{0x34});
    // The source actually copies unknown stack bytes, but their
    // values cannot be trusted as real original runtime state.
    assert(!child->originalByteAt(0x10));
    assert(!child->originalByteAt(0x11));
    assert(!child->originalByteAt(0xAF));
    assert(child->originalWordAt(0xB0)==0x00744DD0u);
    assert(child->originalWordAt(0xB4)==0x3F800000u);
    assert(child->originalWordAt(0xB8)==0u);
    assert(child->originalWordAt(0xBC)==0u);
    assert(child->originalByteAt(0xC0)==std::byte{0});
    assert(!child->originalByteAt(0xC1));
    bool rejected=false;
    try{(void)child->originalWordAt(0x10);}
    catch(const std::logic_error&){rejected=true;}
    assert(rejected && "unknown original scratch cannot be read as a source word");

    // Previously known real source bytes survive later original
    // scratch writes. The child owns the copied proven bytes.
    originalScratch.fill(std::byte{0xFF});
    known.reset();
    assert(child->originalWordAt(0xB0)==0x00744DD0u &&
           child->originalByteAt(0x15)==std::byte{0x34});

    // The alternate real source midpoint branch runs from a
    // complete VU0 frame and uses no fake vector status.
    put(original.originalA2,0,1.5f);
    originalScratch.fill(std::byte{0xCC});
    known.reset();
    const auto middle=originalMovementVU0FirstReturn_36f5f4(
        originalScratch,known,0x01FFF020ull,original);
    assert(middle.originalFirstLengthUnderOne);
    assert(asFloat(middle.originalVF4[0])==1.25f);
    assert(asFloat(middle.originalVF1[0])==0.25f);
    assert(asFloat(middle.originalVF3[0])==0.25f);
    assert(known.count()==0x10);

    // W is consumed by original VF register side effects.
    // Invalid PS2 edge values MUST NOT partially update scratch.
    put(original.originalA3,3,std::numeric_limits<float>::quiet_NaN());
    originalScratch.fill(std::byte{0x5A});
    known.reset();
    known.set(1);
    rejected=false;
    try{(void)originalMovementVU0FirstReturn_36f5f4(
        originalScratch,known,0x01FFF020ull,original);}
    catch(const std::invalid_argument&){rejected=true;}
    assert(rejected && known.count()==1 && known.test(1));
    for(auto byte:originalScratch) assert(byte==std::byte{0x5A});
    put(original.originalA3,3,4.0f);
    rejected=false;
    try{(void)originalMovementVU0FirstReturn_36f5f4(
        originalScratch,known,0x10001FFF020ull,original);}
    catch(const std::invalid_argument&){rejected=true;}
    assert(rejected && known.count()==1 && known.test(1));
    for(auto byte:originalScratch) assert(byte==std::byte{0x5A});
    std::cout<<"ORIGINAL_36F5F4_FIRST_RETURN_PROVENANCE_PASSED\n";
}
