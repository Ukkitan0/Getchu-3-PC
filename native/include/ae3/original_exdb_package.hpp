#pragma once
#include "ae3/original_attribute_record_index.hpp"
#include "ae3/original_attribute_string.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace ae3 {
// Validated host view of the original PCK file byte layout, not a port
// of an as-yet-unlocated retail PCK reader routine. Source bytes are
// externally owned and remain unchanged. Original woods_c/exdb.pck:
// PCK\0, table offset +0x04, count +0x08, and 16-byte table items:
// name string offset, type string offset, data offset, data byte length.
struct OriginalExdbPackageEntry {
    std::string_view name;
    std::string_view type;
    std::span<const std::byte> originalBytes;
    std::uint32_t packageDataOffset=0;
};
inline std::uint32_t originalPckReadLE32(std::span<const std::byte> raw, std::size_t at) {
    if(at>raw.size()||raw.size()-at<4)throw std::out_of_range("Retail PCK field truncated");
    std::uint32_t v=0;
    for(unsigned i=0;i<4;++i)v|=std::uint32_t(std::to_integer<unsigned char>(raw[at+i]))<<(8*i);
    return v;
}
inline std::string_view originalPckReadName(std::span<const std::byte> raw,
                                          std::size_t at,std::size_t stringsEnd) {
    if(at<0x10||at>=stringsEnd||stringsEnd>raw.size())
        throw std::out_of_range("Retail PCK item string offset invalid");
    const auto start=raw.begin()+static_cast<std::ptrdiff_t>(at);
    const auto end=raw.begin()+static_cast<std::ptrdiff_t>(stringsEnd);
    const auto terminator=std::find(start,end,std::byte{0});
    if(terminator==end)throw std::out_of_range("Retail PCK item string not NUL terminated");
    return {reinterpret_cast<const char*>(&*start),static_cast<std::size_t>(terminator-start)};
}
inline std::vector<OriginalExdbPackageEntry> originalExdbPackageEntriesFromPck(
        std::span<const std::byte> bytes) {
    if(bytes.size()<0x10 || bytes[0]!=std::byte{'P'} ||
       bytes[1]!=std::byte{'C'} || bytes[2]!=std::byte{'K'} || bytes[3]!=std::byte{0})
        throw std::invalid_argument("Not an original PCK resource package");
    const std::size_t table=originalPckReadLE32(bytes,4);
    const std::size_t count=originalPckReadLE32(bytes,8);
    if(table<0x10||table>bytes.size()||count>(bytes.size()-table)/16)
        throw std::out_of_range("Original PCK item table is truncated");
    std::vector<OriginalExdbPackageEntry> entries;
    entries.reserve(count);
    for(std::size_t i=0;i<count;++i){
        const std::size_t pos=table+i*16;
        const auto nameOffset=originalPckReadLE32(bytes,pos);
        const auto typeOffset=originalPckReadLE32(bytes,pos+4);
        const auto dataOffset=originalPckReadLE32(bytes,pos+8);
        const auto dataSize=originalPckReadLE32(bytes,pos+12);
        if(dataOffset>bytes.size()||dataSize>bytes.size()-dataOffset ||
           dataOffset<table+count*16)
            throw std::out_of_range("Original PCK data offset/length invalid");
        entries.push_back({originalPckReadName(bytes,nameOffset,table),
            originalPckReadName(bytes,typeOffset,table),
            bytes.subspan(dataOffset,dataSize),dataOffset});
    }
    return entries;
}

// Original stage audio path 0x35FCBC requests type "exdb";
// 0x35FD20 accepts only name "bgm_desc.exdb" before 0x1BB63C.
// The caller obtains the actual data pointer at entry+0x08 and byte
// count at +0x0C. We retain those borrowed bytes, not guest pointers.
inline constexpr std::string_view OriginalBgmExdbName_35fd20="bgm_desc.exdb";
inline constexpr std::string_view OriginalBgmExdbType_35fcbc="exdb";
inline std::vector<OriginalExdbPackageEntry> originalBgmExdbMatchingEntries_35fd20(
        std::span<const OriginalExdbPackageEntry> originalEntries) {
    std::vector<OriginalExdbPackageEntry> result;
    for(const auto& e:originalEntries)
        if(e.type==OriginalBgmExdbType_35fcbc &&
           e.name==OriginalBgmExdbName_35fd20)result.push_back(e);
    return result;
}

// The already recovered original 0x2BB03C textual attribute parser
// is now supplied with *real PCK payload bytes*. This object owns the
// host key tree but NOT the retail bytes. The original +0x04 pointer
// is represented as a source-relative byte offset with guest base 0.
class OriginalExdbDecodedDocument final {
public:
    void decode(std::span<const std::byte> originalExdbPayload) {
        decoded_=false;
        const std::string_view text(reinterpret_cast<const char*>(originalExdbPayload.data()),
                                    originalExdbPayload.size());
        auto parsed=originalAttributeBindLoadedText_2bb03c(tree_,text,0);
        const auto index=originalAttributeRecordIndexFromParser_2bb0f4(parsed.parsed);
        if(index.sourceCountAt20<0||index.sourceStrideAt24<=0)
            throw std::invalid_argument("Invalid original EXDB count or record stride");
        const std::uint32_t begin=index.sourceRecordBaseAt04;
        const std::size_t stride=static_cast<std::size_t>(index.sourceStrideAt24);
        if(begin>originalExdbPayload.size() ||
           static_cast<std::size_t>(index.sourceCountAt20)>(originalExdbPayload.size()-begin)/stride)
            throw std::out_of_range("Original EXDB record extent outside loaded asset");
        // Check all record ranges now, so successful decode has a
        // bounded payload view even if a later getter is not called.
        for(std::uint32_t i=0;i<static_cast<std::uint32_t>(index.sourceCountAt20);++i)
            (void)originalAttributeRecordBytesChecked_2bb0f4(originalExdbPayload,0,index,
                i,static_cast<std::size_t>(index.sourceStrideAt24));
        payload_=originalExdbPayload;
        index_=index;
        decoded_=true;
    }
    std::span<const std::byte> record(std::uint32_t originalIndex) const {
        if(!decoded_)throw std::logic_error("Original EXDB source not decoded");
        return originalAttributeRecordBytesChecked_2bb0f4(payload_,0,index_,originalIndex,
            static_cast<std::size_t>(index_.sourceStrideAt24));
    }
    const OriginalAttributeKeyNode* decodedAttributeRoot() const noexcept {
        return decoded_?tree_.decodedRoot():nullptr;
    }
    OriginalAttributeRecordIndex_2bb0f4 originalRecordIndex() const {
        if(!decoded_)throw std::logic_error("Original EXDB source not decoded");
        return index_;
    }
private:
    std::span<const std::byte> payload_{};
    OriginalAttributeTreeOwner tree_{};
    OriginalAttributeRecordIndex_2bb0f4 index_{};
    bool decoded_=false;
};
} // namespace ae3
