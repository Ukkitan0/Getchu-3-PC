#include "ae3/original_manager_routed_update_57c388.hpp"
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
struct C : OriginalManagerRoutedCalls_57c388 {
    std::vector<std::string> events;
    int created=1,old=2;
    std::array<std::byte,16> originalVector{};
    bool publishOldInRoute=false,publishOldInCtor=false,rejectRoute=false;
    std::uint32_t update=1,finish=0x8765,seenA1=0;
    std::array<std::byte,16> read_original_parent14_vector40(
        const OriginalManagerChild70& c) override {
        assert(c.parent04==this);events.push_back("owner-vector");
        return originalVector;
    }
    void call_original_57c7cc(OriginalManagerChild70& c,std::uint32_t a1) override {
        events.push_back("57C7CC");seenA1=a1;
        originalVector[0]=std::byte{0xFA}; // LQ snapshot persists
        if(publishOldInRoute)c.child10=&old;
        if(rejectRoute)throw std::runtime_error("original route owner unavailable");
    }
    void* call_original_30c21c(OriginalManagerChild70& c,
        const std::array<std::byte,16>& captured) override {
        events.push_back("30C21C");
        assert(captured[0]==std::byte{0x77});
        assert(c.flag1d==0x41);
        if(publishOldInCtor)c.child10=&old;
        return &created;
    }
    void call_original_child_vtable1_delete(void* ptr) override {
        events.push_back("child-delete");assert(ptr==&old);
    }
    std::uint32_t call_original_3161b8(void*& ptr,std::uint32_t a1) override {
        events.push_back("3161B8");seenA1=a1;
        assert(ptr==&created);return update;
    }
    std::uint32_t call_original_316174(
        OriginalManagerChild70& c,std::uint32_t a1) override {
        events.push_back("316174");
        assert(a1==seenA1 && c.word08==0x57c48c && c.word0c==0);
        return finish;
    }
};
int main(){
    C o{};o.originalVector[0]=std::byte{0x77};
    OriginalManagerChild70 c{};c.parent04=&o;c.flag1c=1;c.flag1d=0x41;
    // Owner LQ, genuine 57C7CC, VU+30C21C, 3161B8,
    // original 0x708A38 method pair and 316174.
    assert(originalManagerRoutedUpdate_57c388(c,0xD2,o)==o.finish);
    assert(c.child10==&o.created && c.word08==0x57c48c && c.word0c==0);
    assert(o.events==std::vector<std::string>(
        {"owner-vector","57C7CC","30C21C","3161B8","316174"}));
    o.events.clear();o.update=0;c.word08=0x1234;c.word0c=11;
    assert(originalManagerRoutedUpdate_57c388(c,0xD3,o)==0);
    assert(c.word08==0x1234 && c.word0c==11 &&
           o.events==std::vector<std::string>({"3161B8"}));
    o.events.clear();o.update=1;
    assert(originalManagerRoutedUpdate_57c388(c,0xD4,o)==o.finish);
    assert(o.events==std::vector<std::string>({"3161B8","316174"}));
    c.child10=nullptr;c.flag1c=0;o.publishOldInCtor=true;
    o.events.clear();o.originalVector[0]=std::byte{0x77};
    assert(originalManagerRoutedUpdate_57c388(c,0xD5,o)==o.finish);
    assert(o.events==std::vector<std::string>(
        {"owner-vector","30C21C","child-delete","3161B8","316174"}));
    c.child10=nullptr;c.flag1c=1;o.publishOldInCtor=false;
    o.rejectRoute=true;o.originalVector[0]=std::byte{0x77};o.events.clear();
    bool failed=false;
    try{(void)originalManagerRoutedUpdate_57c388(c,0xD6,o);}
    catch(const std::runtime_error&){failed=true;}
    assert(failed && c.child10==nullptr &&
           o.events==std::vector<std::string>({"owner-vector","57C7CC"}));
    std::cout<<"ORIGINAL_ROUTED_57C388_SOURCE_PASSED\n";
}
