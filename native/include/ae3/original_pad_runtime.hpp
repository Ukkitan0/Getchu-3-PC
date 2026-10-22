#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace ae3 {
// Physical DualShock 2 button bit positions (0=Select through 15=Square).
// Host pressedMask uses POSITIVE polarity (1=held); it is NOT a cast of
// the original PS2 SIO packet, whose on-wire button polarity differs.
// Retail gameplay button-to-gadget dispatch has NOT been recovered.
enum class OriginalPadButton : std::uint16_t {
    select=0x0001, left_stick=0x0002, right_stick=0x0004, start=0x0008,
    up=0x0010, right=0x0020, down=0x0040, left=0x0080,
    l2=0x0100, r2=0x0200, l1=0x0400, r1=0x0800,
    triangle=0x1000, circle=0x2000, cross=0x4000, square=0x8000
};
constexpr std::uint16_t originalPadBit(OriginalPadButton value) noexcept {
    return static_cast<std::uint16_t>(value);
}

// Neutral values are transport conventions. These are not evidence of
// the original retail pad-memory structure, analog filtering, or thresholds.
struct OriginalPadObservation {
    bool connected=false;
    std::uint16_t pressedMask=0;
    std::array<std::uint8_t,4> sticks{128,128,128,128}; // LX, LY, RX, RY; Y down-positive
    std::uint8_t leftTrigger=0, rightTrigger=0;
};
struct OriginalPadFrame {
    std::uint64_t gameFrame=0; // must come from native game frame scheduler
    OriginalPadObservation raw{};
    std::uint16_t newlyPressed=0, newlyReleased=0;
};
class OriginalPadFrameRuntime {
public:
    // No synthetic key repeat or frame-time approximation. One explicit
    // sample per native frame. On device loss, all held buttons release.
    OriginalPadFrame advance(std::uint64_t frame,OriginalPadObservation observation);
    void reset() noexcept;
private:
    std::uint64_t previousFrame_=0;
    std::uint16_t previousButtons_=0;
    bool initialized_=false;
};

// Actual original Lua set_player_weapon binding @ 0x512000 compares names
// to "empty", "net", "club", otherwise yields 3, stores via 0x266CF4.
// This is script-directed selection, NOT a host-button gadget binding.
std::uint32_t originalSetPlayerWeaponSelection_512000(std::string_view name) noexcept;
struct OriginalGadgetSelectionEvent {
    std::uint64_t gameFrame=0;
    std::uint32_t originalSelection=0;
};
class OriginalGadgetSelectionSink {
public:
    virtual ~OriginalGadgetSelectionSink()=default;
    virtual void onOriginalGadgetSelection(const OriginalGadgetSelectionEvent&)=0;
};
// No event until original stage logic explicitly supplies a selection.
void forwardOriginalGadgetSelection(std::optional<std::uint32_t> selection,
    std::uint64_t gameFrame,OriginalGadgetSelectionSink& sink);

// Keyboard mapping remains explicit/unbound until input bindings are
// verified. Virtual key codes are Windows VK values provided by integrator.
struct OriginalKeyboardBindings {
    std::array<unsigned,16> buttonVirtualKeys{};
    struct Axis {unsigned negativeVirtualKey=0,positiveVirtualKey=0;};
    std::array<Axis,4> axes{}; // raw LX,LY,RX,RY, no gameplay mapping
};
// Windows-only host physical input; non-Windows returns disconnected.
// hostWindow must be the foreground window or the sample is disconnected.
OriginalPadObservation pollWin32OriginalKeyboard(void* hostWindow,
    const OriginalKeyboardBindings& bindings) noexcept;
OriginalPadObservation pollWin32OriginalGamepad(void* hostWindow,
    unsigned xinputIndex=0) noexcept;
} // namespace ae3
