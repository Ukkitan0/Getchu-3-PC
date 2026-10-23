#include "ae3/windows_source_sha256.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string_view>
#include <limits>

namespace {
bool verify(const void* data, std::size_t size, std::string_view expected) {
    // The Unreal stage resolver checks an original file on disk, then
    // loads bytes into memory. A modified buffer must not be accepted just
    // because a prior on-disk source hash was valid.
    originalScriptHeader[20] ^= 0x01;
    if (verify(originalScriptHeader.data(), originalScriptHeader.size(),
               "a1a605515b9784d09063bb206240f5b7"
               "83e219a3cf3fdae0f7f8df6d6256b54b")) {
        std::cerr << "original stage SHA256: modified Lua bytes accepted\n";
        return 1;
    }
    std::uint8_t digest[32] = {};
    if (!ae3::originalSourceSha256(data, size, digest))
        return false;
    constexpr char hexadecimal[] = "0123456789abcdef";
    char text[64];
    for (std::size_t i = 0; i < 32; ++i) {
        text[2*i] = hexadecimal[digest[i] >> 4];
        text[2*i+1] = hexadecimal[digest[i] & 15];
    }
    return expected == std::string_view(text, sizeof(text));
}
} // namespace

int main() {
    // NIST-known SHA-256 vectors and a deterministic Lua header fixture.
    if (!verify("", 0,
                "e3b0c44298fc1c149afbf4c8996fb924"
                "27ae41e4649b934ca495991b7852b855")) {
        std::cerr << "original stage SHA256: empty input failed\n";
        return 1;
    }
    if (!verify("abc", 3,
                "ba7816bf8f01cfea414140de5dae2223"
                "b00361a396177a9cb410ff61f20015ad")) {
        std::cerr << "original stage SHA256: abc input failed\n";
        return 1;
    }
    std::array<std::uint8_t, 45> originalScriptHeader{};
    originalScriptHeader[0] = 0x1b;
    originalScriptHeader[1] = 'L';
    originalScriptHeader[2] = 'u';
    originalScriptHeader[3] = 'a';
    originalScriptHeader[4] = 0x50;
    for (std::size_t i=5; i<originalScriptHeader.size(); ++i)
        originalScriptHeader[i] = static_cast<std::uint8_t>(i-5);
    if (!verify(originalScriptHeader.data(), originalScriptHeader.size(),
                "a1a605515b9784d09063bb206240f5b7"
                "83e219a3cf3fdae0f7f8df6d6256b54b")) {
        std::cerr << "original stage SHA256: synthetic header failed\n";
        return 1;
    }
    std::uint8_t digest[32] = {};
    if (ae3::originalSourceSha256(nullptr, 1, digest) ||
        ae3::originalSourceSha256("x",
            static_cast<std::size_t>(std::numeric_limits<ULONG>::max())+1,
            digest)) {
        std::cerr << "original stage SHA256: unsafe input was accepted\n";
        return 1;
    }
    std::cout << "ORIGINAL_STAGE_WINDOWS_SHA256_TEST_PASSED\n";
    return 0;
}
