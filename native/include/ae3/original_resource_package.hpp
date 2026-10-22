#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace ae3 {
// Original USA PCK table as verified for woods_c/exdb.pck:
// "PCK\0" at +0, table offset +4, count +8; each 0x10-byte item
// holds absolute name/type C-string offsets followed by the original
// payload's absolute PCK offset and byte length. Unlike a general PCK
// format conjecture, this reader intentionally rejects unsupported layouts.
// Retail consumer 0x26D24C selects resources by type; 0x26D2B8..0x26D2E0
// compares the request prefix and accepts only a NUL or SPACE suffix.
struct OriginalPckResourceView {
    std::string_view name;
    std::string_view type;
    std::span<const std::byte> payload;
    std::uint32_t payloadOffset = 0;
    std::uint32_t sourceIndex = 0;
};

enum class OriginalPckPayloadSignature { unknown, i3d_mesh, i3d_animation, i3d_collision, tim2, exdb };

inline OriginalPckPayloadSignature originalPckPayloadSignature(std::span<const std::byte> data) noexcept {
    auto starts = [data](const char* s, std::size_t n) {
        if (data.size() < n) return false;
        for (std::size_t i = 0; i < n; ++i)
            if (data[i] != std::byte(static_cast<unsigned char>(s[i]))) return false;
        return true;
    };
    if (starts("I3D_BIN",7)) return OriginalPckPayloadSignature::i3d_mesh;
    if (starts("I3D_I3M",8)) return OriginalPckPayloadSignature::i3d_animation;
    if (starts("I3D_I3C",8)) return OriginalPckPayloadSignature::i3d_collision;
    if (starts("TIM2",4)) return OriginalPckPayloadSignature::tim2;
    if (starts("EDB\n",4)) return OriginalPckPayloadSignature::exdb;
    return OriginalPckPayloadSignature::unknown;
}

inline bool originalPckRetailTypeMatch_26d2b8(std::string_view stored,
                                               std::string_view requested) noexcept {
    if (!stored.starts_with(requested)) return false;
    return stored.size() == requested.size() || stored[requested.size()] == ' ' ||
           stored[requested.size()] == '\0';
}

// The source package allocation and every borrowed name/type/payload span
// share one unmovable owner. Do not expose movable views into ephemeral
// extraction buffers, and do not synthesize 32-bit EE pointers.
class OriginalPckResourcePackage final {
public:
    explicit OriginalPckResourcePackage(std::vector<std::byte> originalBytes)
        : bytes_(std::move(originalBytes)) { parse(); }
    OriginalPckResourcePackage(const OriginalPckResourcePackage&) = delete;
    OriginalPckResourcePackage& operator=(const OriginalPckResourcePackage&) = delete;
    OriginalPckResourcePackage(OriginalPckResourcePackage&&) = delete;
    OriginalPckResourcePackage& operator=(OriginalPckResourcePackage&&) = delete;

    std::span<const OriginalPckResourceView> entries() const noexcept { return entries_; }
    std::span<const std::byte> originalPackageBytes() const noexcept { return bytes_; }
    std::optional<std::size_t> first(std::string_view name,
                                     std::string_view type) const noexcept {
        for (std::size_t i = 0; i < entries_.size(); ++i)
            if (entries_[i].name == name && entries_[i].type == type) return i;
        return std::nullopt;
    }
    const OriginalPckResourceView& require(std::string_view name,
                                           std::string_view type) const {
        const auto index = first(name,type);
        if (!index) throw std::out_of_range("Original PCK resource missing: exact name/type required");
        return entries_[*index];
    }
    std::vector<std::size_t> selectType_26d24c(std::string_view originalRequest) const {
        std::vector<std::size_t> indices;
        for (std::size_t i = 0; i < entries_.size(); ++i)
            if (originalPckRetailTypeMatch_26d2b8(entries_[i].type, originalRequest))
                indices.push_back(i);
        return indices;
    }
private:
    static std::uint32_t read32(std::span<const std::byte> source, std::size_t at) {
        if (at > source.size() || source.size()-at < 4)
            throw std::out_of_range("Original PCK truncated table word");
        std::uint32_t value=0;
        for (unsigned j=0;j<4;++j)
            value |= std::uint32_t(std::to_integer<unsigned char>(source[at+j])) << (j*8);
        return value;
    }
    std::string_view sourceString(std::uint32_t pos, std::size_t poolEnd) const {
        // Absolute PCK offsets verified for the original woods_c EXDB sample;
        // do not reinterpret as base+offset when an offset points into a string.
        if (pos < 0x10 || pos >= poolEnd)
            throw std::out_of_range("Original PCK name/type pointer outside string pool");
        const auto* begin=bytes_.data()+pos;
        std::size_t length=0;
        while (pos+length < poolEnd && bytes_[pos+length]!=std::byte{0}) ++length;
        if (!length || pos+length==poolEnd)
            throw std::out_of_range("Original PCK string missing/empty in pool");
        return {reinterpret_cast<const char*>(begin),length};
    }
    void parse() {
        const std::span<const std::byte> data(bytes_);
        if (data.size() < 0x10 || data[0]!=std::byte{'P'} ||
            data[1]!=std::byte{'C'} || data[2]!=std::byte{'K'} || data[3]!=std::byte{0})
            throw std::invalid_argument("Source is not an original PCK resource package");
        const std::size_t table=read32(data,4), count=read32(data,8);
        if (table < 0x10 || table>data.size() || count>(data.size()-table)/0x10)
            throw std::out_of_range("Original PCK resource table exceeds source allocation");
        const std::size_t tableEnd=table+count*0x10;
        entries_.reserve(count);
        for (std::size_t i=0;i<count;++i) {
            const std::size_t at=table+i*0x10;
            const auto name=sourceString(read32(data,at),table);
            const auto type=sourceString(read32(data,at+4),table);
            const auto offset=read32(data,at+8), length=read32(data,at+12);
            if (offset<tableEnd || offset>data.size() || length>data.size()-offset)
                throw std::out_of_range("Original PCK payload outside source allocation");
            entries_.push_back({name,type,data.subspan(offset,length),offset,
                                static_cast<std::uint32_t>(i)});
        }
    }
    std::vector<std::byte> bytes_;
    std::vector<OriginalPckResourceView> entries_;
};
} // namespace ae3
