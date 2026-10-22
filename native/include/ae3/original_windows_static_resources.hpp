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
}
