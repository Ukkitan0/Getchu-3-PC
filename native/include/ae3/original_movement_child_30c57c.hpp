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
