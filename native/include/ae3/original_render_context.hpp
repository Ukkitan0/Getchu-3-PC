#pragma once
#include "ae3/original_render_pass.hpp"
#include <cstdint>
#include <functional>

namespace ae3 {
// Native representation of original 0x3B76D0..0x3B773C graphics-context
// pointer slots. These are host addresses/handles, NOT 32-bit EE memory.
// The original MIPS code writes pointers at offsets 0x828, 0x82C, 0x830,
// 0x834; 0x818 holds the 32-bit word read from +0x0C of a +0x828 source.
struct OriginalRenderContextSlots {
    std::uint32_t source818=0;
    std::uintptr_t source824=0;
    std::uintptr_t source828=0;
    std::uintptr_t source82c=0;
    std::uintptr_t source830=0;
    std::uintptr_t source834=0;
};
// Exact original source 0x3B76D0, 0x3B76E8, 0x3B76F8, 0x3B7708.
void originalSetRenderSource828(OriginalRenderContextSlots& context,
                               std::uintptr_t source, std::uint32_t originalSourceWord0c) noexcept;
void originalSetRenderSource82c(OriginalRenderContextSlots& context,
                               std::uintptr_t source) noexcept;
void originalSetRenderSource830(OriginalRenderContextSlots& context,
                               std::uintptr_t source) noexcept;
void originalSetRenderSource834(OriginalRenderContextSlots& context,
                               std::uintptr_t source) noexcept;
std::uintptr_t originalGetRenderSource824(const OriginalRenderContextSlots& context) noexcept;
std::uintptr_t originalGetRenderSource828(const OriginalRenderContextSlots& context) noexcept;
std::uintptr_t originalGetRenderSource82c(const OriginalRenderContextSlots& context) noexcept;
std::uintptr_t originalGetRenderSource830(const OriginalRenderContextSlots& context) noexcept;
std::uintptr_t originalGetRenderSource834(const OriginalRenderContextSlots& context) noexcept;

struct OriginalRenderExternalPointers {
    std::uintptr_t savedSlot=0;       // primary s5; secondary s5
    std::uintptr_t savedState=0;      // primary s7
    std::uintptr_t stageSource=0;     // 0x2802D4 -> singleton +4
};
using OriginalRenderSlotResolver=std::function<OriginalRenderContextSlots&(
    OriginalRenderId, std::uint32_t selectorIndex)>;
using OriginalRenderComplexRoutine=std::function<void(
    const OriginalRetailDrawEvent&, OriginalRenderContextSlots&)>;
// Execute verified context state stores, preserving source event order.
// Every still-unported draw/effect/mode function MUST be bound to a real
// implementation; without callbacks this fails closed rather than pretending
// the scene actually rendered. Calls back synchronously per event.
void applyOriginalRetailRenderTrace(const OriginalRetailPassTrace& trace,
                                    const OriginalRenderExternalPointers& pointers,
                                    const OriginalRenderSlotResolver& resolve,
                                    const OriginalRenderComplexRoutine& originalCall);
} // namespace ae3
