#pragma once
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace ae3 {
// Authentic AE3 VFI '*.sz' member: little-endian u32 expanded byte length
// followed by raw RFC1951 DEFLATE (no zlib or gzip wrapper).
// Verified original owner-local ISO extraction: native/tools/extract_retail_iso.py
// and extract_original_stage.py. The original EE loader VA is not yet recovered.
inline constexpr std::size_t OriginalSzMaxOutputBytes = 512u*1024u*1024u;
std::vector<std::uint8_t> originalDecodeSz(
    std::span<const std::uint8_t> originalVfiMember,
    std::size_t maxOutputBytes = OriginalSzMaxOutputBytes);
}
