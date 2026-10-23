#include "ae3/original_movement_child_lifecycle_557e30.hpp"
#include <stdexcept>
#include <utility>

namespace ae3 {
OriginalMovementChildLifecycle_557e30::OriginalMovementChildLifecycle_557e30(
    std::unique_ptr<OriginalMovementChildD0_30c57c> child,
    OriginalMovementParentRead_557e30& originalParent)
    :child_(std::move(child)),parent_(originalParent) {
    if(!child_ || child_->originalWordAt(0)!=
       OriginalMovementChildD0_30c57c::originalVtableVA)
        throw std::invalid_argument(
            "0x557E30 requires real 0x30C57C child with original 0x6B0B20 vtable");
}
void* OriginalMovementChildLifecycle_557e30::originalChildPointer() const noexcept {
    return child_.get();
}
bool OriginalMovementChildLifecycle_557e30::ownsOriginalChild() const noexcept {
    return static_cast<bool>(child_);
}
std::uint32_t OriginalMovementChildLifecycle_557e30::call_original_vtable5(
    void* object,std::uint32_t a1) {
    if(!child_ || object!=child_.get())
        throw std::invalid_argument("0x557E30 called on unowned source child");

    // Retail 0x557E60 loads child+0x04 parent guest VA.
    // The 0x557E64 BNEZ bit0 executes 0x557E68's delay-slot
    // LW parent+0x14 on BOTH branch outcomes. Read this from
    // the actual original game parent; do not skip it merely
    // because source status will be one.
    const auto originalParentVA=child_->originalWordAt(4);
    (void)parent_.read_original_parent_word14(originalParentVA);

    // Source 0x557E34: ANDI a1,1 -> early status 1 at 0x557E80.
    // Source 0x557E6C..0x557E78: (a1>>1)&1 -> same early status1.
    // The actual method has more gameplay paths for zero flags:
    // source 0x557EB4 (and onward), which we MUST NOT fake.
    if((a1&3u)!=0u) return 1u;
    throw std::logic_error(
        "original 0x557E30 non-flag actor update requires unrecovered gameplay methods");
}
void OriginalMovementChildLifecycle_557e30::
call_original_child_vtable1_delete(void* object) {
    if(!child_ || object!=child_.get())
        throw std::invalid_argument("original 0x556A0C deleting destructor object mismatch");

    // Original slot1 at 0x556A0C stores parent vtable 0x6B0C40
    // and tail-jumps to 0x41A3B0 -> source allocator release.
    // Native ownership is a unique_ptr, so release via the
    // matching host allocation and do not expose freed source
    // child bytes or make up an active replacement actor.
    child_.reset();
}
} // namespace ae3
