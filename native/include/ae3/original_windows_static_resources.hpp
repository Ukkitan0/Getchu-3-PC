#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
namespace ae3::runtime {
constexpr std::uint32_t kStaticLiteralSourceVA_3a6cd0=0x720040;
constexpr std::size_t kStaticLiteralSourceBytes_3a6cd0=0xd0;
struct OriginalStaticResourceEntry_3a6cd0 {
    std::array<std::byte,16> sourceLiteral{};
    std::uint32_t typeWord=0;
};
struct OriginalStaticResourceGlobals_3a6cd0 {
    std::array<std::array<std::byte,16>,4> globalLiterals{};
    std::array<OriginalStaticResourceEntry_3a6cd0,9> entries{};
    bool initialized=false;
};
bool originalStaticCtor_3a6cd0(std::span<const std::byte> originalSource,
    OriginalStaticResourceGlobals_3a6cd0& destination) noexcept;
std::uint64_t originalStaticSourceFingerprint_3a6cd0(
    std::span<const std::byte> originalSource) noexcept;
std::array<std::byte,kStaticLiteralSourceBytes_3a6cd0> originalReadStaticSourceFromElf_3a6cd0(
    const std::filesystem::path& originalUsaElf);
// Retail original 0x39FC78 -> 0x39FA10 copies 38 authentic 16-byte
// source records (0x71F600..) to the original global 0x699E10..0x69A06F.
constexpr std::uint32_t kFourthCtorLiteralVA_39fc78=0x71f600;
constexpr std::size_t kFourthCtorLiteralBytes_39fc78=0x260;
struct OriginalFourthGlobals_39fc78 {
    std::array<std::array<std::byte,16>,38> namesAndParameters{};
    bool initialized=false;
};
bool originalFourthCtor_39fc78(std::span<const std::byte> originalSource,
    OriginalFourthGlobals_39fc78& destination) noexcept;
std::array<std::byte,kFourthCtorLiteralBytes_39fc78> originalReadFourthSourceFromElf_39fc78(
    const std::filesystem::path& originalUsaElf);
}
