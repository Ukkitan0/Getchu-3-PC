#include "ae3/original_manager_packed_compare.hpp"
#include "ae3/original_manager_type2_update_57c554.hpp"
#include "ae3/original_manager_followup_57c48c.hpp"
#include "ae3/original_manager_movement_57bf5c.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>

using namespace ae3;
namespace {
void require(bool value,const char* reason) {
    if (!value) throw std::runtime_error(reason);
}
void setWord(std::array<std::byte,16>& bytes,std::size_t lane,
             std::uint32_t bits) {
    require(lane<4,"source PCEQW lane outside genuine SQ");
    for (std::size_t i=0;i<4;++i)
        bytes[lane*4+i]=std::byte((bits>>(8*i))&0xffu);
}
struct Type2 final:OriginalManagerType2Calls_57c554 {
    void* call_original_30c360(OriginalManagerChild70&) override {
        throw std::runtime_error("unexpected original type2 creation");
    }
    void call_original_child_vtable1_delete(void*) override {
        throw std::runtime_error("unexpected source child destruction");
    }
    std::uint32_t call_original_3161b8(void*&,std::uint32_t) override {
        return 1;
    }
    // PCEQW not overridden: the actual native original packed
    // byte-lane comparator must be used by default.
};
struct Followup final:OriginalManagerFollowupCalls_57c48c {
    void* call_original_30c520(OriginalManagerChild70&) override {
        throw std::runtime_error("unexpected followup creation");
    }
    void call_original_child_vtable1_delete(void*) override {
        throw std::runtime_error("unexpected source child deletion");
    }
    std::uint32_t call_original_3161b8(void*&,std::uint32_t) override {
        return 1;
    }
};
struct Movement final:OriginalManagerMovementCalls_57bf5c {
    float call_original_vu0_direction_57bfac(OriginalManagerChild70&) override {
        throw std::runtime_error("unexpected missing direction owner");
    }
    void call_original_21b854(OriginalManagerChild70&) override {
        throw std::runtime_error("unexpected projection");
    }
    float call_original_vu0_metric_57c090(OriginalManagerChild70&) override {
        throw std::runtime_error("unexpected metric");
    }
    void call_original_36f5f4(OriginalMovementScratch_57bf5c&) override {
        throw std::runtime_error("unexpected scratch owner");
    }
    void* call_original_30c57c(
        OriginalManagerChild70&,const OriginalMovementScratch_57bf5c&) override {
        throw std::runtime_error("unexpected child constructor");
    }
    void call_original_child_vtable1_delete(void*) override {
        throw std::runtime_error("unexpected deletion");
    }
    std::uint32_t call_original_3161b8(void*&,std::uint32_t) override {
        return 1;
    }
    std::uint32_t call_original_316174(
        OriginalManagerChild70&,std::uint32_t) override {
        throw std::runtime_error("unexpected source next-method dispatch");
    }
    std::uint8_t read_original_6354e7() override {return 0;}
    std::uint8_t read_original_6354e0() override {return 0;}
    void call_original_21db64(OriginalManagerChild70&) override {
        throw std::runtime_error("unexpected diagnostic");
    }
};
}
int main() {
    std::array<std::byte,16> original20{},original40{};
    const std::array<std::uint32_t,4> originalWords{
        0x00000000u,0x80000000u,0x7fc01234u,0xffffffffu};
    for(std::size_t lane=0;lane<4;++lane) {
        setWord(original20,lane,originalWords[lane]);
        setWord(original40,lane,originalWords[lane]);
    }
    require(originalManagerPackedPceqwEqual_57c1e0(original20,original40),
            "four original PCEQW lanes must match as raw bits");
    for(std::size_t byte=0;byte<16;++byte) {
        original40[byte]^=std::byte{1};
        require(!originalManagerPackedPceqwEqual_57c1e0(original20,original40),
                "each source PCEQW byte must affect equality");
        original40[byte]^=std::byte{1};
    }
    // IEEE754 numeric float equality would falsely equate signed
    // zeroes and treat identical NaN bit patterns as unequal.
    setWord(original40,0,0x80000000u);
    require(!originalManagerPackedPceqwEqual_57c1e0(original20,original40),
            "signed zero must retain original PCEQW raw-lane semantics");
    setWord(original40,0,0);
    require(originalManagerPackedPceqwEqual_57c1e0(original20,original40),
            "matching source NaN payload must compare as exact bits");

    OriginalManagerChild70 child{};
    bool rejected=false;
    try {(void)originalManagerPackedPceqwEqual_57c1e0(child);}
    catch(const std::invalid_argument&){rejected=true;}
    require(rejected,"uninitialized source controller must reject");
    child.controller20=original20;
    rejected=false;
    try {(void)originalManagerPackedPceqwEqual_57c1e0(child);}
    catch(const std::invalid_argument&){rejected=true;}
    require(rejected,"missing original controller +0x40 must reject");
    child.controller40=original40;
    int genuineChild=1;
    child.child10=&genuineChild;
    child.alternative30=1;
    child.word14=0xabcdu;
    Type2 type2{};
    Followup followup{};
    require(originalManagerType2Update_57c554(child,0,type2)==0xabcdu,
            "original type2 must use default native PCEQW result");
    require(originalManagerFollowupUpdate_57c48c(child,0,followup)==0xabcdu,
            "original followup must use default native PCEQW result");
    child.word14=2;child.flag1c=0;
    Movement movement{};
    require(originalManagerMovementUpdate_57bf5c(child,0,movement)==2,
            "original movement must use exact packed SQ equality");
    // A one-byte change affects all three original MMI sites.
    (*child.controller40)[15]^=std::byte{1};
    child.word14=0xabcdu;
    require(originalManagerType2Update_57c554(child,0,type2)==1,
            "original type2 must return one on packed SQ mismatch");
    require(originalManagerFollowupUpdate_57c48c(child,0,followup)==1,
            "original followup must return one on packed SQ mismatch");
    child.word14=2;
    require(originalManagerMovementUpdate_57bf5c(child,0,movement)==1,
            "original movement must return one on packed SQ mismatch");
    child.controller40.reset();
    rejected=false;
    try {(void)originalManagerType2Update_57c554(child,0,type2);}
    catch(const std::invalid_argument&){rejected=true;}
    require(rejected && child.child10==&genuineChild,
            "missing source SQ must reject without manufacturing child state");
    std::cout<<"ORIGINAL_PACKED_PCEQW_CONTROLLER_SOURCE_PASSED\n";
    return 0;
}
