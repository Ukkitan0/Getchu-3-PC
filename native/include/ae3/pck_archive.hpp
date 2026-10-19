#pragma once
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace ae3 {
enum class PckAssetKind { mesh, animation, texture, other };
struct PckMember {
    std::size_t index = 0;
    std::string name;
    std::uint32_t type = 0,offset = 0,size = 0;
    PckAssetKind kind = PckAssetKind::other;
};
// Data format recovered from the owner's Exporter.zip and independently
// validated against its actual bg.pck file. Parsing game DATA is not
// CPU/IOP/GS hardware emulation. Do not publish extracted game assets.
class PckArchive {
public:
    explicit PckArchive(const std::filesystem::path& source);
    explicit PckArchive(std::vector<std::uint8_t> data);
    const std::vector<PckMember>& entries() const {return members_;}
    std::vector<std::uint8_t> read(const PckMember& item) const;
    void extractTo(const std::filesystem::path& folder) const;
private:
    std::vector<std::uint8_t> bytes_;
    std::vector<PckMember> members_;
    void parse();
};
const char* pckExtension(PckAssetKind kind);
}
