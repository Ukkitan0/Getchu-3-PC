#include "ae3/original_navgraph_loader.hpp"
#include "ae3/original_nav_source_refs.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>
#include <zlib.h>

namespace ae3 {
std::size_t originalNavGraphSlot_4f0fbc(std::string_view name) noexcept {
    if(name=="nav_monkey.nav")return 0;
    if(name=="nav_enemy.nav")return 1;
    // The actual 0x4F0FBC returns 2 when both first comparisons fail;
    // it compares nav_extra0.nav at 0x4F1008 but discards that result.
    return 2;
}
std::vector<std::uint8_t> originalNavInflateSZ(
    std::span<const std::uint8_t> src) {
    if(src.size()<6)throw std::invalid_argument("Original SZ stream too small");
    const std::uint32_t outSize=std::uint32_t(src[0])|
        (std::uint32_t(src[1])<<8)|(std::uint32_t(src[2])<<16)|
        (std::uint32_t(src[3])<<24);
    constexpr std::uint32_t maxSz=128u*1024u*1024u;
    if(outSize>maxSz)throw std::out_of_range("Original SZ uncompressed size exceeds native safety limit");
    if(src.size()-4>std::numeric_limits<uInt>::max())
        throw std::out_of_range("Original SZ compressed member too large");
    std::vector<std::uint8_t> out(outSize);
    z_stream stream{};
    stream.next_in=const_cast<Bytef*>(reinterpret_cast<const Bytef*>(src.data()+4));
    stream.avail_in=static_cast<uInt>(src.size()-4);
    stream.next_out=reinterpret_cast<Bytef*>(out.data());
    stream.avail_out=static_cast<uInt>(out.size());
    if(inflateInit2(&stream,-MAX_WBITS)!=Z_OK)
        throw std::runtime_error("Raw DEFLATE initialization failed");
    const int status=inflate(&stream,Z_FINISH);
    const auto produced=stream.total_out;
    inflateEnd(&stream);
    if(status!=Z_STREAM_END || produced!=outSize)
        throw std::runtime_error("Original SZ raw DEFLATE truncated or size mismatch");
    return out;
}
std::vector<OriginalNavSource> originalNavReadPackage(
    std::span<const std::uint8_t> originalSZ) {
    const auto bytes=originalNavInflateSZ(originalSZ);
    const auto read32=[&](std::size_t i)->std::uint32_t {
        if(i>bytes.size()||bytes.size()-i<4)
            throw std::out_of_range("Original PCK truncated 32-bit header");
        return std::uint32_t(bytes[i])|(std::uint32_t(bytes[i+1])<<8)|
            (std::uint32_t(bytes[i+2])<<16)|(std::uint32_t(bytes[i+3])<<24);
    };
    if(bytes.size()<12 || read32(0)!=0x004b4350)
        throw std::invalid_argument("Original SZ does not contain PCK archive");
    const auto table=static_cast<std::size_t>(read32(4));
    const auto count=static_cast<std::size_t>(read32(8));
    if(table>bytes.size() || count>(bytes.size()-table)/16)
        throw std::out_of_range("Original PCK navigation file table truncated");
    // The retail navigation packages use ABSOLUTE PCK name offsets,
    // e.g. 0x22 for nav_monkey.nav. The older generic PCK utility
    // currently assumes offsets relative to +0x10 and is not suitable.
    const auto nameAt=[&](std::size_t offset)->std::string {
        if(offset<12 || offset>=table)
            throw std::out_of_range("Original PCK nav name pointer invalid");
        const auto end=std::find(bytes.begin()+offset,bytes.begin()+table,0);
        if(end==bytes.begin()+table)throw std::invalid_argument("Unterminated original PCK nav name");
        return {bytes.begin()+offset,end};
    };
    std::vector<OriginalNavSource> result;
    result.reserve(count);
    for(std::size_t i=0;i<count;i++){
        const std::size_t at=table+i*16;
        const auto name=nameAt(read32(at));
        const auto payload=static_cast<std::size_t>(read32(at+8));
        const auto size=static_cast<std::size_t>(read32(at+12));
        if(payload>bytes.size() || size>bytes.size()-payload ||
           payload<table+count*16)
            throw std::out_of_range("Original PCK nav payload invalid");
        if(name.size()<4 || name.compare(name.size()-4,4,".nav")!=0)continue;
        OriginalNavSource source;
        source.originalFilename=name;
        source.originalSlot=originalNavGraphSlot_4f0fbc(name);
        source.originalNavBytes.assign(bytes.begin()+payload,bytes.begin()+payload+size);
        result.push_back(std::move(source));
    }
    return result;
}
std::vector<OriginalNavSource> originalNavReadPackage(
    Archive& archive,const VfiEntry& entry) {
    if(entry.path.size()<17 ||
       entry.path.compare(entry.path.size()-17,17,"navigation.pck.sz")!=0)
        throw std::invalid_argument("Expected original VFI navigation.pck.sz entry");
    return originalNavReadPackage(archive.read(entry));
}
OriginalStageSpatialLoaded originalNavLoadGraph_4f0a5c(
    const OriginalNavSource& src,std::uint32_t originalVA,
    std::uint32_t ownerVA) {
    if(src.originalNavBytes.empty()||!originalVA)
        throw std::invalid_argument("Missing original loaded navigation allocation or VA");
    // The actual 0x4F0B14 invokes 0x21D46C(slot, source_pointer).
    // 0x21D46C is source-translated by original_stage_spatial_loader.cpp.
    std::span<const std::uint8_t> bytes(src.originalNavBytes);
    const auto raw=std::as_bytes(bytes);
    OriginalStageSpatialSource originalSource{originalVA,raw,ownerVA};
    auto originalGraph=originalStageBuildSourceSpatial_21d46c(originalSource);
    // Original 0x21D6DC resolves type-2 source table pointers directly
    // into record+0x10+4*actionIndex. Type-1 child allocations remain
    // unported and are retained in pendingOriginalActions.
    (void)originalNavResolveType2_21d6dc(originalSource,originalGraph);
    return originalGraph;
}
} // namespace ae3
