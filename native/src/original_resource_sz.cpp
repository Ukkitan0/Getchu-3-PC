#include "ae3/original_resource_sz.hpp"
#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>
#include <string>
#include <zlib.h>

namespace ae3 {
namespace {
class InflateOwner final {
public:
    InflateOwner() {
        // Original SZ stores naked raw-DEFLATE bytes (no zlib Adler32 header/trailer).
        if(inflateInit2(&state_,-MAX_WBITS)!=Z_OK)
            throw std::runtime_error("Cannot initialize original SZ native DEFLATE decoder");
    }
    ~InflateOwner() { (void)inflateEnd(&state_); }
    InflateOwner(const InflateOwner&)=delete;
    InflateOwner& operator=(const InflateOwner&)=delete;
    z_stream& stream() noexcept { return state_; }
private:
    z_stream state_{};
};
std::uint32_t readSize(std::span<const std::uint8_t> source) {
    if(source.size()<4) throw std::out_of_range("Truncated original SZ uncompressed-size header");
    return std::uint32_t(source[0]) | (std::uint32_t(source[1])<<8) |
           (std::uint32_t(source[2])<<16) | (std::uint32_t(source[3])<<24);
}
}
std::vector<std::uint8_t> originalDecodeSz(std::span<const std::uint8_t> compressed,
                                            std::size_t maxOutputBytes) {
    const std::uint32_t expanded=readSize(compressed);
    if(expanded>maxOutputBytes)
        throw std::length_error("Original SZ uncompressed member exceeds permitted allocation budget");
    InflateOwner decoder;
    auto& stream=decoder.stream();
    std::vector<std::uint8_t> original;
    // Do NOT trust the source's expanded count enough to allocate a huge vector
    // up front. Grow only when the underlying raw DEFLATE stream produces bytes.
    std::array<std::uint8_t,64*1024> block{};
    const auto data=compressed.subspan(4);
    std::size_t fed=0;
    while(true) {
        if(stream.avail_in==0 && fed<data.size()) {
            const std::size_t take=std::min<std::size_t>(
                data.size()-fed,std::numeric_limits<uInt>::max());
            stream.next_in=const_cast<Bytef*>(reinterpret_cast<const Bytef*>(data.data()+fed));
            stream.avail_in=static_cast<uInt>(take);
            fed+=take;
        }
        stream.next_out=block.data();
        stream.avail_out=static_cast<uInt>(block.size());
        const auto before=stream.avail_in;
        const auto rc=inflate(&stream,Z_NO_FLUSH);
        const std::size_t produced=block.size()-stream.avail_out;
        if(produced>expanded || original.size()>expanded-produced)
            throw std::out_of_range("Original SZ DEFLATE exceeds declared uncompressed length");
        original.insert(original.end(),block.data(),block.data()+produced);
        if(rc==Z_STREAM_END)break;
        if(rc!=Z_OK)
            throw std::runtime_error("Malformed original SZ raw-DEFLATE stream");
        if(produced==0 && stream.avail_in==before && fed==data.size())
            throw std::runtime_error("Original SZ DEFLATE stream made no progress");
        if(produced==0 && stream.avail_in==0 && fed==data.size())
            throw std::out_of_range("Truncated original SZ raw-DEFLATE stream");
    }
    if(original.size()!=expanded)
        throw std::out_of_range("Original SZ decoded length differs from retail header");
    // zlib may have unread bytes in its current input chunk; no trailing junk
    // is valid after the one original member's deflate stream.
    if(fed!=data.size() || stream.avail_in!=0)
        throw std::invalid_argument("Original SZ member has trailing bytes after DEFLATE stream");
    return original;
}
} // namespace ae3
