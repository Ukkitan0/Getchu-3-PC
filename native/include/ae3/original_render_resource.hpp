#pragma once
#include "ae3/original_render_selector.hpp"
#include <array>
#include <cstdint>
#include <optional>

namespace ae3 {
// Typed native owner for original actor +0x110 handle -> +0x00 inner
// 0x60-byte graphics resource (0x26A510, 0x26A854). Not EE memory layout;
// undefined malloc data is intentionally tracked as unset, not fabricated.
struct OriginalRenderResourceState {
    std::uint8_t flag00=0;      // original inner +0x00
    std::uint8_t flag08=0;      // original inner +0x08
    OriginalRenderSelector selector{}; // inner +0x10/+0x14
    std::optional<float> value04;
    std::optional<float> value0c;
    bool selected14Initialized=false;
    std::array<std::uintptr_t,5> references40to50{};
};
struct OriginalRenderResourceOwner {
    std::optional<OriginalRenderResourceState> state; // optional +0x110 handle
};
// 0x26A854 allocates an outer 4-byte handle and 0x60-byte inner resource
// only if actor+0x110 was null. 0x26A510 initializes precisely known bytes.
OriginalRenderResourceState& originalEnsureRenderActorResource(OriginalRenderResourceOwner& owner);
// Wrapper accessors 0x26AFAC, 0x26B3D0, 0x26B550: returns zero when
// optional handle missing, otherwise raw zero-extended original flag byte.
std::uint8_t originalReadActorFlag00(const OriginalRenderResourceOwner& owner) noexcept;
std::uint8_t originalReadActorFlag08(const OriginalRenderResourceOwner& owner) noexcept;
std::uint8_t originalReadActorFlag10(const OriginalRenderResourceOwner& owner) noexcept;
// 0x26A8DC, 0x26A968: update source bytes through an existing inner handle.
void originalSetResourceFlag00(OriginalRenderResourceState& state,std::uint8_t value) noexcept;
void originalSetResourceFlag08(OriginalRenderResourceState& state,std::uint8_t value) noexcept;
// 0x26A8AC and 0x26A938 also store +4 and +C float values respectively.
// This only covers their verified scalar stores, not intervening VU commands.
void originalActivateResourceValue04(OriginalRenderResourceState& state,float value);
void originalActivateResourceValue0c(OriginalRenderResourceState& state,float value);
// 0x26AEB4 sets selected+0x14 then byte+0x10=1; 0x26AEE0
// clears byte+0x10 but does not clear the selected value.
void originalSelectResourceIndex(OriginalRenderResourceState& state,std::uint32_t index) noexcept;
void originalClearResourceIndex(OriginalRenderResourceState& state) noexcept;
std::uint32_t originalReadResourceIndex(const OriginalRenderResourceState& state);
} // namespace ae3
