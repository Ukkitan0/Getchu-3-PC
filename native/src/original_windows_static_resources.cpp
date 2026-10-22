#include "ae3/original_windows_static_resources.hpp"
#include <algorithm>
namespace ae3::runtime {
std::uint64_t originalStaticSourceFingerprint_3a6cd0(
    std::span<const std::byte> source) noexcept {
    std::uint64_t result=UINT64_C(14695981039346656037);
    for(auto value:source){
        result^=static_cast<std::uint8_t>(value);
        result*=UINT64_C(1099511628211);
    }
    return result;
}
bool originalStaticCtor_3a6cd0(
    std::span<const std::byte> originalSource,
    OriginalStaticResourceGlobals_3a6cd0& destination) noexcept {
    if(originalSource.size()!=kStaticLiteralSourceBytes_3a6cd0 ||
       originalStaticSourceFingerprint_3a6cd0(originalSource)!=
          UINT64_C(0xaff2510d8c444e7e))return false;
    OriginalStaticResourceGlobals_3a6cd0 result{};
    // 0x3A6BE0..0x3A6BF0 original four LQ/SQ copies:
    // 0x720040..0x72007F -> 0x745F40..0x745F7F.
    for(std::size_t i=0;i<result.globalLiterals.size();++i)
        std::copy_n(originalSource.begin()+i*16,16,
                    result.globalLiterals[i].begin());
    // 0x3A6BF4..0x3A6CB0: nine genuine 16-byte source literals,
    // paired with original stack type-word writes, not random padding.
    constexpr std::array<std::uint32_t,9> kTypeWords={1,1,1,2,1,1,4,3,2};
    for(std::size_t i=0;i<result.entries.size();++i){
        std::copy_n(originalSource.begin()+0x40+i*16,16,
                    result.entries[i].sourceLiteral.begin());
        result.entries[i].typeWord=kTypeWords[i];
    }
    result.initialized=true;
    destination=result;
    return true;
}
}
