#pragma once
#include <array>
#include <cstdint>
#include <string_view>

namespace ae3 {
// Original Player vtable slot 2, 0x34A014, compares a 16-byte key with
// zero-padded, 16-byte names embedded in original SCUS_975.01.
using OriginalResourceName16=std::array<std::uint8_t,16>;
OriginalResourceName16 originalResourceName16(std::string_view name);

// Native pointers to *actual original Player subobjects*; null means
// an original object has not been reconstructed. The suffixes are actual
// guest structure offsets. +0x5C/+0x60 are pointer fields; the rest
// correspond to embedded subobjects, NOT guest pointer fields.
struct OriginalPlayerResources {
    void* player_at_5c=nullptr;
    void* main_player_at_60=nullptr;
    void* item_chatcher_embedded_64=nullptr; // original retail spelling
    void* force_embedded_180=nullptr;
    void* bar_interface_embedded_70=nullptr;
    void* iZone_embedded_110=nullptr;
    void* iHanger_embedded_188=nullptr;
    void* iRope_embedded_1a0=nullptr;
    void* iDashBall_embedded_1d0=nullptr;
    void* iHeroSwitch_embedded_258=nullptr;
};

// 0x37D348 loads the original global pointer at 0x6A99BC;
// 0x37D354 clears it and 0x37D360 reads byte +0x8A0 only if present.
struct OriginalSecondaryFactoryGlobal {
    void* instanceAt_6a99bc=nullptr;
    std::uint8_t instanceByte8a0=0;
};
void* originalSecondaryFactoryGet_37d348(const OriginalSecondaryFactoryGlobal&) noexcept;
void originalSecondaryFactoryReset_37d354(OriginalSecondaryFactoryGlobal&) noexcept;
std::uint8_t originalSecondaryFactoryByte_37d360(const OriginalSecondaryFactoryGlobal&) noexcept;

void* originalPlayerResourceLookup_34a014(
    const OriginalPlayerResources&,
    const OriginalResourceName16&,
    const OriginalSecondaryFactoryGlobal&) noexcept;
} // namespace ae3
