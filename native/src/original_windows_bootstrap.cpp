#include "ae3/original_windows_bootstrap.hpp"
#include <bit>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace ae3::runtime {
namespace {
// Authenticated USA ELF SHA-256:
// 5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63
// 0x428AB8..0x428B0C: at 0x428AD4 load 0x62E89C; JALR in loop;
// decrement pointer by 4; stop when 0xFFFFFFFF sentinel at 0x62E660.
// Original 143 addresses are listed in EXACT native dispatch order.
// This is source metadata, not original executable code/data bytes.
constexpr std::array<std::uint32_t,kOriginalRetailConstructorCount_428ab8>
kOriginalConstructors = {
    0x41a390, 0x417c58, 0x3a6cd0, 0x39fc78, 0x391600, 0x38abfc, 0x38aa18, 
    0x384ec0, 0x383644, 0x3835b0, 0x3828a4, 0x382580, 0x38157c, 0x37c5b0, 
    0x377a94, 0x377934, 0x37530c, 0x373aac, 0x372074, 0x370628, 0x3703a8, 
    0x36f5d8, 0x36d9dc, 0x36bba8, 0x368bf8, 0x367148, 0x365db4, 0x3640d0, 
    0x35f420, 0x358160, 0x350384, 0x349f80, 0x3357fc, 0x3346c8, 0x3304a0, 
    0x329c98, 0x326448, 0x322f34, 0x321974, 0x31751c, 0x31604c, 0x30c084, 
    0x2fbc9c, 0x2f7a3c, 0x2f11fc, 0x2eb3a8, 0x2e3470, 0x2df124, 0x2d7d4c, 
    0x2c9f60, 0x2c47f4, 0x2c3f6c, 0x2bb55c, 0x2ba0bc, 0x2b4384, 0x2a1b84, 
    0x2a1aa8, 0x29d8dc, 0x29c9a0, 0x292ecc, 0x29150c, 0x28ad3c, 0x288420, 
    0x2852a4, 0x283d14, 0x2788f8, 0x2724b0, 0x271d9c, 0x26f984, 0x26f084, 
    0x26781c, 0x255bbc, 0x24b7dc, 0x245b40, 0x23ed50, 0x234e8c, 0x22e1c8, 
    0x22c7d8, 0x2205dc, 0x21ae88, 0x20c290, 0x20ad44, 0x20a6bc, 0x20a39c, 
    0x208718, 0x207a18, 0x207350, 0x1f308c, 0x1f2100, 0x1f11f4, 0x1ee9d8, 
    0x1ea6f0, 0x1ea378, 0x1e2314, 0x1dfdf0, 0x1de620, 0x1bb3d0, 0x1b3500, 
    0x1b30f8, 0x1ae568, 0x1a7260, 0x1a6214, 0x1a5924, 0x1a20ac, 0x19db20, 
    0x1990cc, 0x195e34, 0x194770, 0x1908e4, 0x18bc2c, 0x18b4ac, 0x188c98, 
    0x184504, 0x17c260, 0x1727bc, 0x16f54c, 0x16dec4, 0x1680b0, 0x15aa88, 
    0x15a46c, 0x159570, 0x1516e4, 0x14baec, 0x14a4f4, 0x146398, 0x13b098, 
    0x133be4, 0x130f9c, 0x126fc8, 0x118884, 0x1165d4, 0x114e44, 0x114978, 
    0x113fb8, 0x113c78, 0x113464, 0x112ca8, 0x11169c, 0x111428, 0x110dc4, 
    0x10d9cc, 0x10c4a4, 0x10526c
};
} // namespace
const std::array<std::uint32_t,kOriginalRetailConstructorCount_428ab8>&
originalConstructorOrder_428ab8() noexcept {return kOriginalConstructors;}

OriginalConstructorProgress originalRunConstructors_428ab8(
    OriginalNativeConstructorCalls& dispatch){
    OriginalConstructorProgress result{};
    for(const auto originalVA : kOriginalConstructors){
        // Must fail closed on first untranslated native target.
        if(!dispatch.callRecoveredConstructor(originalVA)){
            result.firstUnrecovered=originalVA;
            return result;
        }
        ++result.executed;
    }
    result.complete=true;
    return result;
}
void originalAllocatorCtor_41a390(OriginalAllocatorGlobals_41a390& globals) noexcept {
    // 0x41A390 passes original a0=1, a1=65535 to 0x41A088.
    // All twelve branches increment their own original global uint64.
    // A first transition to 1 calls genuine 0x4173B0 which is JR RA/NOP,
    // so the complete observable native result is exactly increment-by-one.
    for(auto& value:globals.counters)++value;
}
bool originalNumericCtor_417c58(OriginalNumericGlobals_417c58& out) noexcept {
    // Source 0x417C58 tail-jumps to 0x417B50(1,0xFFFF).
    // 0x417B84 and 0x417BA8 invoke original no-op 0x4173B0.
    // 0x417BC8/0x417BD4: 0x1009 << 50 = IEEE double 10, then 16.
    // 0x417C10/0x417C1C: the same 10 and 0x201 << 53 = double 8.
    // Original 0x41D6B8 -> 0x41EFD0 performs double square root;
    // original 0x4240D8 performs the subsequent float64 multiplication.
    // This is a fixed-input native translation of the two real global
    // initializers only, NOT an implementation of generic 41EFD0/4240D8.
    if(!std::numeric_limits<double>::is_iec559 || sizeof(double)!=8)
        return false;
    const double a=std::bit_cast<double>(UINT64_C(0x4024000000000000));
    const double b=std::bit_cast<double>(UINT64_C(0x4030000000000000));
    const double d=std::bit_cast<double>(UINT64_C(0x4020000000000000));
    const double sqrtA=std::sqrt(a);
    const double sqrtB=std::sqrt(b);
    const double first=sqrtA*sqrtB;
    const double sqrtD=std::sqrt(d);
    const double second=sqrtA*sqrtD;
    const std::uint64_t firstBits=std::bit_cast<std::uint64_t>(first);
    const std::uint64_t secondBits=std::bit_cast<std::uint64_t>(second);
    // Fail closed if the host floating implementation doesn't produce the
    // required correctly-rounded IEEE results for the source constants.
    // The real retail numeric caller never accepts approximate substitutes.
    if(firstBits!=UINT64_C(0x40294c583ada5b53) ||
       secondBits!=UINT64_C(0x4021e3779b97f4a8))
        return false;
    out.first_69e678=firstBits;
    out.second_69e680=secondBits;
    out.initialized=true;
    return true;
}
bool OriginalNativeTerminationRegistry::register_428b80(Callback callback){
    if(!callback)return false;
    // 0x428B94..0x428BF8: each original block has 32 four-byte callback
    // slots at +8, count at +4 and previous block at +0.
    if(blocks_.empty()||blocks_.back().size()==32)
        blocks_.emplace_back();
    blocks_.back().push_back(std::move(callback));
    return true;
}
std::size_t OriginalNativeTerminationRegistry::count() const noexcept {
    std::size_t total=0;
    for(const auto& block:blocks_)total+=block.size();
    return total;
}
void OriginalNativeTerminationRegistry::runRegisteredCallbacks_428cb8(){
    // Caller is responsible for ensuring callbacks are the game's actual
    // recovered destructor methods and that all global-owner teardown
    // dependencies are available. This translates only the source loop.
    while(!blocks_.empty()){
        auto& block=blocks_.back();
        if(block.empty()){blocks_.pop_back();continue;}
        auto callback=std::move(block.back());
        block.pop_back();
        callback();
    }
}
} // namespace ae3::runtime
