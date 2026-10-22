#include "ae3/original_windows_bootstrap.hpp"
#include "ae3/original_windows_static_resources.hpp"
#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace ae3::runtime;
namespace {
void require(bool ok,const char* what){if(!ok)throw std::runtime_error(what);}
class NativeConstructorRecorder final:public OriginalNativeConstructorCalls {
public:
    std::vector<std::uint32_t> called;
    std::uint32_t unresolved=0;
    bool callRecoveredConstructor(std::uint32_t va) override {
        called.push_back(va);
        return va!=unresolved;
    }
};
void check(int argc,char**argv){
    const auto& source=originalConstructorOrder_428ab8();
    require(source.size()==143,"original retail constructor count");
    require(source.front()==0x41a390&&source.back()==0x10526c,
            "original source constructor table endpoints/order");
    // Original table was extracted from the SHA-256-verified USA ELF;
    // a full-order fingerprint detects a wrong middle entry that endpoint
    // and monotonic checks alone would miss.
    std::uint64_t fingerprint=14695981039346656037ull;
    for(std::uint32_t va:source){
        for(unsigned shift=0;shift<32;shift+=8){
            fingerprint^=(va>>shift)&0xffu;
            fingerprint*=1099511628211ull;
        }
    }
    require(fingerprint==0xe8da5c252d4839d5ull,
            "original static constructor addresses/order differ from verified ELF");
    for(std::size_t i=1;i<source.size();++i)
        require(source[i]<source[i-1],"source constructor table must traverse strictly reverse addresses");
    NativeConstructorRecorder incomplete;
    incomplete.unresolved=source[2];
    const auto interrupted=originalRunConstructors_428ab8(incomplete);
    require(!interrupted.complete && interrupted.executed==2 &&
            interrupted.firstUnrecovered==source[2] && incomplete.called.size()==3,
            "source dispatcher must fail at the first missing native constructor");
    NativeConstructorRecorder none;
    none.unresolved=source.front();
    auto halted=originalRunConstructors_428ab8(none);
    require(halted.executed==0&&halted.firstUnrecovered==0x41a390 &&
            !halted.complete,"initial original game constructor must not be skipped");
    NativeConstructorRecorder complete;
    const auto finished=originalRunConstructors_428ab8(complete);
    require(finished.complete&&finished.executed==source.size() &&
            !finished.firstUnrecovered && complete.called==std::vector<std::uint32_t>(source.begin(),source.end()),
            "original 143-entry dispatch order changed");
    OriginalAllocatorGlobals_41a390 globals;
    for(const auto value:globals.counters)
        require(value==0,"original constructor globals must start as zeroed CRT BSS");
    originalAllocatorCtor_41a390(globals);
    for(const auto value:globals.counters)
        require(value==1,"original 0x41A088 first allocator/global registration lost");
    originalAllocatorCtor_41a390(globals);
    for(const auto value:globals.counters)
        require(value==2,"original first initializer reentry counter behavior wrong");
    globals.counters[6]=0xffffffffffffffffull;
    originalAllocatorCtor_41a390(globals);
    require(globals.counters[6]==0,
            "original 64-bit DADDIU accumulator wrapping not preserved");
    for(std::size_t i=0;i<globals.counters.size();++i)
        if(i!=6)require(globals.counters[i]==3,
                         "original registration updates other counters incorrectly");
    OriginalNumericGlobals_417c58 numeric;
    require(originalNumericCtor_417c58(numeric) && numeric.initialized,
            "Original 0x417C58 fixed finite IEEE constructor failed");
    require(numeric.first_69e678==UINT64_C(0x40294c583ada5b53) &&
            numeric.second_69e680==UINT64_C(0x4021e3779b97f4a8),
            "0x417B50 must store original correctly rounded sqrt products");
    const auto previous=numeric;
    require(originalNumericCtor_417c58(numeric) &&
            numeric.first_69e678==previous.first_69e678 &&
            numeric.second_69e680==previous.second_69e680,
            "Original repeated second constructor must preserve constant globals");
    class RecoveredStartup final:public OriginalNativeConstructorCalls {
    public:
        OriginalAllocatorGlobals_41a390 allocator{};
        OriginalNumericGlobals_417c58 numeric{};
        bool callRecoveredConstructor(std::uint32_t va) override {
            if(va==0x41a390) {originalAllocatorCtor_41a390(allocator);return true;}
            if(va==0x417c58) return originalNumericCtor_417c58(numeric);
            return false;
        }
    } startup;
    const auto actualBoundary=originalRunConstructors_428ab8(startup);
    require(actualBoundary.executed==2 &&
            actualBoundary.firstUnrecovered==0x3a6cd0 &&
            !actualBoundary.complete &&
            startup.allocator.counters[0]==1 &&
            startup.numeric.initialized,
            "native original boot must stop at genuine third constructor");
    if(argc==2){
        const auto literal=originalReadStaticSourceFromElf_3a6cd0(argv[1]);
        class OriginalThreeConstructors final : public OriginalNativeConstructorCalls {
        public:
            std::span<const std::byte> source;
            OriginalAllocatorGlobals_41a390 allocator{};
            OriginalNumericGlobals_417c58 numeric{};
            OriginalStaticResourceGlobals_3a6cd0 staticResources{};
            bool callRecoveredConstructor(std::uint32_t va) override {
                if(va==0x41a390){originalAllocatorCtor_41a390(allocator);return true;}
                if(va==0x417c58)return originalNumericCtor_417c58(numeric);
                if(va==0x3a6cd0)return originalStaticCtor_3a6cd0(source,staticResources);
                return false;
            }
        } startupWithOriginalSource;
        startupWithOriginalSource.source=literal;
        const auto result=originalRunConstructors_428ab8(startupWithOriginalSource);
        require(!result.complete && result.executed==3 &&
                result.firstUnrecovered==0x39fc78 &&
                startupWithOriginalSource.staticResources.initialized,
                "original private-source constructor #3 should advance to #4");
    }
    OriginalNativeTerminationRegistry registry;
    require(!registry.register_428b80({}),"original exit callback registry accepted absent function");
    std::vector<int> calls;
    for(int i=0;i<71;++i)
        require(registry.register_428b80([&calls,i]{calls.push_back(i);}),
                "native destructor registration failed");
    require(registry.count()==71,"original 32-element termination block rollover");
    registry.runRegisteredCallbacks_428cb8();
    require(registry.count()==0&&calls.size()==71,
            "original termination callbacks not all invoked");
    for(int i=0;i<71;++i)
        require(calls[std::size_t(i)]==70-i,
                "original termination callback reverse sequence lost");
    // Native callback throws: preserve unexecuted stack, no invented success.
    int attempts=0;
    registry.register_428b80([&]{++attempts;});
    registry.register_428b80([&]{throw std::runtime_error("unresolved native teardown dependency");});
    bool thrown=false;
    try{registry.runRegisteredCallbacks_428cb8();}
    catch(const std::runtime_error&){thrown=true;}
    require(thrown&&registry.count()==1&&attempts==0,
            "native teardown error must stop and retain later callbacks");
    registry.runRegisteredCallbacks_428cb8();
    require(attempts==1&&registry.count()==0,"native teardown resume lost source callbacks");
}
}
int main(int argc,char**argv) {
    try {check(argc,argv);std::cout<<"ORIGINAL_WINDOWS_BOOTSTRAP_PASSED original_ctors=143 teardown_callbacks=71\n";return 0;}
    catch(const std::exception& e){
        std::cerr<<"ORIGINAL_WINDOWS_BOOTSTRAP_FAILED "<<e.what()<<"\n";return 1;
    }
}
