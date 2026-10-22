#include "ae3/original_windows_static_resources.hpp"
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace ae3::runtime {
std::array<std::byte,kStaticLiteralSourceBytes_3a6cd0>
originalReadStaticSourceFromElf_3a6cd0(const std::filesystem::path& file){
    std::error_code ec;
    const auto size=std::filesystem::file_size(file,ec);
    if(ec || size!=6488148u)
        throw std::runtime_error("Original USA ELF file length mismatch");
    std::ifstream in(file,std::ios::binary);
    if(!in)throw std::runtime_error("Original USA ELF cannot be opened");
    std::vector<std::uint8_t> data(static_cast<std::size_t>(size));
    if(!in.read(reinterpret_cast<char*>(data.data()),
                static_cast<std::streamsize>(data.size())))
        throw std::runtime_error("Incomplete original USA ELF read");
    if(data.size()<52 || data[0]!=0x7f || data[1]!='E' || data[2]!='L' ||
       data[3]!='F' || data[4]!=1 || data[5]!=1 || data[6]!=1 ||
       data[18]!=8 || data[19]!=0)
        throw std::runtime_error("Expected original USA little-endian MIPS ELF32");
    auto word16=[&](std::size_t at)->std::uint16_t{
        if(at>data.size()||data.size()-at<2)throw std::runtime_error("ELF header truncated");
        return std::uint16_t(data[at])|(std::uint16_t(data[at+1])<<8);
    };
    auto word32=[&](std::size_t at)->std::uint32_t{
        if(at>data.size()||data.size()-at<4)throw std::runtime_error("ELF header truncated");
        return std::uint32_t(data[at])|(std::uint32_t(data[at+1])<<8)|
            (std::uint32_t(data[at+2])<<16)|(std::uint32_t(data[at+3])<<24);
    };
    if(word32(24)!=0x100134u)
        throw std::runtime_error("Original USA ELF entrypoint mismatch");
    const std::size_t phoff=word32(28);
    const std::size_t phsize=word16(42),count=word16(44);
    if(phsize<32 || !count || count>128 || phoff>data.size() ||
       count>(data.size()-phoff)/phsize)
        throw std::runtime_error("Original USA ELF program headers out of range");
    for(std::size_t i=0;i<count;++i){
        const std::size_t p=phoff+i*phsize;
        if(word32(p)!=1)continue;
        const auto offset=word32(p+4),vaddr=word32(p+8),length=word32(p+16);
        const std::uint64_t start=kStaticLiteralSourceVA_3a6cd0;
        const std::uint64_t finish=start+kStaticLiteralSourceBytes_3a6cd0;
        if(start<vaddr || finish>std::uint64_t(vaddr)+length)continue;
        const std::uint64_t rawOffset=std::uint64_t(offset)+(start-vaddr);
        if(rawOffset>data.size()||
           kStaticLiteralSourceBytes_3a6cd0>data.size()-rawOffset)
            throw std::runtime_error("Original VA mapping exceeds ELF file bounds");
        std::array<std::byte,kStaticLiteralSourceBytes_3a6cd0> original{};
        std::memcpy(original.data(),data.data()+rawOffset,original.size());
        if(originalStaticSourceFingerprint_3a6cd0(original)!=
               UINT64_C(0xaff2510d8c444e7e))
            throw std::runtime_error("Original constructor literal region fingerprint mismatch");
        return original;
    }
    throw std::runtime_error("Original 0x720040 literal VA absent from USA executable");
}
} // namespace ae3::runtime
