#pragma once
#include "ae3/archive.hpp"
#include "ae3/original_resource_package.hpp"
#include "ae3/original_resource_sz.hpp"
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace ae3 {
// Ownership path: original USA ISO or DATA.BIN -> VFI index -> original PCK
// allocation -> original 16-byte typed resource entries. No stage-selection
// heuristic or replacement asset is introduced. Each package owns its source
// bytes independently of the ISO stream's lifetime.
class OriginalResourceVfiMount final {
public:
    explicit OriginalResourceVfiMount(const std::filesystem::path& ownedUsaIsoOrDataBin)
        : archive_(ownedUsaIsoOrDataBin) {}
    OriginalResourceVfiMount(const OriginalResourceVfiMount&) = delete;
    OriginalResourceVfiMount& operator=(const OriginalResourceVfiMount&) = delete;
    OriginalResourceVfiMount(OriginalResourceVfiMount&&) = delete;
    OriginalResourceVfiMount& operator=(OriginalResourceVfiMount&&) = delete;

    const std::vector<VfiEntry>& vfiEntries() const noexcept { return archive_.entries(); }
    // Exact source VFI member path required. Caller must derive the stage's
    // actual filename from original stage tables, not infer from location.
    std::unique_ptr<OriginalPckResourcePackage> openPck(const std::string& originalVfiPath) {
        const auto index=archive_.find(originalVfiPath);
        if (!index) throw std::out_of_range("Original stage PCK not present in USA VFI index");
        const auto raw=readOriginalFile(originalVfiPath);
        std::vector<std::byte> owned;
        owned.reserve(raw.size());
        for (const auto value:raw) owned.push_back(std::byte{value});
        return std::make_unique<OriginalPckResourcePackage>(std::move(owned));
    }
    // Never guess alternate filenames or invisibly replace the stored bytes.
    // The explicit *.sz suffix in the original VFI entry opts into retail SZ.
    std::vector<std::uint8_t> readOriginalFile(const std::string& originalVfiPath) {
        const auto index=archive_.find(originalVfiPath);
        if (!index) throw std::out_of_range("Original file not present in USA VFI index");
        auto stored=archive_.read(archive_.entries().at(*index));
        if (isOriginalSzName(archive_.entries().at(*index).path))
            return originalDecodeSz(stored);
        return stored;
    }
    // Byte-exact VFI source read for SHA validation and differential decoding.
    std::vector<std::uint8_t> readStoredVfiMember(const std::string& originalVfiPath) {
        const auto index=archive_.find(originalVfiPath);
        if (!index) throw std::out_of_range("Original file not present in USA VFI index");
        return archive_.read(archive_.entries().at(*index));
    }
private:
    static bool isOriginalSzName(const std::string& path) noexcept {
        if (path.size()<3) return false;
        const auto tail=path.substr(path.size()-3);
        return tail==".sz" || tail==".SZ" || tail==".Sz" || tail==".sZ";
    }

    Archive archive_;
};
} // namespace ae3
