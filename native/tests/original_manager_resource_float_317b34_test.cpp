#include "ae3/original_manager_resource_float_317b34.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
using namespace ae3;
int main() {
    OriginalManagerResourceFloatFields_317b34 p{
        0x710000u,1u,0x7FC01234u,std::nullopt};
    // Original LWC1 reads raw data, even if source is NaN.
    assert(originalManagerResourceFloatBits_317b34(p,1)==0x7FC01234u);
    assert(originalManagerResourceFloatBits_317b34(p,0)==0x7FC01234u);
    p.originalParentWord40=0;
    bool refused=false;
    try{(void)originalManagerResourceFloatBits_317b34(p,0);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused);
    p.originalFloatAt3B4Bits=0xBF800000u;
    assert(originalManagerResourceFloatBits_317b34(p,0)==0xBF800000u);
    assert(originalManagerResourceFloatBits_317b34(p,0x100u)==0xBF800000u);
    refused=false;
    try{(void)originalManagerResourceFloatBits_317b34(p,1u);}
    catch(const std::logic_error&){refused=true;}
    assert(refused);
    // Unrecovered VU0 path must not substitute the other known
    // original table float or synthesize a host metric.
    p.originalParentWord40=1;
    p.originalFloatAt374Bits.reset();
    refused=false;
    try{(void)originalManagerResourceFloatBits_317b34(p,1u);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused);
    std::cout<<"ORIGINAL_RESOURCE_FLOAT_317B34_FAST_PATHS_PASSED\n";
}
