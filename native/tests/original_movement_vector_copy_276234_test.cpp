#include "ae3/original_movement_vector_copy_276234.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
using namespace ae3;
namespace {
struct OriginalStore final:OriginalMovementVectorStoreOwner_276234 {
    std::uint64_t lastGuestVA{};
    std::array<std::uint32_t,4> lastVector{};
    int stores=0;
    bool fail=false;
    void store_original_128bit_at_guest(
        std::uint64_t addr,const std::array<std::uint32_t,4>& lanes) override {
        ++stores;
        lastGuestVA=addr;
        lastVector=lanes;
        if(fail)throw std::runtime_error("original game guest store unavailable");
    }
};
}
int main() {
    OriginalStore owner;
    const std::array<std::uint32_t,4> full{
        0x3f800000u,0xbf800000u,0x12345678u,0xfedcba98u};
    // Genuine 30C114 and 30C14C JAL source copies full
    // 128-bit input to parent+0x40 (not to a host pointer).
    originalMovementVectorCopy_276234(0x340000ull,full,owner);
    assert(owner.stores==1 && owner.lastGuestVA==0x340040ull &&
           owner.lastVector==full);
    // 64-bit original guest address arithmetic and full lanes.
    originalMovementVectorCopy_276234(
        0xffffffff81234000ull,full,owner);
    assert(owner.stores==2 &&
           owner.lastGuestVA==0xffffffff81234040ull &&
           owner.lastVector==full);
    // SQ offset uses full-width EE effective-address addition.
    originalMovementVectorCopy_276234(
        0xffffffffffffffd0ull,full,owner);
    assert(owner.stores==3 && owner.lastGuestVA==0x10ull);
    // An invalid source SQ alignment cannot claim game state
    // was written successfully.
    bool refused=false;
    try {originalMovementVectorCopy_276234(
        0x340004ull,full,owner);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused && owner.stores==3);
    // The owner preserves real game memory and exceptions.
    owner.fail=true;
    refused=false;
    try {originalMovementVectorCopy_276234(
        0x340000ull,full,owner);}
    catch(const std::runtime_error&){refused=true;}
    assert(refused && owner.stores==4);
    std::cout<<"ORIGINAL_276234_SOURCE_128BIT_COPY_PASSED\n";
}
