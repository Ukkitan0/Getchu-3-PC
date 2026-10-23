#include "ae3/original_manager_followup_57c48c.hpp"
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
struct Calls:OriginalManagerFollowupCalls_57c48c {
    int created=0,previous=0;
    bool insertPrevious=false,vectorEqual=true,missingVector=false;
    std::uint32_t updated=1,a1Seen=0;
    std::vector<std::string> calls;
    void* call_original_30c520(OriginalManagerChild70& c) override {
        calls.emplace_back("30C520");assert(c.flag1d==0x42);
        if(insertPrevious)c.child10=&previous;
        return &created;
    }
    void call_original_child_vtable1_delete(void* p) override {
        calls.emplace_back("child_delete");assert(p==&previous);
    }
    std::uint32_t call_original_3161b8(void*& ptr,std::uint32_t a1) override {
        calls.emplace_back("3161B8");a1Seen=a1;assert(ptr==&created);return updated;
    }
    bool call_original_variant_vector_equal_57c508(const OriginalManagerChild70&) override {
        calls.emplace_back("PCEQW");
        if(missingVector)throw std::runtime_error("source controller+0x20/+0x40 missing");
        return vectorEqual;
    }
};
int main(){
    OriginalManagerChild70 c{};Calls o{};c.flag1d=0x42;c.word14=0x714;
    // Source null child: 30C520 then 3161B8; variant-zero returns +0x14.
    assert(originalManagerFollowupUpdate_57c48c(c,0x21,o)==0x714);
    assert(c.child10==&o.created && o.a1Seen==0x21);
    assert(o.calls==std::vector<std::string>({"30C520","3161B8"}));
    o.calls.clear();assert(originalManagerFollowupUpdate_57c48c(c,0x22,o)==0x714);
    assert(o.calls==std::vector<std::string>({"3161B8"}));
    o.updated=0;o.calls.clear();c.alternative30=1;
    assert(originalManagerFollowupUpdate_57c48c(c,0x23,o)==0);
    assert(o.calls==std::vector<std::string>({"3161B8"}));
    o.updated=1;o.calls.clear();o.vectorEqual=true;
    assert(originalManagerFollowupUpdate_57c48c(c,0x24,o)==0x714);
    assert(o.calls==std::vector<std::string>({"3161B8","PCEQW"}));
    o.calls.clear();o.vectorEqual=false;
    assert(originalManagerFollowupUpdate_57c48c(c,0x25,o)==1);
    assert(o.calls==std::vector<std::string>({"3161B8","PCEQW"}));
    c.child10=nullptr;c.alternative30=0;o.insertPrevious=true;o.calls.clear();
    assert(originalManagerFollowupUpdate_57c48c(c,0x26,o)==0x714);
    assert(o.calls==std::vector<std::string>({"30C520","child_delete","3161B8"}));
    c.alternative30=1;o.calls.clear();o.missingVector=true;
    bool failed=false;try{(void)originalManagerFollowupUpdate_57c48c(c,0x27,o);}
    catch(const std::runtime_error&){failed=true;}
    assert(failed && o.calls==std::vector<std::string>({"3161B8","PCEQW"}));
    std::cout<<"ORIGINAL_FOLLOWUP_57C48C_PASSED\n";
}
