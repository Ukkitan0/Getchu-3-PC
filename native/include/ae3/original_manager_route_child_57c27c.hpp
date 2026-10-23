#pragma once
#include "ae3/original_manager_child.hpp"
#include <cstdint>

namespace ae3 {
// Original USA SCUS_975.01 0x57C27C..0x57C388. This controller
// action is separate from 0x57BF5C and from 0x57C388.
class OriginalManagerRouteChildCalls_57c27c {
public:
    virtual ~OriginalManagerRouteChildCalls_57c27c() = default;
    // 0x57C2A8: parent-owned source readiness/availability predicate.
    virtual std::uint32_t call_original_3176ac(void* originalParent04) = 0;
    // 0x57C2C4: genuine child construction from parent and incoming a1.
    virtual void* call_original_30c1d4(
        void* originalParent04, std::uint32_t originalA1) = 0;
    virtual void call_original_child_vtable1_delete(void* originalChild) = 0;
    // 0x57C2FC: original (child+0x10 pointer, a1, 0, 0) update.
    virtual std::uint32_t call_original_3161b8(
        void*& sourceChild10, std::uint32_t originalA1) = 0;
    // 0x57C338 receives the controller with its original +0x08/+0x0C
    // method pair already published. Return actual source v0.
    virtual std::uint32_t call_original_316174(
        OriginalManagerChild70& controller, std::uint32_t originalA1) = 0;
};

std::uint32_t originalManagerRouteChildUpdate_57c27c(
    OriginalManagerChild70& controller, std::uint32_t originalA1,
    OriginalManagerRouteChildCalls_57c27c& source);
} // namespace ae3
