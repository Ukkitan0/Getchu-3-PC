#pragma once
#include <array>
#include <cstdint>
#include <stdexcept>

namespace ae3 {
// Complete source-verified retail USA 0x276538..0x276558 (9 EE
// instructions), SHA256 of original words:
// 9fac81a5a29c684c47eb01060fca63b2170cd868e717212e586451296db6a9be.
//
// This native function reproduces the *entire wrapper*: source
// 27653C ADDIU a0,a0,0x50; 276540 SQ a1,0(sp);
// 276548 JAL 274988; 27654C DADDU a1,sp,zero delay slot.
// The original 274988 VU0 normalization remains a separate
// game-owner operation; no host transform or rounding is invented.
class OriginalMovementForwardOwner_274988 {
public:
    virtual ~OriginalMovementForwardOwner_274988()=default;
    // Original a0 points to parent+0x50 as an EE guest VA.
    // Original a1 points to a stack-local copy of all 128 bits.
    // Owner implements the real 274988 routine and its VF effects.
    virtual void call_original_274988(
        std::uint64_t originalParentPlus50SignExtended,
        const std::array<std::uint32_t,4>& originalStackVector)=0;
};

inline std::uint64_t originalMovementAddiuParent50_276538(
    std::uint64_t originalParentWord14SignExtended) {
    // Original 557E68 loads s4 using LW and 557FF0 passes it
    // with DADDU. This particular caller must supply a canonical
    // 64-bit sign-extended 32-bit EE register, not a host pointer.
    const auto low=static_cast<std::uint32_t>(originalParentWord14SignExtended);
    const auto source=static_cast<std::uint64_t>(
        static_cast<std::int64_t>(static_cast<std::int32_t>(low)));
    if(source!=originalParentWord14SignExtended)
        throw std::invalid_argument(
            "original 276538 parent a0 not sign-extended from retail 557E68");
    // MIPS64 ADDIU performs 32-bit wrap and sign-extends the
    // result to the full 64-bit register. Do NOT do a host
    // 64-bit pointer addition or zero-extension.
    const std::uint32_t after=low+0x50u;
    return static_cast<std::uint64_t>(
        static_cast<std::int64_t>(static_cast<std::int32_t>(after)));
}
inline void originalMovementForward_276538(
    std::uint64_t originalParentWord14SignExtended,
    const std::array<std::uint32_t,4>& originalEES3,
    OriginalMovementForwardOwner_274988& originalOwner) {
    const auto target=
        originalMovementAddiuParent50_276538(originalParentWord14SignExtended);
    // Retail SQ copies the complete 128-bit original a1 GPR
    // to stack+0, then the JAL delay-slot moves sp into a1.
    // The stack itself is unobservable except through 274988;
    // the typed immutable 128-bit value models that exact copy.
    originalOwner.call_original_274988(target,originalEES3);
}
} // namespace ae3
