#include "ae3/original_manager_update_316174.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace ae3;
struct OriginalOwner:OriginalManagerUpdateOwner_316174 {
    int existing=12,replacement=13,nested=14;
    int updates=0,deletions=0;
    bool replaceInUpdate=false,triggerNested=false,failMethod=false;
    bool failDelete=false;
    std::uint32_t originalResult=0,lastA1=0;
    OriginalManagerChild70* controller=nullptr;
    OriginalUpdateDepth_696c98* depth=nullptr;
    std::vector<std::uint32_t> observedDepth;
    std::uint32_t call_original_vtable5(
        void* obj,std::uint32_t a1) override {
        assert(obj==&existing || obj==&nested);
        ++updates;lastA1=a1;
        assert(depth);
        observedDepth.push_back(depth->depth);
        if(failMethod)throw std::runtime_error("unresolved source vtable[5]");
        if(triggerNested && obj==&existing) {
            assert(originalManagerGuardedUpdate_316174(
                       &nested,0xCAFE,*depth,*this)==originalResult);
            assert(depth->depth==1);
        }
        if(replaceInUpdate && obj==&existing)controller->child10=&replacement;
        return originalResult;
    }
    void call_original_child_vtable1_delete(void* obj) override {
        assert(obj==&existing || obj==&replacement);
        ++deletions;
        if(failDelete)throw std::runtime_error("unresolved deleting destructor");
    }
};
int main() {
    OriginalUpdateDepth_696c98 depth{};
    OriginalManagerChild70 c{};
    OriginalOwner owner{};
    owner.controller=&c;owner.depth=&depth;
    c.child10=&owner.existing;
    c.word14=0xDADA;
    // Original 0x3161B8 applies low-byte flags a2/a3 before 0x316174.
    // Source zero result: no child destruction; word14 still stored.
    assert(originalManagerChildUpdate_3161b8(c,0x10,1,0,depth,owner)==0);
    assert(owner.lastA1==0x11 && c.word14==0 && c.child10==&owner.existing &&
           owner.deletions==0 && depth.depth==0);
    assert(originalManagerChildUpdate_3161b8(c,0x10,0,1,depth,owner)==0);
    assert(owner.lastA1==0x12);
    // Only the original LOW BYTE matters for source flags.
    assert(originalManagerChildUpdate_3161b8(
               c,0x10,0x100,0x100,depth,owner)==0);
    assert(owner.lastA1==0x10);
    assert(originalManagerChildUpdate_3161b8(
               c,0x10,0x101,0x202,depth,owner)==0);
    assert(owner.lastA1==0x13);
    // Native owner sees original global +0x696C98 incremented
    // during the actual source virtual call, not before/after only.
    assert(owner.observedDepth==std::vector<std::uint32_t>({1,1,1,1}));
    owner.originalResult=2;
    assert(originalManagerChildUpdate_3161b8(
               c,0x40,0,0,depth,owner)==1);
    assert(c.word14==2 && c.child10==nullptr &&
           owner.deletions==1 && depth.depth==0);

    // The source rereads child+0x10 AFTER virtual slot5 updates.
    // A reentrantly installed new source child gets destroyed, not
    // the stale prior pointer snapshot.
    c.child10=&owner.existing;owner.replaceInUpdate=true;
    owner.originalResult=1;
    assert(originalManagerChildUpdate_3161b8(
               c,7,0,0,depth,owner)==1);
    assert(owner.deletions==2 && c.child10==nullptr && c.word14==1);
    owner.replaceInUpdate=false;

    // The retail update-depth word is SHARED between nested updates.
    c.child10=&owner.existing;owner.originalResult=0;
    owner.triggerNested=true;owner.observedDepth.clear();
    assert(originalManagerChildUpdate_3161b8(
               c,9,0,0,depth,owner)==0);
    assert(owner.observedDepth==std::vector<std::uint32_t>({1,2}));
    assert(depth.depth==0 && c.child10==&owner.existing);
    owner.triggerNested=false;

    // Unresolved original engine virtual calls reject, with neither
    // falsely published source word14 nor a stuck global counter.
    owner.failMethod=true;c.word14=0xCAFE;
    bool rejected=false;
    try {(void)originalManagerChildUpdate_3161b8(
             c,0,0,0,depth,owner);}
    catch(const std::runtime_error&){rejected=true;}
    assert(rejected && depth.depth==0 && c.word14==0xCAFE &&
           c.child10==&owner.existing);
    owner.failMethod=false;

    // Missing source child is invalid, NOT a constructed fake object.
    c.child10=nullptr;rejected=false;
    try {(void)originalManagerChildUpdate_3161b8(
             c,0,0,0,depth,owner);}
    catch(const std::invalid_argument&){rejected=true;}
    assert(rejected && depth.depth==0);

    // Real deleting destructor must execute. If the owner cannot
    // perform it, do not falsely clear the original child pointer.
    c.child10=&owner.existing;owner.failDelete=true;owner.originalResult=1;
    rejected=false;
    try {(void)originalManagerChildUpdate_3161b8(
             c,0,0,0,depth,owner);}
    catch(const std::runtime_error&){rejected=true;}
    assert(rejected && c.child10==&owner.existing &&
           c.word14==1 && depth.depth==0);
    std::cout<<"ORIGINAL_MANAGER_UPDATE_316174_3161B8_PASSED\n";
}
