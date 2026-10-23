#pragma once
#include "ae3/original_manager_resource_float_317b34.hpp"
#include "ae3/original_manager_resource_vu0_317b34.hpp"
#include "ae3/original_movement_step_fast_36fb18.hpp"
#include <cstdint>

namespace ae3 {
// Original 0x557F30..0x557F50: child+0x09 mode -> JAL
// 0x317B34, source f0 -> f20, load parent+0x70 as f12,
// multiply in 0x36FB18 JAL delay slot, then real 36FB18.
// No guest VA is directly dereferenced on the host.
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
    virtual std::uint32_t read_original_parent_float70(
        std::uint32_t originalParentGuestVA)=0;
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
