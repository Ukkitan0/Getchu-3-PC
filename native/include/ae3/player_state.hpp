#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace ae3 {
// Field layout independently described by the Ape Escape 3 (USA) 2021
// reverse-engineering article, anchored to the game's Data @ 0x00649910.
// Host C++ value type: NO guest machine state, PS2 kernel or CPU emulation.
// Packed layout preserves the original persisted scalar field offsets;
// callers must not assume this is already a complete native save system.
#pragma pack(push, 1)
struct PlayerProgress {
    std::int32_t player_type = 0;
    std::int32_t player_left = 0;
    float player_life = 100.f;
    std::int32_t player_costume = 0;
    float costume_timer = 0;
    float player_energy = 0;
    float player_energy_capacity = 0;
    std::array<std::int32_t, 8> costume_possess{};
    std::array<std::int32_t, 12> gmecha_possess{};
    std::array<std::int32_t, 4> gmecha_assign{};
    std::int32_t gmecha_currentkey = 0;
    std::int32_t ammo_type = 0;
    std::array<std::int32_t, 3> ammo_count{};
    std::array<std::int32_t, 3> ammo_max{};
    std::int32_t rccar_type = 0;
    std::array<std::int32_t, 4> rccar_poss{};
    std::int32_t dance_type = 0;
    std::array<std::int32_t, 4> dance_poss{};
    std::int32_t chip_count = 0;
    std::int32_t max_chip_count = 0;
    std::array<std::uint8_t, 570> capture_flag{};
    std::array<std::uint8_t, 259> shop_item{};
};
#pragma pack(pop)
static_assert(offsetof(PlayerProgress,player_life)==8);
static_assert(offsetof(PlayerProgress,player_energy)==20);
static_assert(offsetof(PlayerProgress,capture_flag)==204);
static_assert(offsetof(PlayerProgress,shop_item)==774);
static_assert(sizeof(PlayerProgress)==1033);

// The article identifies the original HP update as clamp(life + delta, 0, 100).
// No simulated EE register calls, PS2 memory dereferences, or cheat patches.
float changePlayerHealth(PlayerProgress& state, float delta);
}
