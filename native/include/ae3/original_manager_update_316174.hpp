#pragma once
#include "ae3/original_manager_child.hpp"
#include <cstdint>

namespace ae3 {
// Original 0x316174..0x3161B8 and 0x3161B8..0x31622C,
// audited against the exact USA SCUS_975.01 executable.
//
// Original 0x316174 uses global word 0x696C98 as a live,
/// reentrant virtual-update nesting counter. The actual game owner
// must preserve the SAME counter for all original object updates.
struct OriginalUpdateDepth_696c98 {
    std::uint32_t depth=0;
};

class OriginalManagerUpdateOwner_316174 {
public:
    virtual ~OriginalManagerUpdateOwner_316174()=default;
    // Original 0x316174 reads object->vtable[5] (+0x14) and JALR.
    // Use genuine recovered virtual method, never cast a guest VA
    // into a host function pointer.
    virtual std::uint32_t call_original_vtable5(
        void* originalObject,std::uint32_t originalA1)=0;
    // Original 0x316200..0x316208 invokes vtable slot 1
    // (deleting destructor) on the CURRENT child pointer.
    virtual void call_original_child_vtable1_delete(
        void* originalObject)=0;
};

// Recovered source 0x316174: increment original 0x696C98 before
// virtual slot-5 invocation; decrement afterward. An exception from
// an unresolved owner cannot leave the native source counter stuck.
std::uint32_t originalManagerGuardedUpdate_316174(
    void* originalChild,std::uint32_t originalA1,
    OriginalUpdateDepth_696c98& originalDepth,
    OriginalManagerUpdateOwner_316174& genuineOwner);

// Recovered source 0x3161B8: applies low-byte a2/a3 flags to a1,
// calls 0x316174 on child+0x10, stores the genuine result at +0x14,
// and returns ZERO when still active. Any NONZERO returned result
// destroys current child by its real vtable[1], clears +0x10 and
// returns ONE. This is the original controller lifecycle semantics.
std::uint32_t originalManagerChildUpdate_3161b8(
    OriginalManagerChild70& originalController,
    std::uint32_t originalA1,std::uint32_t originalA2,
    std::uint32_t originalA3,
    OriginalUpdateDepth_696c98& originalDepth,
    OriginalManagerUpdateOwner_316174& genuineOwner);
} // namespace ae3
