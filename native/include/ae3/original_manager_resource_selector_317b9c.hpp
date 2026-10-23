#pragma once
#include <cstdint>
#include <optional>

namespace ae3 {
// Complete 14-word retail SCUS_975.01 method 0x317B9C..0x317BD4.
// Called from movement child 0x557EE8 with original a0=parent
// and a1=child+0x09 mode byte. Values are SOURCE 32-bit guest VAs,
// not host pointers. parent+0x34 is conditionally loaded by retail.
struct OriginalManagerResourceFields_317b9c {
    std::uint32_t originalParentWord10; // resource base guest VA
    std::uint32_t originalParentWord40; // selects fixed +0x1C0
    std::optional<std::uint32_t> originalParentWord34; // needed only for nonzero mode
};

// Returns genuine guest resource address, preserving original EE
// unsigned 32-bit ADDIU wrap and branch-likely mode-zero behavior.
// Missing original +0x34 is refused only when actually accessed.
std::uint32_t originalManagerSelectResource_317b9c(
    const OriginalManagerResourceFields_317b9c& genuineParent,
    std::uint32_t originalModeA1);
} // namespace ae3
