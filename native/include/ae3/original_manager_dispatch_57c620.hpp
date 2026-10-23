#pragma once
#include "ae3/original_manager_child.hpp"
#include <cstdint>

namespace ae3 {
// Original concrete controller virtual slot 5 at source vtable
// 0x6B1834 -> 0x57C620. This is source control flow and dispatch,
// NOT a guessed movement/animation implementation.
//
// word08/word0c encode a source function/method and tagged context
// offset. The real game manager must resolve that ORIGINAL method
// before executing native recovered code; never reinterpret a 32-bit
// guest VA or subobject offset as a host function pointer.
class OriginalManagerDispatchCalls_57c620 {
public:
    virtual ~OriginalManagerDispatchCalls_57c620()=default;
    virtual std::uint32_t call_original_direct_57c7ac(
        std::uint32_t originalMethodVA,std::int32_t signedSourceThisOffset)=0;
    virtual std::uint32_t call_original_virtual_57c79c(
        std::uint32_t sourceVirtualTableByteOffset,
        std::int32_t signedSourceThisOffset)=0;
    // 0x57C6C0 passes original parent+0x04, pending polygon+0x50.
    virtual void call_original_30ffd0(void* originalParent,void* originalPolygon50)=0;
    // Original post-dispatch diagnostics read these actual global bytes
    // only if resource+0x50 remains unhandled. The gate is dynamic.
    virtual std::uint8_t read_original_6354e7()=0;
    virtual std::uint8_t read_original_6354e0()=0;
    // Original 0x2478EC conditional initialization and 0x247D84 warning.
    // Caller must provide real source-derived semantics, not fake logs.
    virtual void call_original_2478ec_247d84_warning()=0;
};

// Entry receives original a1 mask. The real source returns 2 if the
// controller update-method field+0x08 is zero, otherwise the original
// dispatch result, or 1 when source argument bits bypass dispatch.
// If a genuine pending polygon remains and the original dynamic
// diagnostic gate is active, the real game-owned warning is mandatory.
// Missing original methods must throw/fail, not return synthetic success.
std::uint32_t originalManagerDispatch_57c620(
    OriginalManagerChild70& originalController,
    std::uint32_t originalA1,
    OriginalManagerDispatchCalls_57c620& originalCalls);
} // namespace ae3
