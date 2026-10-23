#pragma once
// Owner-original AE3 stage integrity, Win64 CNG SHA-256 implementation.
//
// This is a source-file authenticity check: it does not perform game
// emulation and does not contain retail data. The function is shared
// verbatim between native Windows CTest and the Unreal stage loader.
//
// Do not use FPlatformMisc::GetSHA256Signature. Its generic Windows fallback
// can assert instead of providing a hash in supported UE versions.
#if !defined(_WIN32)
#error Original stage Win64 SHA-256 helper is only compiled on Windows
#endif

#include <windows.h>
#include <bcrypt.h>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace ae3 {
inline bool originalSourceSha256(
    const void* Data, const std::size_t Size, std::uint8_t (&Digest)[32]) noexcept
{
    if ((Data == nullptr && Size != 0) ||
        Size > std::numeric_limits<ULONG>::max())
        return false;
    BCRYPT_ALG_HANDLE Provider = nullptr;
    const NTSTATUS OpenStatus = BCryptOpenAlgorithmProvider(
        &Provider, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
    if (OpenStatus < 0 || Provider == nullptr)
        return false;

    const NTSTATUS HashStatus = BCryptHash(
        Provider, nullptr, 0,
        reinterpret_cast<PUCHAR>(const_cast<void*>(Data)),
        static_cast<ULONG>(Size), Digest,
        static_cast<ULONG>(sizeof(Digest)));
    const NTSTATUS CloseStatus = BCryptCloseAlgorithmProvider(Provider, 0);
    if (HashStatus < 0 || CloseStatus < 0) {
        for (auto& Byte : Digest)
            Byte = 0;
        return false;
    }
    return true;
}
} // namespace ae3
