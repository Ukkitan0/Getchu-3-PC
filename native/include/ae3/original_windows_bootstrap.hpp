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
