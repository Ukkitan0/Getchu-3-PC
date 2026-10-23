#include "ae3/original_manager_type2_update_57c554.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
namespace {
struct OriginalCalls final:OriginalManagerType2Calls_57c554 {
    int allocated=0,deleted=0,updated=0,compared=0;
    int previous=1,created=2;
    bool installPreviousDuringCreate=false;
    bool missingVectorOperands=false;
    bool equal=true;
    std::uint32_t updateResult=1;
    std::uint32_t actualOriginalA1=0;
    std::vector<std::string> events;
    void* call_original_30c360(OriginalManagerChild70& child) override {
        ++allocated;events.emplace_back("30c360");
        assert(child.flag1d==0x51);
        if (installPreviousDuringCreate)
            child.child10=&previous;
        return &created;
    }
    void call_original_child_vtable1_delete(void* child) override {
        ++deleted;events.emplace_back("child-delete");
        assert(child==&previous);
    }
    std::uint32_t call_original_3161b8(
        void*& sourceChild10,std::uint32_t sourceA1) override {
        ++updated;events.emplace_back("3161b8");
        actualOriginalA1=sourceA1;
        assert(sourceChild10==&created || sourceChild10==&previous);
        return updateResult;
    }
    bool call_original_variant_vector_equal_57c5d4(
        const OriginalManagerChild70&) override {
        ++compared;events.emplace_back("vu-compare");
        if (missingVectorOperands)
            throw std::runtime_error("original 0x57C554 VU operands missing");
        return equal;
    }
};
}
int main() {
    OriginalManagerChild70 controller{};
    OriginalCalls original{};
    controller.flag1d=0x51;
    controller.word14=0x1234;

    // Original 0x57C570 null child creates the real 0x30C360 child,
    // then calls 0x3161B8, returning 1 for variant-zero.
    assert(originalManagerType2Update_57c554(
               controller,0xABCD,original)==1);
    assert(controller.child10==&original.created &&
           original.actualOriginalA1==0xABCD &&
           original.allocated==1 && original.updated==1 &&
           original.deleted==0 && original.compared==0);
    assert(original.events==std::vector<std::string>({"30c360","3161b8"}));

    // A preexisting original child is updated without creating another.
    original.events.clear();
    assert(originalManagerType2Update_57c554(
               controller,0x55AA,original)==1);
    assert(original.allocated==1 && original.updated==2 &&
           original.actualOriginalA1==0x55AA);
    assert(original.events==std::vector<std::string>{"3161b8"});

    // 0x3161B8 false result terminates with ZERO; no VU comparison.
    original.events.clear();original.updateResult=0;
    controller.alternative30=1;
    assert(originalManagerType2Update_57c554(
               controller,0,original)==0);
    assert(original.compared==0 && original.events==
        std::vector<std::string>{"3161b8"});

    // Variant-one returned source controller+0x14 on VU mismatch.
    original.events.clear();original.updateResult=1;
    original.equal=false;
    assert(originalManagerType2Update_57c554(
               controller,0x17,original)==0x1234);
    assert(original.compared==1 && original.events==
        std::vector<std::string>({"3161b8","vu-compare"}));
    original.equal=true;original.events.clear();
    assert(originalManagerType2Update_57c554(
               controller,0x18,original)==1);
    assert(original.compared==2);

    // Original construction can synchronously publish an old child
    // before returning a new one; source deletes old first, then
    // stores new pointer and calls 0x3161B8.
    controller.child10=nullptr;
    original.installPreviousDuringCreate=true;
    controller.alternative30=0;
    original.events.clear();
    assert(originalManagerType2Update_57c554(
               controller,1,original)==1);
    assert(controller.child10==&original.created &&
           original.deleted==1 &&
           original.events==std::vector<std::string>(
                {"30c360","child-delete","3161b8"}));

    // Missing original packed VU comparison must not claim success.
    original.missingVectorOperands=true;
    controller.alternative30=1;
    bool rejected=false;
    try {(void)originalManagerType2Update_57c554(
             controller,0,original);}
    catch (const std::runtime_error&) {rejected=true;}
    assert(rejected && controller.child10==&original.created);
    std::cout<<"ORIGINAL_TYPE2_57C554_SOURCE_CONTROL_FLOW_PASSED\n";
}
