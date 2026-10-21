#include "ae3/original_navgraph_topology.hpp"
#include <limits>
#include <stdexcept>

namespace ae3 {
namespace {
std::uint16_t u16(std::span<const std::byte> original,std::size_t at) {
    if(at>original.size()||original.size()-at<2)
        throw std::out_of_range("Original .nav descriptor truncated");
    return static_cast<std::uint16_t>(std::to_integer<unsigned char>(original[at])|
        (unsigned(std::to_integer<unsigned char>(original[at+1]))<<8));
}
std::uint32_t u32(std::span<const std::byte> original,std::size_t at) {
    return std::uint32_t(u16(original,at))|(std::uint32_t(u16(original,at+2))<<16);
}
}
OriginalNavGraphTopology::OriginalNavGraphTopology(
    std::span<const std::byte> source) {
    const auto count=static_cast<std::size_t>(u32(source,0x24));
    const auto relative=static_cast<std::size_t>(u32(source,0x3c));
    constexpr std::size_t recordStride=0x1e;
    if(relative>source.size()||count>(source.size()-relative)/recordStride)
        throw std::out_of_range("Original NavGraph descriptor array exceeds source data");
    links_.reserve(count);
    for(std::size_t index=0;index<count;index++) {
        const auto at=relative+index*recordStride;
        const auto numLinks=static_cast<std::size_t>(u16(source,at+2));
        // 0x21B404 loads the two-byte count and list at descriptor+4.
        // Original .nav descriptor reserves three u16 link slots.
        if(numLinks>3)
            throw std::out_of_range("Original NavGraph record has more than 3 linked neighbors");
        auto& links=links_.emplace_back();
        for(std::size_t k=0;k<numLinks;k++) {
            const auto target=u16(source,at+4+2*k);
            if(target>=count)
                throw std::out_of_range("Original NavGraph linked record index invalid");
            links.push_back(target);
        }
    }
}
std::span<const std::uint16_t> OriginalNavGraphTopology::links(std::size_t index) const {
    if(index>=links_.size())throw std::out_of_range("Original NavGraph record index invalid");
    return links_[index];
}
std::uint16_t OriginalNavGraphTopology::degree_21b3f8(std::size_t recordIndex) const {
    return static_cast<std::uint16_t>(links(recordIndex).size());
}
const OriginalSpatialRecord70* OriginalNavGraphTopology::neighbor_21b404(
    std::span<const OriginalSpatialRecord70> records,
    std::size_t recordIndex,std::size_t linkIndex) const {
    if(records.size()!=links_.size())
        throw std::invalid_argument("Source NavGraph topology/record count mismatch");
    const auto adjacency=links(recordIndex);
    // Genuine 0x21B420 branches to null when requested index >= count.
    if(linkIndex>=adjacency.size())return nullptr;
    return &records[adjacency[linkIndex]];
}
OriginalNavGraphOwned originalNavGraphConstruct_4f0b14(
    std::span<const std::uint8_t> bytes,
    std::uint32_t sourceVA,std::uint32_t graphVA) {
    const auto raw=std::as_bytes(bytes);
    // Validate source adjacency before original record allocation so
    // incomplete source files cannot partially construct live actors.
    OriginalNavGraphTopology topology(raw);
    const auto built=originalStageBuildSourceSpatial_21d46c({sourceVA,raw,graphVA});
    if(built.records.size()!=topology.size())
        throw std::runtime_error("Source original navigation counts diverged");
    return {std::move(built),std::move(topology)};
}
const OriginalSpatialRecord70* originalNavVisitAdjacent_21e648(
    const OriginalNavGraphOwned& graph,
    const OriginalSpatialGridFields& grid,float x,float y,
    const OriginalVU0Vec4& query,OriginalNavGraphWalkCalls& original) {
    const OriginalSpatialRecord70* selected=
        originalStageNearestGrid_21d898(graph.records,grid,x,y,query);
    if(!selected)selected=originalStageNearestAll_21da2c(graph.records,query);
    if(!selected)return nullptr;
    const auto span=std::span<const OriginalSpatialRecord70>(graph.records.records);
    const auto index=static_cast<std::size_t>(selected-span.data());
    const auto degree=graph.topology.degree_21b3f8(index);
    // 0x21E698..0x21E6E8 iterates count+1 times. The final
    // neighbor lookup (index==count) yields null at 0x21B404.
    const OriginalSpatialRecord70* current=selected;
    for(std::size_t k=0;k<=degree;k++) {
        if(current)original.call_original_21db64(*current);
        current=graph.topology.neighbor_21b404(span,index,k);
    }
    return selected;
}
std::size_t originalNavSlotFromName_4f0fbc(std::string_view value) noexcept {
    if(value=="nav_monkey.nav")return 0;
    if(value=="nav_enemy.nav")return 1;
    return 2;
}
void originalNavBindSlot_4f0b00(OriginalNavGraphSlots& state,
                               std::string_view name,
                               const OriginalNavGraphOwned& graph) noexcept {
    state.originalSlots_647b80[originalNavSlotFromName_4f0fbc(name)]=&graph;
}
const OriginalNavGraphOwned* originalNavLookupSlot_647b80(
    const OriginalNavGraphSlots& state,std::size_t index) noexcept {
    return index<state.originalSlots_647b80.size()?state.originalSlots_647b80[index]:nullptr;
}
} // namespace ae3
