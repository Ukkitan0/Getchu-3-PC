#pragma once
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

namespace ae3 {
// This is a native DATA.BIN reader, NOT a simulated PS2 filesystem.
// ISO files are treated as ordinary host files; the host OS reads the bytes.
struct Span { std::uint64_t offset = 0, size = 0; };

class DiskFile {
public:
    explicit DiskFile(const std::filesystem::path& path);
    std::vector<std::uint8_t> read(std::uint64_t offset, std::size_t length);
    std::uint64_t size() const { return length_; }
private:
    std::ifstream file_;
    std::uint64_t length_ = 0;
};

struct VfiEntry {
    std::string path;
    std::uint64_t offset = 0;
    std::uint32_t size = 0;
};

class Archive {
public:
    // Accepts a directly extracted DATA.BIN or a USA Ape Escape 3 ISO.
    // No PS2 firmware, CPU, IOP, DMA or graphics system is executed or simulated.
    explicit Archive(const std::filesystem::path& ownedGameData);
    const std::vector<VfiEntry>& entries() const { return entries_; }
    const std::string& sourceKind() const { return kind_; }
    std::vector<std::uint8_t> read(const VfiEntry& entry);
    // Stream the owner's original DATA.BIN out of an ISO for an independent
    // offline asset converter. Never invokes any console execution runtime.
    // Refuses to overwrite files, including the source input.
    void extractDataBin(const std::filesystem::path& destination);
    std::optional<std::size_t> find(const std::string& relativePath) const;
    const std::filesystem::path& source() const { return source_; }
private:
    DiskFile input_;
    std::filesystem::path source_;
    std::string kind_;
    Span data_{};
    std::vector<VfiEntry> entries_;
};

std::string assetType(const std::string& name);
} // namespace ae3
