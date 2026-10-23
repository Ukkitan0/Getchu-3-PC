#pragma once
#include "ae3/original_manager_child.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace ae3 {
// The distinct 200-instruction original controller method
// SCUS_975.01 0x57BF5C..0x57C27C. Original VU0 operands, original
// game-owned child constructors and diagnostics remain REQUIRED source
// operations, not host-calculated movement substitutes.
//
// 0x57C120 creates two original 0x441F40 scratch objects at stack
// +0x20 and +0x60 (source spacing 0x40). These opaque host-owned
// buffers are untouched by genuine original 0x441F40 (no-op).
// They may be consumed only AFTER original 0x36F5F4 has supplied
// the actual original data needed by the 0x30C57C constructor.
struct OriginalMovementScratch_57bf5c {
    std::array<std::byte,0x40> sourceStack20{};
    std::array<std::byte,0x40> sourceStack60{};
};
class OriginalManagerMovementCalls_57bf5c {
public:
    virtual ~OriginalManagerMovementCalls_57bf5c()=default;
    // Original VF normalisation and COP1 comparison at
    // 0x57BFAC..0x57C008. Return the genuine source FPR f1.
    virtual float call_original_vu0_direction_57bfac(
        OriginalManagerChild70& originalController)=0;
    // Source 0x21B854 is called only when controller flag+0x1C !=0
    // and first normal-vs-(-0.707) gate did not transition.
    virtual void call_original_21b854(
        OriginalManagerChild70& originalController)=0;
    // Source VU0 metric + SQRT.S at 0x57C090..0x57C0D8.
    // Called after the optional 0x21B854 projection; return the
    // actual source FPR f0 tested against the original 2.0f.
    virtual float call_original_vu0_metric_57c090(
        OriginalManagerChild70& originalController)=0;
    // Source 0x441F40 (called twice at 0x57C128/0x57C130) is a
    // FULLY verified original no-op: no store to either stack block.
    // No external callback or synthetic scratch initializer exists.
    virtual void call_original_36f5f4(
        OriginalMovementScratch_57bf5c& originalScratch)=0;
    virtual void* call_original_30c57c(
        OriginalManagerChild70& originalController,
        const OriginalMovementScratch_57bf5c& originalScratch)=0;
    virtual void call_original_child_vtable1_delete(void*)=0;
    virtual std::uint32_t call_original_3161b8(
        void*& originalChild10,std::uint32_t originalA1)=0;
    virtual std::uint32_t call_original_316174(
        OriginalManagerChild70&,std::uint32_t originalA1)=0;
    // Original 0x57C1E0..0x57C1F4 128-bit PCEQW/PPACB equality.
    // Must compare genuine source +0x20 and +0x40 vectors.
    virtual bool call_original_vectors_equal_57c1e0(
        const OriginalManagerChild70&)=0;
    virtual std::uint8_t read_original_6354e7()=0;
    virtual std::uint8_t read_original_6354e0()=0;
    // Diagnostic/source fallback at 0x57C264 called only on
    // update=0 and original global gate 6354E7 && !6354E0.
    virtual void call_original_21db64(
        OriginalManagerChild70& originalController)=0;
};

// Recovered outer original branch/update order, preserving source
// 0x708A30 {0x57C27C,0} and 0x708A38 {0x57C48C,0}
// method constants. A missing real owner callback MUST fail/throw.
// This is native translated controller logic, not a full actor.
std::uint32_t originalManagerMovementUpdate_57bf5c(
    OriginalManagerChild70& originalController,
    std::uint32_t originalA1,
    OriginalManagerMovementCalls_57bf5c& originalCalls);
} // namespace ae3
