#pragma once
#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>

namespace ae3 {

struct OriginalManagerResourceFields_317b9c;
class OriginalMovementNormalCalls_557ed8;
class OriginalMovementChildD0_30c57c;
class OriginalMovementRegenerationOwner_36f8c8;
struct OriginalMovementRegenerateResult_36f8c8;
OriginalMovementRegenerateResult_36f8c8 originalMovementRegenerateOwned_36f8c8(
    OriginalMovementChildD0_30c57c&,std::uint64_t,
    OriginalMovementRegenerationOwner_36f8c8&);
struct OriginalMovementFlaggedInputs_36fb5c;
struct OriginalMovementFlaggedPass_36fb5c;
void originalMovementCommitFlaggedReturn_36fb5c(
    OriginalMovementChildD0_30c57c&,
    const OriginalMovementFlaggedInputs_36fb5c&,
    const OriginalMovementFlaggedPass_36fb5c&);
void originalMovementNormalBegin_557ed8(
    OriginalMovementChildD0_30c57c& child,
    std::uint32_t originalParentWord14,
    OriginalMovementNormalCalls_557ed8& genuineOwner);

// Original 0x30C57C..0x30C670, exactly 61 EE instruction words.
// Retail requests 0xD0 bytes from 0x41B800 and initializes only
// certain fields. This native typed view tracks which bytes were
// genuinely written, NOT an invented fully-zeroed actor object.
// Guest EE vtable and parent addresses stay 32-bit SOURCE VALUES.
// Host allocation ownership is explicit, no guest VA dereference.
class OriginalMovementChildD0_30c57c {
public:
    static constexpr std::size_t sourceObjectSize=0xD0;
    static constexpr std::uint32_t originalVtableVA=0x006B0B20u;
    // Returns no value for bytes left uninitialized by retail
    // 0x30C57C; never silently substitute zeros for unknown state.
    std::optional<std::byte> originalByteAt(std::size_t offset) const {
        if(offset>=sourceObjectSize) throw std::out_of_range(
            "original movement child byte outside 0xD0 allocation");
        if(!initialized_.test(offset)) return std::nullopt;
        return bytes_[offset];
    }
    std::uint32_t originalWordAt(std::size_t offset) const {
        if(offset>sourceObjectSize-4) throw std::out_of_range(
            "original movement child word outside 0xD0 allocation");
        std::uint32_t value=0;
        for(std::size_t i=0;i<4;++i) {
            if(!initialized_.test(offset+i))
                throw std::logic_error(
                    "original 0x30C57C has not initialized this word");
            value|=std::uint32_t(std::to_integer<unsigned>(
                bytes_[offset+i]))<<(8u*i);
        }
        return value;
    }
private:
    friend OriginalMovementRegenerateResult_36f8c8 originalMovementRegenerateOwned_36f8c8(
        OriginalMovementChildD0_30c57c&,std::uint64_t,
        OriginalMovementRegenerationOwner_36f8c8&);
    friend void originalMovementCommitFlaggedReturn_36fb5c(
        OriginalMovementChildD0_30c57c&,
        const OriginalMovementFlaggedInputs_36fb5c&,
        const OriginalMovementFlaggedPass_36fb5c&);
    // This friend is narrowly limited to the source 0x557ED8..0x557F30
    // writes (+0xB4/+0xB8/+0xC0). It is NOT a public arbitrary guest
    // memory patch method or a replacement animation updater.
    friend void originalMovementNormalBegin_557ed8(
        OriginalMovementChildD0_30c57c&,std::uint32_t,
        OriginalMovementNormalCalls_557ed8&);
    friend std::unique_ptr<OriginalMovementChildD0_30c57c>
    originalMovementChildCreate_30c57c(
        std::uint32_t,std::span<const std::byte,0x110>,
        const std::bitset<0x110>&,std::uint32_t,std::uint32_t);
    void writeByte(std::size_t offset,std::byte value) noexcept {
        bytes_[offset]=value;
        initialized_.set(offset);
    }
    void writeWord(std::size_t offset,std::uint32_t value) noexcept {
        for(std::size_t i=0;i<4;++i)
            writeByte(offset+i,std::byte((value>>(8u*i))&255u));
    }
    std::array<std::byte,sourceObjectSize> bytes_{};
    std::bitset<sourceObjectSize> initialized_{};
};

// Genuine retail signature: a0 original parent, a1 source scratch,
// a2 first low-byte argument, a3 second low-byte argument.
// Original 0x57C15C caller passes a2=1 and a3=controller+0x1D.
// The scratch must be the actual source-produced 0x110-byte frame,
// not invented geometry/actor motion.
std::unique_ptr<OriginalMovementChildD0_30c57c>
originalMovementChildCreate_30c57c(
    std::uint32_t originalParentGuestVA,
    std::span<const std::byte,0x110> originalScratch,
    std::uint32_t originalA2,std::uint32_t originalA3);

// Identical retail 0x30C57C write/copy sequence, but the source
// 0x110-byte stack scratch may contain uninitialized/unknown bytes.
// The game performs the full 0xB0 copy, yet native callers must
// NOT treat copied unknown bytes as grounded original actor state.
// Source `knownSourceBytes` marks ONLY data proven initialized by
// the original game. Unknown bytes remain unreadable in the child.
std::unique_ptr<OriginalMovementChildD0_30c57c>
originalMovementChildCreate_30c57c(
    std::uint32_t originalParentGuestVA,
    std::span<const std::byte,0x110> originalScratch,
    const std::bitset<0x110>& knownSourceBytes,
    std::uint32_t originalA2,std::uint32_t originalA3);
} // namespace ae3
