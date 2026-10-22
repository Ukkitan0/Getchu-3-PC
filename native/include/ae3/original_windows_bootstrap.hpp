#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace ae3::runtime {

// Retail USA SCUS_975.01 static constructor dispatcher 0x428AB8, source
// table [0x62E664 .. 0x62E89C], traversed in descending ADDRESS order.
// These are *original function IDs*, never callable guest instruction pointers.
constexpr std::size_t kOriginalRetailConstructorCount_428ab8 = 143;
const std::array<std::uint32_t,kOriginalRetailConstructorCount_428ab8>&
originalConstructorOrder_428ab8() noexcept;

class OriginalNativeConstructorCalls {
public:
    virtual ~OriginalNativeConstructorCalls()=default;
    // True ONLY if the REAL named retail constructor ran in native C++.
    // No simulated PS2 MIPS, optimistic placeholders, or silent success.
    virtual bool callRecoveredConstructor(std::uint32_t originalVA)=0;
};
struct OriginalConstructorProgress {
    std::size_t executed=0;
    std::uint32_t firstUnrecovered=0;
    bool complete=false;
};
OriginalConstructorProgress originalRunConstructors_428ab8(
    OriginalNativeConstructorCalls& dispatch);

// Original 0x41A390 -> 0x41A088(1,0xFFFF) updates twelve independent
// global 64-bit constructor counters. Source 0x4173B0 is a *true no-op*;
// every special first-increment branch calls that method and returns.
// Native C++ globals replace EE BSS addresses, not EE memory emulation.
struct OriginalAllocatorGlobals_41a390 {
    std::array<std::uint64_t,12> counters{};
};
void originalAllocatorCtor_41a390(OriginalAllocatorGlobals_41a390&) noexcept;

// Original second constructor 0x417C58 -> 0x417B50(1,0xffff).
// Verified original 0x41D6B8 -> 0x41EFD0 is the double sqrt, and
// 0x4240D8 is the double multiply. ONLY this constructor's three
// hardcoded positive finite inputs have been source-recovered here.
// Values correspond to original global 0x69E678 and 0x69E680.
struct OriginalNumericGlobals_417c58 {
    std::uint64_t first_69e678=0;
    std::uint64_t second_69e680=0;
    bool initialized=false;
};
bool originalNumericCtor_417c58(OriginalNumericGlobals_417c58&) noexcept;

// 0x428B80: original host-independent registration of 32 termination
// callbacks per 0x88-byte source descriptor. We do not copy guest pointers.
class OriginalNativeTerminationRegistry {
public:
    using Callback = std::function<void()>;
    bool register_428b80(Callback callback);
    // 0x428CB8 walks the original callback blocks newest-first, each block
    // last-to-first; its unrecovered global owner hook and OS exit remain
    // separate native-engine dependencies, NOT automatically run here.
    void runRegisteredCallbacks_428cb8();
    std::size_t count() const noexcept;
private:
    std::vector<std::vector<Callback>> blocks_;
};
} // namespace ae3::runtime
