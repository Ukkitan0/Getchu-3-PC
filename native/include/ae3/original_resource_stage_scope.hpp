#pragma once
#include "ae3/original_resource_stage_catalog.hpp"
#include "ae3/original_exdb_package.hpp"
#include <cstddef>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ae3 {
// Lifetime boundary for a single *actual* indexed USA stage folder.
// This is native ownership, not a recovered retail scene transition,
// original EE address allocator, or a replacement stage loader.
//
// All returned spans/views/references are borrowed; consumers MUST keep
// this scope alive. Destroy the scope only after graphics/Lua/nav/actor
// consumers have released their borrowed references.
class OriginalStageResourceScope final {
public:
    OriginalStageResourceScope(const std::filesystem::path& originalIsoOrDataBin,
                               std::string originalStage)
        : mount_(originalIsoOrDataBin),
          catalog_(mount_.vfiEntries()),
          stage_(std::move(originalStage)) {
        if (catalog_.entriesForStage(stage_).empty())
            throw std::out_of_range("Stage folder absent from original USA VFI archive");
    }
    OriginalStageResourceScope(const OriginalStageResourceScope&) = delete;
    OriginalStageResourceScope& operator=(const OriginalStageResourceScope&) = delete;
    OriginalStageResourceScope(OriginalStageResourceScope&&) = delete;
    OriginalStageResourceScope& operator=(OriginalStageResourceScope&&) = delete;

    std::string_view stageName() const noexcept { return stage_; }
    const OriginalStageResourceCatalog& catalog() const noexcept { return catalog_; }
    std::vector<std::size_t> originalStageVfiIndices() const {
        return catalog_.entriesForStage(stage_);
    }

    // Stable source-backed bytes until this stage scope is destroyed.
    // A VFI entry named "*.sz" is uncompressed with the prior-verified
    // source SZ layout; all other files retain their exact stored bytes.
    std::span<const std::uint8_t> file(std::string_view relativePath) {
        const auto& entry = requireVfi(relativePath);
        const auto key = entry.sourceEntry.path;
        if (auto it = files_.find(key); it!=files_.end())
            return *it->second;
        auto owned = std::make_unique<std::vector<std::uint8_t>>(
            mount_.readOriginalFile(key));
        const auto* stable = owned.get();
        files_.emplace(key, std::move(owned));
        return *stable;
    }

    // Stable typed PCK resource catalog with owned original bytes.
    // Strictly requires "*.pck" or "*.pck.sz" *in the actual VFI index*.
    const OriginalPckResourcePackage& package(std::string_view relativePath) {
        const auto& entry = requireVfi(relativePath);
        const auto folded = originalResourceAsciiFold(entry.uncompressedName);
        if (!std::string_view(folded).ends_with(".pck"))
            throw std::invalid_argument("Requested original stage entry is not PCK");
        const auto key = entry.sourceEntry.path;
        if (auto it = packages_.find(key); it!=packages_.end())
            return *it->second;
        auto owned = mount_.openPck(key);
        auto* stable = owned.get();
        packages_.emplace(key,std::move(owned));
        return *stable;
    }

    const OriginalPckResourceView& requireMember(std::string_view packagePath,
                                                  std::string_view sourceName,
                                                  std::string_view sourceType) {
        return package(packagePath).require(sourceName,sourceType);
    }
    // 0x26D24C's recovered original type-selection boundary. Never
    // silently substitute members from a different stage/package.
    std::vector<std::size_t> filterMembers_26d24c(
            std::string_view packagePath,std::string_view requestedType) {
        return package(packagePath).selectType_26d24c(requestedType);
    }

    // Original 0x2BB03C/0x2BB0F4 attribute parser. The EXDB document owns
    // its decoded key tree, and borrows only payload bytes from our owned
    // PCK. Documents are destroyed BEFORE PCKs during scope teardown.
    const OriginalExdbDecodedDocument& exdb(std::string_view packagePath,
                                            std::string_view originalExdbName) {
        const auto& entry = requireVfi(packagePath);
        const auto& pck = package(packagePath);
        const auto index = pck.first(originalExdbName,"exdb");
        if (!index)throw std::out_of_range("Original EXDB name/type not in source package");
        const auto& key = entry.sourceEntry.path;
        for(const auto& decoded:documents_)
            if(decoded.sourcePck==key && decoded.sourceIndex==*index)
                return *decoded.document;
        auto document = std::make_unique<OriginalExdbDecodedDocument>();
        document->decode(pck.entries()[*index].payload);
        auto* stable = document.get();
        documents_.push_back({key,*index,std::move(document)});
        return *stable;
    }

    std::size_t loadedRawFileCount() const noexcept {return files_.size();}
    std::size_t loadedPackageCount() const noexcept {return packages_.size();}
    std::size_t decodedExdbCount() const noexcept {return documents_.size();}

private:
    const OriginalStageVfiResource& requireVfi(std::string_view path) const {
        const auto index=catalog_.findExact(stage_,path);
        if(!index)throw std::out_of_range("Original stage VFI resource not found");
        return catalog_.entries()[*index];
    }
    struct DecodedExdbOwner {
        std::string sourcePck;
        std::size_t sourceIndex=0;
        std::unique_ptr<OriginalExdbDecodedDocument> document;
    };
    OriginalResourceVfiMount mount_;
    OriginalStageResourceCatalog catalog_;
    std::string stage_;
    std::unordered_map<std::string,std::unique_ptr<std::vector<std::uint8_t>>> files_;
    std::unordered_map<std::string,std::unique_ptr<OriginalPckResourcePackage>> packages_;
    std::deque<DecodedExdbOwner> documents_;
};
} // namespace ae3
