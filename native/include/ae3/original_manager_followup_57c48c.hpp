#pragma once
#include "ae3/original_manager_child.hpp"
#include <cstdint>
namespace ae3 {
// Original USA 0x57C48C..0x57C554, method installed by 0x57C388.
class OriginalManagerFollowupCalls_57c48c {
public:
    virtual ~OriginalManagerFollowupCalls_57c48c()=default;
    // Original +4 parent, packed parent +0x40 vector,
    // controller mode +0x1D; owner supplies genuine operands.
    virtual void* call_original_30c520(OriginalManagerChild70&)=0;
    virtual void call_original_child_vtable1_delete(void*)=0;
    virtual std::uint32_t call_original_3161b8(
        void*& sourceChild10,std::uint32_t originalA1)=0;
    // Source 0x57C508..0x57C51C PCEQW, PPACB, NOR, SLTIU.
    virtual bool call_original_variant_vector_equal_57c508(
        const OriginalManagerChild70&)=0;
};
std::uint32_t originalManagerFollowupUpdate_57c48c(
    OriginalManagerChild70&,std::uint32_t originalA1,
    OriginalManagerFollowupCalls_57c48c&);
} // namespace ae3
