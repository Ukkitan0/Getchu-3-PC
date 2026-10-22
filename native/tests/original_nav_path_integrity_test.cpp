#include "ae3/original_nav_path_integrity.hpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

using namespace ae3;
static void put16(std::vector<std::byte>& b,std::size_t i,unsigned n) {
    b.at(i)=std::byte(n&255u);
    b.at(i+1)=std::byte((n>>8)&255u);
}
static void put32(std::vector<std::byte>& b,std::size_t i,unsigned n) {
    put16(b,i,n);
    put16(b,i+2,n>>16);
}
template<class Fn> static void rejected(Fn&& f) {
    bool got=false;
    try{f();}
    catch(const std::invalid_argument&){got=true;}
    catch(const std::out_of_range&){got=true;}
    if(!got)throw std::runtime_error("Original graph route integrity failed to reject invalid path");
}
int main() {
    std::vector<std::byte> nav(0x80+5*0x1eu);
    put32(nav,0x24,5);put32(nav,0x3c,0x80);
    const auto edge=[&](int src,int slot,int dst) {
        const auto at=std::size_t(0x80+0x1e*src);
        put16(nav,at+2,slot+1);put16(nav,at+4+slot*2,dst);
    };
    edge(0,0,1);edge(0,1,2);edge(1,0,3);edge(2,0,3);
    // Actual native source workspace is DFS ordered, NOT vertex-ID
    // sorted. 0->1->3 and 0->2->3 are BOTH valid paths.
    const std::array<std::uint32_t,4> admitted{0,1,3,2};
    const std::array<std::uint32_t,3> predecessorPath{0,1,3};
    const std::array<std::uint32_t,3> alternatePath{0,2,3};
    originalNavValidateNativeSourceRouteIntegrity(nav,5,admitted,predecessorPath,2);
    originalNavValidateNativeSourceRouteIntegrity(nav,5,admitted,alternatePath,2);
    originalNavValidateNativeSourceRouteIntegrity(
        nav,5,admitted,std::span<const std::uint32_t>{},-1);
    rejected([&]{originalNavValidateNativeSourceRouteIntegrity(
        nav,5,admitted,predecessorPath,1);});
    const std::array<std::uint32_t,3> wrongEnd{0,2,1};
    rejected([&]{originalNavValidateNativeSourceRouteIntegrity(
        nav,5,admitted,wrongEnd,2);});
    const std::array<std::uint32_t,3> cyclic{0,1,0};
    rejected([&]{originalNavValidateNativeSourceRouteIntegrity(
        nav,5,admitted,cyclic,0);});
    const std::array<std::uint32_t,2> disconnected{0,3};
    rejected([&]{originalNavValidateNativeSourceRouteIntegrity(
        nav,5,admitted,disconnected,2);});
    const std::array<std::uint32_t,3> repeatedAdmission{0,1,1};
    rejected([&]{originalNavValidateNativeSourceRouteIntegrity(
        nav,5,repeatedAdmission,predecessorPath,2);});
    rejected([&]{originalNavValidateNativeSourceRouteIntegrity(
        nav,5,admitted,predecessorPath,-1);});
    rejected([&]{originalNavValidateNativeSourceRouteIntegrity(
        nav,5,admitted,std::span<const std::uint32_t>{},2);});
    rejected([&]{originalNavValidateNativeSourceRouteIntegrity(
        nav,4,admitted,predecessorPath,2);});
    auto broken=nav;put16(broken,0x80+2,4);
    rejected([&]{originalNavValidateNativeSourceRouteIntegrity(
        broken,5,admitted,predecessorPath,2);});
    // Synthetic mutation regression; deterministic seed and original
    // format, no copyrighted game assets or gameplay approximation.
    std::mt19937 rng(0x220994);
    std::size_t accepted=0,rejectedCount=0;
    for(std::size_t repeat=0;repeat<20000;++repeat) {
        auto candidate=predecessorPath;
        for(auto& value:candidate)
            if((rng()&1u)==0)value=rng()%8u;
        try {
            originalNavValidateNativeSourceRouteIntegrity(nav,5,admitted,candidate,2);
            ++accepted;
        } catch(const std::invalid_argument&) {
            ++rejectedCount;
        } catch(const std::out_of_range&) {
            ++rejectedCount;
        }
    }
    if(!accepted || !rejectedCount)
        throw std::runtime_error("Original route integrity mutation coverage incomplete");
}
