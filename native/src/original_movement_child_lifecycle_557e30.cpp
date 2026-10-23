#include "ae3/original_movement_child_lifecycle_557e30.hpp"
#include "ae3/original_movement_child_early_36f954.hpp"
#include "ae3/original_movement_normal_begin_557ed8.hpp"
#include "ae3/original_movement_metric_step_557f34.hpp"
#include "ae3/original_movement_post_vector_557f54.hpp"
#include "ae3/original_movement_post_step_558030.hpp"
#include "ae3/original_movement_flag0_regenerate_36f83c.hpp"
#include <stdexcept>
#include <utility>

namespace ae3 {
OriginalMovementChildLifecycle_557e30::OriginalMovementChildLifecycle_557e30(
    std::unique_ptr<OriginalMovementChildD0_30c57c> child,
    OriginalMovementParentRead_557e30& originalParent,
    OriginalMovementNormalCalls_557ed8* normalOwner,
    OriginalMovementMetricOwner_557f34* metricOwner,
    OriginalMovementPostVectorOwner_557f54* postVectorOwner,
    OriginalMovementPostStepContinuation_558030* postStepOwner)
    :child_(std::move(child)),parent_(originalParent),
     normalOwner_(normalOwner),metricOwner_(metricOwner),
     postVectorOwner_(postVectorOwner),postStepOwner_(postStepOwner) {
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
    const auto originalParentWord14 =
        parent_.read_original_parent_word14(originalParentVA);

    // Source 0x557E34: ANDI a1,1 -> early status 1 at 0x557E80.
    // Source 0x557E6C..0x557E78: (a1>>1)&1 -> same early status1.
    // The actual method has more gameplay paths for zero flags:
    // source 0x557EB4 (and onward), which we MUST NOT fake.
    if((a1&3u)!=0u) return 1u;
    // Source 0x557E78 BEQZL executes 0x557E7C LBU child+0xC0
    // only when both a1 low bits are clear. The retail constructor
    // initializes +0xC0 to zero. Preserve the source read before
    // refusing the still-unrecovered 0x557EB4+ gameplay branches.
    const auto flagC0=child_->originalByteAt(0xC0);
    if(!flagC0)
        throw std::logic_error("original 0x557E7C child +0xC0 is unknown");
    // Genuine retail 0x557EB4 BNEZL branches to 0x557F34
    // when +0xC0 is nonzero. Its delay slot loads child+0x09;
    // that path still needs source movement/gameplay recovery.
    if (*flagC0==std::byte{0}) {
        // Retail 0x557EC8 JAL 0x36F954 with a0=child+0x10
        // and 0x704B04 f12=2.0 in the JAL delay slot.
        // When original 36F954 returns 1, 0x557ED0 BNEZ
        // branches to the method epilogue; its delay slot sets
        // v0=2. Real controller 0x3161B8 then owns deletion.
        if(originalMovementChildTerminal36F954(*child_))
            return 2u;
        if(normalOwner_) {
            // Source 0x557ED8..0x557F30: genuinely enter the
            // original nonterminal reset/resource/collision setup
            // through real source callback methods. Parent+0x14
            // came from mandatory 0x557E68 delay-slot load.
            originalMovementNormalBegin_557ed8(
                *child_,originalParentWord14,*normalOwner_);
            if(metricOwner_) {
                // Source 0x557F34..0x557F50 executes
                // original 0x317B34 metric and source 0x36FB18
                // call, but post-call child state at 0x557F54
                // still needs source collision/animation recovery.
                const auto movement=originalMovementMetricStep_557f34(
                    *child_,*metricOwner_);
                if(const auto early=try_original_post_vector_557f54(originalParentWord14))
                    return *early;
                if(const auto post=try_original_post_step_558030(
                       movement.originalF20MetricRaw,
                       movement.originalMovementF0Raw))
                    return *post;
            }
        }
    }
    else if (metricOwner_ && *flagC0!=std::byte{0}) {
        // Source 0x557EB4 BNEZL: an ALREADY initialized child
        // jumps directly to 0x557F34, skipping 36F954,
        // 557ED8 resets and 276B68/2773D4 setup calls.
        // Original 0x557EB8 LBU child+0x09 runs in the branch
        // delay slot; originalMovementMetricStep_557f34 reads
        // that exact typed child mode before calling 317B34.
        // The actual 36FB18 movement step still requires source
        // owner callbacks when the genuine fast path is absent.
        const auto movement=originalMovementMetricStep_557f34(
            *child_,*metricOwner_);
        if(const auto early=try_original_post_vector_557f54(originalParentWord14))
            return *early;
        if(const auto post=try_original_post_step_558030(
               movement.originalF20MetricRaw,
               movement.originalMovementF0Raw))
            return *post;
    }
    // A successfully executed prefix is NOT a complete actor
    // frame. Original 0x557F54+ collision/animation is missing.
    throw std::logic_error(
        "original 0x557F34 gameplay continuation requires unrecovered source methods");
}
std::optional<std::uint32_t> OriginalMovementChildLifecycle_557e30::
try_original_post_vector_557f54(std::uint32_t originalParentWord14) {
    if(!postVectorOwner_)
        return std::nullopt;
    // Original 557F54 flags+B8 bit1 zero requires original
    // 558094 JAL 36F8C8. Do not run the bit1-set normalization
    // on a missing original regenerated VF1 state.
    if((child_->originalWordAt(0xB8)&2u)==0u)
        return std::nullopt;
    const auto source=originalMovementPostVector_557f54(
        *child_,originalParentWord14);
    // Source VU0 VF1/VF2/Q/GPR s3 side effects must reach the
    // original game owner before slot5 can return status1.
    postVectorOwner_->publish_original_557f54_vu0(source);
    return source.originalSlot5EarlyStatus;
}
std::optional<std::uint32_t> OriginalMovementChildLifecycle_557e30::
try_original_post_step_558030(
    std::uint32_t originalF20ResourceMetric,
    std::uint32_t originalF21MovementReturn) {
    if(!postStepOwner_)
        return std::nullopt;
    // This owner is not a replacement for the unported original
    // 557FC8..55802C actor/collision operations. Before allowing
    // the genuine 558030 block, require successfully published
    // 557F54 VU0 outputs from the same child and genuine bit1.
    if(!postVectorOwner_ || (child_->originalWordAt(0xB8)&2u)==0u)
        throw std::logic_error(
            "original 558030 cannot follow an absent 557F54 vector state");
    // Original 557FC8 checks +B8 bit0. If clear the original
    // 558084 call regenerates 36F83C before the remaining
    // source actor/collision methods. Do not skip this source call
    // or ask for an already-completed 558030 continuation first.
    if((child_->originalWordAt(0xB8)&1u)==0u) {
        const auto binding=postStepOwner_->read_original_flag0_binding_36f83c(*child_);
        if(!binding || !binding->originalOwner)
            throw std::logic_error(
                "original 557FC8 bit0-clear requires real 36F83C regeneration owner");
        (void)originalMovementRegenerateFlag0Owned_36f83c(
            *child_,binding->genuineChildScratchGuestVA,*binding->originalOwner);
    }
    // The post-step owner now receives a child that has passed
    // the required original flag0 regeneration when selected.
    const auto original=postStepOwner_->read_original_post_step_after_557fc8(*child_);
    if(!original)
        return std::nullopt;
    // Original 557F48 stores the 317B34 resource metric in f20,
    // while 557F60 MOV.S f21,f0 captures the genuine 36FB18
    // movement return. Those caller-owned COP1 registers are the
    // exact operands consumed by 558060 DIV.S. A continuation
    // cannot quietly substitute a different resource or movement.
    if(original->originalF20ResourceMetricBits!=originalF20ResourceMetric ||
       original->originalF21MovementReturnBits!=originalF21MovementReturn)
        throw std::invalid_argument(
            "original 558030 f20/f21 continuation mismatches 557F48/557F60 source");
    // The continuation owner certifies source-backed intermediate
    // processing and supplies f20/f21/s5, never placeholder physics.
    // Return source status 0 or 2 without inventing completion.
    return originalMovementPostStep_558030(*child_,*original,*postStepOwner_);
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
