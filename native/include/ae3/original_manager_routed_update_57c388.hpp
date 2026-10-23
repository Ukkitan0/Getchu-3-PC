#pragma once
#include "ae3/original_manager_child.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
namespace ae3 {
// Original 0x57C388..0x57C48C: route-owning child transition.
class OriginalManagerRoutedCalls_57c388 {
public:
    virtual ~OriginalManagerRoutedCalls_57c388()=default;
    // Source LQ at 0x57C3C4 from (parent+0x14)+0x40 BEFORE route.
    virtual std::array<std::byte,16> read_original_parent14_vector40(
        const OriginalManagerChild70&)=0;
    virtual void call_original_57c7cc(OriginalManagerChild70&,std::uint32_t originalA1)=0;
    // Genuine 0x57C3D8..0x57C3E8 VU0 preparation + 0x30C21C.
    // Owner supplies real controller+0x40 VF operand and executes
    // original VU transform; never substitute host steering.
    virtual void* call_original_30c21c(
        OriginalManagerChild70&,
        const std::array<std::byte,16>& capturedOriginalOwnerVector40)=0;
    virtual void call_original_child_vtable1_delete(void*)=0;
    virtual std::uint32_t call_original_3161b8(
        void*& originalChild10,std::uint32_t originalA1)=0;
    virtual std::uint32_t call_original_316174(
        OriginalManagerChild70&,std::uint32_t originalA1)=0;
};
std::uint32_t originalManagerRoutedUpdate_57c388(
    OriginalManagerChild70&,std::uint32_t originalA1,
    OriginalManagerRoutedCalls_57c388&);
} // namespace ae3
