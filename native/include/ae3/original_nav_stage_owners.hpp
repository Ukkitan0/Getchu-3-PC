#pragma once
#include "ae3/original_nav_pck_bridge.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ae3 {
// 0x21ED30 allocates exactly three original 0x28-byte owners at
// 0x647B80 + (slot*4), then calls 0x21D3D0 for each.
// 0x21D3D0 clears owner words +0,+4,+0x10 and initially zeros +0x18..+0x24
// through 0x21AF94; later +0x18 is overwritten with record pointer
// and +0x20 with record count. +0x0C is the -1 sentinel.
// Typed *semantic* host state, NOT a binary guest 0x28-byte struct.
struct OriginalNavOwnerFields_21d3d0 {
    bool sourceHeaderPointerPresent=false;      // original +0x00 non-null
    bool sourceRecordsPointerPresent=false;     // original +0x04 non-null
    std::int32_t sourceGridIndexAt0c=-1;          // original +0x0c
    bool sourceGridPointerPresent=false;        // original +0x10, still 0 after 0x21D46C
    bool sourceRecordListPointerPresent=false;  // original +0x18, set by 0x21D56C
    std::uint32_t sourceRecordCountAt14=0;      // set by original 0x21D568
    std::uint32_t sourceRecordCountAt20=0;      // set by original 0x21D564
    // Source +0x18 is a live record-array pointer, and +0x20
    // overlaps the word initially cleared by 0x21AF94. Do NOT model
    // +0x18..0x24 as an independent persistent 4-word list.
};
// This owner uses host lifetime management, never PS2 32-bit pointer
// reinterpretation. Its backing retail bytes are kept owned while the
// reconstructed original graph constructor / actor-data readers use it.
class OriginalNavStageOwners_21ed30 final {
public:
    struct Slot {
        OriginalNavOwnerFields_21d3d0 sourceFields{};
        std::string originalResourceName{};
        std::vector<std::uint8_t> originalSourceAllocation{};
        std::optional<OriginalStageSpatialLoaded> decodedGraph{};
        std::uint32_t originalSourceVA=0;
        std::uint32_t originalOwnerVA=0;
        std::size_t decodedRecordCount=0;
        bool active() const noexcept {return decodedGraph.has_value();}
    };
    // Original stage init 0x21ED30 calls owner constructor 0x21D3D0
    // in ascending slot order, installing all three owners.
    void originalInitialize_21ed30() {
        originalTeardown_21ed94();
        for(auto& slot:slots_)slot=std::make_unique<Slot>();
    }
    bool initialized() const noexcept {
        for(const auto& s:slots_)if(!s)return false;
        return true;
    }
    const Slot& originalAt(std::size_t slot) const {
        if(slot>=slots_.size()||!slots_[slot])
            throw std::out_of_range("Original nav owner slot not initialized");
        return *slots_[slot];
    }
    // 0x4F0A5C uses the source name to select its 0..2 slot and then
    // passes the source data and that slot's owner into 0x21D46C.
    // The runtime VA values are required real lifecycle inputs; no
    // game memory addresses are manufactured from this retail PCK.
    void originalAttach_4f0a5c(
        const OriginalNavSource& source,
        const OriginalNavRuntimeSourceBinding_4f0a5c& binding) {
        if(!initialized())throw std::logic_error("Original 0x21ED30 must initialize nav owners");
        if(source.originalSlot>=slots_.size()||
           source.originalFilename!=binding.originalName||
           !binding.originalSourceRuntimeVA||!binding.originalGraphOwnerVA||
           source.originalNavBytes.empty())
            throw std::invalid_argument("Original stage nav allocation/owner identity missing");
        // Construct fully before replacing an active graph; this adds
        // native exception safety, not original R5900 failure parity.
        auto decoded=originalNavLoadGraph_4f0a5c(source,
            binding.originalSourceRuntimeVA,binding.originalGraphOwnerVA);
        std::vector<std::uint8_t> owned=source.originalNavBytes;
        auto& target=*slots_[source.originalSlot];
        originalReleaseSlot_21d7d8(source.originalSlot);
        target.originalResourceName=source.originalFilename;
        target.originalSourceAllocation=std::move(owned);
        target.originalSourceVA=binding.originalSourceRuntimeVA;
        target.originalOwnerVA=binding.originalGraphOwnerVA;
        target.decodedRecordCount=decoded.records.size();
        target.decodedGraph.emplace(std::move(decoded));
        target.sourceFields.sourceHeaderPointerPresent=true;
        target.sourceFields.sourceRecordsPointerPresent=true;
        target.sourceFields.sourceRecordListPointerPresent=true;
        // 0x21D46C does not populate owner+0x10 (grid pointer).
        target.sourceFields.sourceRecordCountAt14=static_cast<std::uint32_t>(target.decodedRecordCount);
        target.sourceFields.sourceRecordCountAt20=static_cast<std::uint32_t>(target.decodedRecordCount);
    }
    void originalReleaseSlot_21d7d8(std::size_t slot) {
        if(slot>=slots_.size()||!slots_[slot])
            throw std::out_of_range("Original 0x21D7D8 owner does not exist");
        auto& s=*slots_[slot];
        // Original 0x21D7D8 frees header, calls 0x21B384 on the
        // 0x70-byte records, releases record array, then grid +0x10.
        // Host graph destruction frees its native owned data. Pending
        // actor actions are NOT constructed child objects and may not
        // be passed to the original actor destructor.
        s.decodedGraph.reset();
        s.originalSourceAllocation.clear();
        s.originalResourceName.clear();
        s.originalSourceVA=s.originalOwnerVA=0;
        s.decodedRecordCount=0;
        s.sourceFields=OriginalNavOwnerFields_21d3d0{};
    }
    // 0x21ED94 calls 0x21D43C -> 0x21D7D8 for all three owners,
    // frees their 0x28-byte allocations and clears 0x647B80 slots.
    void originalTeardown_21ed94() noexcept {
        for(auto& slot:slots_)slot.reset();
    }
    ~OriginalNavStageOwners_21ed30()=default;
private:
    std::array<std::unique_ptr<Slot>,3> slots_{};
};
} // namespace ae3
