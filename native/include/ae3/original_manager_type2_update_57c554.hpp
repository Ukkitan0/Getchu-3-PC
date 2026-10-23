#pragma once
#include "ae3/original_manager_child.hpp"
#include "ae3/original_manager_packed_compare.hpp"
#include <cstdint>

namespace ae3 {
// The original controller 0x57BCB8 installs source method 0x57C554
// for action type 2. The complete original 0x57C554..0x57C620
// function is 51 EE instructions (hash-pinned separately).
// This is source actor child creation/update, NOT a substitute
// movement system or synthesized Unreal Actor.
//
// Source game owners must implement original 0x30C360 creation,
// polymorphic child destruction and original 0x3161B8 child updating.
class OriginalManagerType2Calls_57c554 {
public:
    virtual ~OriginalManagerType2Calls_57c554()=default;
    // Original +0x04 parent, +0x40 source VU0 operand, +0x50
    // pending polygon and +0x1D low-byte mode are preserved by
    // owner. Never create a fake child to satisfy the callback.
    virtual void* call_original_30c360(
        OriginalManagerChild70& originalController)=0;
    // Source child vtable[1] deleting destructor if original+0x10
    // becomes non-null while genuine child construction runs.
    virtual void call_original_child_vtable1_delete(void* originalChild)=0;
    // Source 0x3161B8 receives pointer to controller+0x10,
    // unchanged original a1 and zero a2/a3; may change child pointer.
    virtual std::uint32_t call_original_3161b8(
        void*& sourceChild10,std::uint32_t originalA1)=0;
    // Source alternative30 VU/EE vector equality (0x57C5D4..F8).
    // Missing source owner operands must reject, not be approximated.
    virtual bool call_original_variant_vector_equal_57c5d4(
        const OriginalManagerChild70& originalController) {
        return originalManagerPackedPceqwEqual_57c1e0(originalController);
    }
};

// If original 0x3161B8 returns zero => source result zero.
// Source branch-delay audit: variant-zero returns source field +0x14.
// Variant-one ALSO returns +0x14 when all four PCEQW vector lanes
// match, and returns 1 otherwise. Prior documentation inverted this.
// This method mutates only genuine typed controller +0x10 on a
// verified source child creation/update sequence.
std::uint32_t originalManagerType2Update_57c554(
    OriginalManagerChild70& originalController,
    std::uint32_t originalA1,
    OriginalManagerType2Calls_57c554& originalCalls);
} // namespace ae3
