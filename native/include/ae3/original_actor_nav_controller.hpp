#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace ae3 {
// Host-native semantic fields, not an emulated 0x70-byte EE object.
// Both retail constructors invoke still-unrecovered 0x57BCB8 afterward.
struct OriginalActorRouteControllerPrepared_57ba98 {
    const void* originalActor = nullptr;                   // +0x04
    const void* originalRoute = nullptr;                   // +0x18
    std::uint32_t wordsAt08And0c[2]{};                     // +0x08, +0x0C
    void* childAt10 = nullptr;                             // +0x10
    std::uint32_t wordAt14 = 0;                           // +0x14
    std::uint8_t routeHasAtLeastTwo = 0;                  // +0x1C
    std::uint8_t originalRequestedFlag = 0;               // +0x1D
    // Variant 0 does not write +0x20..+0x2F: absent != zero.
    std::optional<std::array<std::byte, 16>> vectorAt20;
    std::uint8_t variantAt30 = 0;                         // +0x30
    bool requiresOriginalPostConstructor_57bcb8 = true;
};
// 0x57BA98: variant zero, with no source vector write at +0x20.
OriginalActorRouteControllerPrepared_57ba98 originalActorControllerPrepare_57ba98(
    const void* originalActor, const void* originalRoute,
    std::int32_t originalRouteWordAt04, std::uint32_t lowByteFlag);

// 0x57BAF8: variant one. The retail EE SQ stores $a3's full 16 bytes
// at +0x20; argument $t0 is stored as a byte at +0x1D.
OriginalActorRouteControllerPrepared_57ba98 originalActorControllerPrepare_57baf8(
    const void* originalActor, const void* originalRoute,
    std::int32_t originalRouteWordAt04,
    const std::array<std::byte, 16>& originalVector, std::uint32_t lowByteFlag);

// Child+0x10 is a virtual source object. Each method remains explicit
// until the original child-vtable implementations have been recovered.
struct OriginalActorControllerChildCalls {
    virtual ~OriginalActorControllerChildCalls() = default;
    virtual void call_virtual_04_destroy(void* child) = 0;
    virtual std::int32_t call_virtual_08_status(void* child) = 0;
    virtual void call_virtual_0c_notify(void* child) = 0;
    virtual std::int32_t call_virtual_10_forward(void* child,
        std::uint32_t originalArg5, std::int32_t originalArg6) = 0;
};

// 0x57BC0C: actor+0x40 nonzero -> 3, otherwise child virtual+8,
// or zero when there is no child.
std::int32_t originalActorControllerStatus_57bc0c(
    bool sourceActorWord40Nonzero, void* childAt10,
    OriginalActorControllerChildCalls& originalCalls);
// 0x57BC54: child vtable+0x10, or original incoming $a2 if absent.
std::int32_t originalActorControllerForward_57bc54(
    void* childAt10, std::uint32_t originalArg5,
    std::int32_t originalArg6, OriginalActorControllerChildCalls& originalCalls);
// 0x57BC88: notify child at vtable+0x0C when present.
void originalActorControllerNotify_57bc88(
    void* childAt10, OriginalActorControllerChildCalls& originalCalls);

// 0x57BB60 and deleting destructor 0x57BBB4: child vtable+0x04
// then deleting variant calls original 0x41A3B0. These helpers do
// not write guest vtable addresses or implement host allocation.
struct OriginalActorControllerDeallocateCalls {
    virtual ~OriginalActorControllerDeallocateCalls() = default;
    virtual void call_original_41a3b0(void* object) = 0;
};
void originalActorControllerDestroy_57bb60(
    void* childAt10, OriginalActorControllerChildCalls& originalCalls);
void originalActorControllerDelete_57bbb4(
    void* sourceController, void* childAt10,
    OriginalActorControllerChildCalls& originalCalls,
    OriginalActorControllerDeallocateCalls& allocator);
} // namespace ae3
