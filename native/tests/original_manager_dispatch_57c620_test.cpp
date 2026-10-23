#include "ae3/original_manager_dispatch_57c620.hpp"
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
struct OriginalOwner final:OriginalManagerDispatchCalls_57c620 {
    std::vector<std::string> sequence;
    std::uint32_t dispatchReturn=1;
    std::uint32_t originalMethod=0;
    std::int32_t originalThisOffset=0;
    void* parent=nullptr;
    void* pending=nullptr;
    bool throwCleanup=false,throwWarning=false;
    std::uint8_t originalE7=0,originalE0=0;
    std::uint32_t call_original_direct_57c7ac(
        std::uint32_t sourceVA,std::int32_t sourceOffset) override {
        sequence.emplace_back("direct");
        originalMethod=sourceVA;
        originalThisOffset=sourceOffset;
        return dispatchReturn;
    }
    std::uint32_t call_original_virtual_57c79c(
        std::uint32_t sourceVtableOffset,std::int32_t sourceOffset) override {
        sequence.emplace_back("virtual");
        originalMethod=sourceVtableOffset;
        originalThisOffset=sourceOffset;
        return dispatchReturn;
    }
    void call_original_30ffd0(void* originalParent,void* originalPending) override {
        sequence.emplace_back("assign30ffd0");
        parent=originalParent;
        pending=originalPending;
        if(throwCleanup)throw std::runtime_error("original polygon update is unavailable");
    }
    std::uint8_t read_original_6354e7() override {
        sequence.emplace_back("e7");
        return originalE7;
    }
    std::uint8_t read_original_6354e0() override {
        sequence.emplace_back("e0");
        return originalE0;
    }
    void call_original_2478ec_247d84_warning() override {
        sequence.emplace_back("warning");
        if(throwWarning)throw std::runtime_error("original diagnostics unavailable");
    }
};
void equals(const std::vector<std::string>& got,
            std::initializer_list<std::string> expected) {
    assert(got==std::vector<std::string>(expected));
}
}
int main() {
    OriginalManagerChild70 c{};
    OriginalOwner owner{};
    int parent=1, pending=2, child=3;
    c.parent04=&parent;
    c.originalPendingPolygon50=&pending;

    // Genuine source comparison with 0x708A50/54 (both ZERO):
    // method+8 is zero => return 2, irrespective of method+0xC.
    c.word08=0;c.word0c=0xdeadbeefu;
    assert(originalManagerDispatch_57c620(c,0,owner)==2);
    assert(c.originalPendingPolygon50==&pending);
    assert(owner.sequence.empty());

    // Source a1 flags bit0 or bit1 bypass polymorphic dispatch, return1.
    c.word08=0x57bf5c;c.word0c=0;
    assert(originalManagerDispatch_57c620(c,1,owner)==1);
    equals(owner.sequence,{"assign30ffd0"});
    assert(owner.parent==&parent && owner.pending==&pending);
    assert(c.originalPendingPolygon50==nullptr);
    owner.sequence.clear();
    c.originalPendingPolygon50=&pending;
    assert(originalManagerDispatch_57c620(c,2,owner)==1);
    equals(owner.sequence,{"assign30ffd0"});
    assert(c.originalPendingPolygon50==nullptr);

    // With no flag bits and no child, controller method+8 is
    // DIRECT source JALR; method+0xC encodes signed owner offset.
    owner.sequence.clear();
    c.word0c=0xfffffffcu; // original SRA1 -> -2
    owner.dispatchReturn=0;
    assert(originalManagerDispatch_57c620(c,0,owner)==0);
    equals(owner.sequence,{"direct"});
    assert(owner.originalMethod==0x57bf5c &&
           owner.originalThisOffset==-2);

    // When child+0x10 exists, dispatch occurs EVEN with input bit0.
    // Tagged low bit of method+0xC selects original vtable method.
    owner.sequence.clear();
    c.child10=&child;
    c.word08=0x57c557;
    c.word0c=0x15; // signed source this-offset = 10, tagged virtual
    owner.dispatchReturn=3;
    c.originalPendingPolygon50=&pending;
    assert(originalManagerDispatch_57c620(c,1,owner)==3);
    equals(owner.sequence,{"virtual","assign30ffd0"});
    assert(owner.originalMethod==0x57c554 &&
           owner.originalThisOffset==10 &&
           c.originalPendingPolygon50==nullptr);
    c.child10=nullptr;

    // Returning zero leaves original pending polygon+0x50 owned, then checks source
    // globals; global E0 must be read ONLY when E7 is nonzero.
    owner.sequence.clear();
    c.word08=0x57bf5c;c.word0c=0;
    c.originalPendingPolygon50=&pending;
    owner.dispatchReturn=0;
    owner.originalE7=0;
    owner.originalE0=0;
    assert(originalManagerDispatch_57c620(c,0,owner)==0);
    equals(owner.sequence,{"direct","e7"});
    assert(c.originalPendingPolygon50==&pending);

    owner.sequence.clear();
    owner.originalE7=1;owner.originalE0=1;
    assert(originalManagerDispatch_57c620(c,0,owner)==0);
    equals(owner.sequence,{"direct","e7","e0"});

    owner.sequence.clear();
    owner.originalE7=1;owner.originalE0=0;
    assert(originalManagerDispatch_57c620(c,0,owner)==0);
    equals(owner.sequence,{"direct","e7","e0","warning"});
    assert(c.originalPendingPolygon50==&pending);

    // Fail closed if real original diagnostic or spatial polygon update
    // callback cannot execute. Never silently publish a fake update.
    owner.sequence.clear();owner.throwWarning=true;
    bool refused=false;
    try {(void)originalManagerDispatch_57c620(c,0,owner);}
    catch(const std::runtime_error&) {refused=true;}
    assert(refused && c.originalPendingPolygon50==&pending);
    owner.throwWarning=false;
    owner.sequence.clear();owner.throwCleanup=true;
    owner.dispatchReturn=1;
    refused=false;
    try {(void)originalManagerDispatch_57c620(c,0,owner);}
    catch(const std::runtime_error&) {refused=true;}
    assert(refused && c.originalPendingPolygon50==&pending);

    std::cout<<"ORIGINAL_57C620_CONTROLLER_DISPATCH_PASSED\n";
}
