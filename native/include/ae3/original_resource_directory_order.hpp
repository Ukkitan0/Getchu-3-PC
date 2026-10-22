#pragma once
#include "ae3/original_resource_stage_catalog.hpp"
#include <algorithm>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace ae3 {
// The original 0x25182C caller queries a directory's ordered tree
// through 0x509C10, walks its records via 0x49C150 and invokes
// 0x24D858. Original key comparator 0x627E60 compares source string
// bytes and then their lengths. This is a host-side directory VIEW
// over files genuinely present in VFI, not a reconstruction of the
// original DirInfo tree nodes, owner allocator or original path policy.
inline bool originalResourceDirectoryKeyLess_627e60(
         std::string_view lhs,std::string_view rhs) noexcept {
    const auto common=std::min(lhs.size(),rhs.size());
    for(std::size_t i=0;i<common;++i) {
        const auto a=static_cast<unsigned char>(lhs[i]);
        const auto b=static_cast<unsigned char>(rhs[i]);
        if(a!=b)return a<b;
    }
    return lhs.size()<rhs.size();
}
struct OriginalStageDirectoryVfiEntry_251958 {
    std::size_t originalCatalogIndex=0;
    std::string_view originalStageRelativePath;
    std::string_view originalLeafFilename;
};
// An explicit directory path prevents inventing where the original
// registration worker starts traversal. Return direct child FILES only
// in recovered source ordered-tree key sequence, preserving the original
// full source filenames and their corresponding actual VFI indices.
inline std::vector<OriginalStageDirectoryVfiEntry_251958>
originalStageDirectoryVfiView_251958(
          const OriginalStageResourceCatalog& catalog,
          std::string_view originalStageName,
          std::string_view explicitStageRelativeDirectory = {}) {
    if(!originalResourceValidStageName(originalStageName))
        throw std::invalid_argument("Original stage identifier is invalid");
    if(!explicitStageRelativeDirectory.empty() &&
       !originalResourceValidStageRelativeName(explicitStageRelativeDirectory))
        throw std::invalid_argument("Invalid original stage source directory");
    std::vector<OriginalStageDirectoryVfiEntry_251958> result;
    const auto expectedPrefix=explicitStageRelativeDirectory.empty()
                            ?std::string{}:std::string(explicitStageRelativeDirectory)+"/";
    for(const auto index:catalog.entriesForStage(originalStageName)) {
        const auto& file=catalog.entries()[index];
        const auto relative=std::string_view(file.stageRelativePath);
        if(!relative.starts_with(expectedPrefix))continue;
        const auto leaf=relative.substr(expectedPrefix.size());
        if(leaf.empty() || leaf.find('/')!=std::string_view::npos)continue;
        result.push_back({index,relative,leaf});
    }
    std::sort(result.begin(),result.end(),
              [](const auto& a,const auto& b) {
                  return originalResourceDirectoryKeyLess_627e60(
                      a.originalLeafFilename,b.originalLeafFilename);
              });
    return result;
}
} // namespace ae3
