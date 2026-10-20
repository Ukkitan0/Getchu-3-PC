#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace ae3 {
// Transcode the VERIFIED ORIGINAL SCUS_975.01 Lua 5.0 binary format
// (little-endian int32, size_t32, float32) to a host Lua 5.0.3 x64
// binary chunk (int32, size_t64, double64).
//
// This is a DATA REPRESENTATION CONVERSION: the original Lua opcode
// stream, function graph, local names and script logic are unchanged.
// The resulting code executes with Lua 5.0 on the host CPU, WITHOUT
// interpreting MIPS/PS2 EE hardware.
//
// Input format verified against 168 area.luc files from an owned ISO;
// convert on the host at the time each original script is loaded.
// Throws std::runtime_error on invalid/truncated/unsupported formats.
struct RetailLua50Stats {
    std::size_t prototypes=0;
    std::size_t constants=0;
    std::size_t instructions=0;
};
std::vector<std::uint8_t> retargetRetailLua50(
    std::string_view retailChunk, RetailLua50Stats* stats=nullptr);
} // namespace ae3
