#include "ae3/original_manager_resource_selector_317b9c.hpp"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
using namespace ae3;
int main(){
    OriginalManagerResourceFields_317b9c fields{
        0x0071A100u,1u,std::nullopt};
    // Retail parent's +0x40==1 branch wins over any mode.
    assert(originalManagerSelectResource_317b9c(fields,0)==0x0071A2C0u);
    assert(originalManagerSelectResource_317b9c(fields,1)==0x0071A2C0u);
    assert(originalManagerSelectResource_317b9c(fields,0x101)==0x0071A2C0u);
    fields.originalParentWord40=0;
    // BEQZL mode-zero delay-slot, no parent+0x34 needed.
    assert(originalManagerSelectResource_317b9c(fields,0)==0x0071A320u);
    assert(originalManagerSelectResource_317b9c(fields,0x100)==0x0071A320u);
    // Source +0x34 read is mandatory only for nonzero mode.
    bool refused=false;
    try{(void)originalManagerSelectResource_317b9c(fields,1);}
    catch(const std::invalid_argument&){refused=true;}
    assert(refused);
    fields.originalParentWord34=0;
    assert(originalManagerSelectResource_317b9c(fields,0x101)==0x0071A220u);
    fields.originalParentWord34=1;
    assert(originalManagerSelectResource_317b9c(fields,1)==0x0071A240u);
    fields.originalParentWord34=0xffffffffu;
    assert(originalManagerSelectResource_317b9c(fields,0xff)==0x0071A240u);
    // ADDIU uses original 32-bit wrap semantics, not host pointers.
    fields.originalParentWord10=0xFFFFFF80u;
    fields.originalParentWord40=1;
    assert(originalManagerSelectResource_317b9c(fields,0)==0x140u);
    fields.originalParentWord40=0;
    assert(originalManagerSelectResource_317b9c(fields,0)==0x1a0u);
    fields.originalParentWord34=0;
    assert(originalManagerSelectResource_317b9c(fields,1)==0xa0u);
    fields.originalParentWord34=2;
    assert(originalManagerSelectResource_317b9c(fields,1)==0xc0u);
    std::cout<<"ORIGINAL_MANAGER_RESOURCE_317B9C_PASSED\n";
}
