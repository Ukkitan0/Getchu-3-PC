#pragma once
#include "ae3/original_manager_resource_float_317b34.hpp"
#include "ae3/original_manager_resource_vu0_317b34.hpp"
#include "ae3/original_movement_step_fast_36fb18.hpp"
#include "ae3/original_movement_flagged_36fb5c.hpp"
#include "ae3/original_movement_regenerate_owned_36f8c8.hpp"
#include <cstdint>

namespace ae3 {
// Original 0x557F30..0x557F50: child+0x09 mode -> JAL
// 0x317B34, source f0 -> f20, load parent+0x70 as f12,
// multiply in 0x36FB18 JAL delay slot, then real 36FB18.
// No guest VA is directly dereferenced on the host.
struct OriginalMovementRegenerationBinding_36f8c8 {
    std::uint64_t genuineChildScratchGuestVA;
    OriginalMovementRegenerationOwner_36f8c8* originalOwner;
};
struct OriginalMovementMetricStepResult_557f34 {
    std::uint32_t originalF20MetricRaw;
    std::uint32_t originalF12ScaledRaw;
    std::uint32_t originalMovementF0Raw;
};
class OriginalMovementMetricOwner_557f34 {
public:
    virtual ~OriginalMovementMetricOwner_557f34()=default;
    virtual OriginalManagerResourceFloatFields_317b34
    read_original_resource_float_fields(std::uint32_t originalParentGuestVA)=0;
    // Invoked ONLY on actual original 317B34 nonzero-mode VU0
    // branch. Real original VF1 and resource table words required.
    virtual OriginalResourceVU0Inputs_317b34
    read_original_resource_vu0_inputs(std::uint32_t originalParentGuestVA)=0;
    // Source original 0x317B60/0x317B68/0x317B6C updates
    // physical VF1, not only COP1 f0. Required on the VU0
    // branch to keep the genuine persistent VU0 register state.
    virtual void publish_original_vf1_after_317b34(
        std::uint32_t originalParentGuestVA,
        const std::array<std::uint32_t,4>& originalVF1After)=0;
    virtual std::uint32_t read_original_parent_float70(
        std::uint32_t originalParentGuestVA)=0;
    // Optional genuine unflagged 36FC18 regeneration before
    // resuming full source 36FB18. Never guess guest scratch
    // addresses or any VF register values.
    virtual std::optional<OriginalMovementRegenerationBinding_36f8c8>
    read_original_regeneration_binding_36f8c8(
        const OriginalMovementChildD0_30c57c&) {return std::nullopt;}
    // Optional *second* original 36FC18 regeneration for a
    // FLAGGED positive-movement pass with remaining f20>0.
    // Not the initial bit1-clear 36FC18 binding above: the
    // owner must know current live VU0 registers and original
    // child scratch VA at each subsequent iteration.
    virtual std::optional<OriginalMovementRegenerationBinding_36f8c8>
    read_original_flagged_loop_binding_36fc18(
        const OriginalMovementChildD0_30c57c&) {return std::nullopt;}
    // Opt-in original 36FB5C flagged positive-movement path:
    // the owner supplies actual 128-bit VF1, source scratch
    // guest address and actual child status/flags. Default
    // absent preserves mandatory retail 36FB18 owner fallback.
    virtual std::optional<OriginalMovementFlaggedInputs_36fb5c>
    read_original_flagged_vf1_36fb5c(
        const OriginalMovementChildD0_30c57c&,
        std::uint32_t) {return std::nullopt;}
    virtual void publish_original_vf1_after_36fb5c(
        const std::array<std::uint32_t,4>&) {
        throw std::logic_error(
            "original 36FB5C live VF1 state publication unavailable");
    }
    // Required for movement f12 positive below threshold.
    virtual std::uint32_t call_original_36fb18(
        OriginalMovementChildD0_30c57c& originalChild,
        std::uint32_t originalScaledF12Raw)=0;
};

// Executes ORIGINAL source prefix through 0x557F50 using a
// genuine source child that completed 0x557ED8 normal setup.
// The subsequent 0x557F54 collision/animation remains incomplete.
OriginalMovementMetricStepResult_557f34 originalMovementMetricStep_557f34(
    OriginalMovementChildD0_30c57c& originalChild,
    OriginalMovementMetricOwner_557f34& originalOwner);
} // namespace ae3
