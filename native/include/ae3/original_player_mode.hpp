#pragma once
#include <cstdint>
namespace ae3 {
// C++ translation of SCUS_975.01 Player secondary vmethod at 0x34A57C.
// Keep original addresses as names until their actual code is recovered.
// No synthesized player movement or game state is supplied here.
class OriginalPlayerModeCalls {
public:
    virtual ~OriginalPlayerModeCalls() = default;
    virtual void call_33fce4(std::uint32_t selector) = 0;
    virtual void call_34675c(float value) = 0;
    virtual void call_3509ac(std::uint32_t kind, std::uint32_t one) = 0;
    virtual void call_33fd90() = 0;
    virtual void call_324260(bool enabled) = 0;
};
// Returns true if an original function is dispatched. The native caller
// must supply REAL recovered implementations, not no-op placeholders.
bool dispatchOriginalPlayerMode(std::uint32_t originalMode,
                                OriginalPlayerModeCalls& original);
} // namespace ae3


namespace ae3 {
struct OriginalChipGainCore {
    std::int32_t pendingBeforeFinalClamp = 0;
    std::int32_t threshold = -1;
    bool crossedThreshold = false;
};
// Source-level port of the integer branch logic of original 0x33FCE4.
// The returned pending value feeds original 0x26FB24, which performs
// further packed-instruction clamping and max-chip state updates.
OriginalChipGainCore originalChipGainBeforeClamp(
    std::int32_t chipCount, std::int32_t gained,
    bool limitedTo100, bool survivalMode) noexcept;
}
