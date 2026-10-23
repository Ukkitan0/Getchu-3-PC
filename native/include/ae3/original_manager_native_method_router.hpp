#pragma once
#include "ae3/original_manager_dispatch_57c620.hpp"
#include "ae3/original_manager_route_child_57c27c.hpp"
#include "ae3/original_manager_routed_update_57c388.hpp"
#include "ae3/original_manager_followup_57c48c.hpp"
#include "ae3/original_manager_type2_update_57c554.hpp"
#include "ae3/original_manager_movement_57bf5c.hpp"
#include <cstdint>
namespace ae3 {
// Joins source-recovered controller update entrypoints with original
// 0x57C620 direct-method dispatcher. Does NOT allocate game actors.
struct OriginalManagerNativeMethodOwner {
    OriginalManagerRouteChildCalls_57c27c& routeChild;
    OriginalManagerRoutedCalls_57c388& routedChild;
    OriginalManagerFollowupCalls_57c48c& followup;
    OriginalManagerType2Calls_57c554& type2;
    // Actual game owner handles original 0x30FFD0, tagged virtual
    // methods, diagnostics and unrecovered/source-offset dispatches.
    OriginalManagerDispatchCalls_57c620& realOwner;
    // Optional genuine source owner for the original 0x57BF5C
    // movement operation. Null continues to require explicit real
    // owner dispatch, which must reject until translated/available.
    OriginalManagerMovementCalls_57bf5c* movement=nullptr;
};
std::uint32_t originalManagerDispatchNativeMethods_57c620(
    OriginalManagerChild70& controller,std::uint32_t originalA1,
    OriginalManagerNativeMethodOwner& originalOwner);
} // namespace ae3
