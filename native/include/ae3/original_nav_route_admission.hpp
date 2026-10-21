#pragma once
#include "ae3/original_nav_route_search.hpp"
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace ae3 {
// Original 0x2207D0 allocates 0x3800 bytes for 0x38-byte nodes.
inline constexpr std::size_t kOriginalNavWorkspaceCapacity=0x3800/0x38;

// Original 0x21AFB8 begins a visit epoch before the first 0x220AA4
// call. On uint32 wrap it clears all polygon marks at offset +0x28.
// Native storage preserves the state without emulating guest pointers.
struct OriginalNavEpochState {
    std::uint32_t epoch=0;
    std::vector<std::uint32_t> lastSeenEpoch;
    std::vector<std::uint8_t> seen;
};
void originalNavStartEpoch_21afb8(OriginalNavEpochState&,std::size_t originalPolygonCount);
bool originalNavAlreadySeen_21b014(OriginalNavEpochState&,std::size_t sourceIndex);
void originalNavMarkSeen_21b038(OriginalNavEpochState&,std::size_t sourceIndex);

// Original 0x4E3DAC tests exact pointer membership in a linked list.
// 0x21F89C forwards the game query to that predicate.
bool originalNavMembership_4e3dac(
    const OriginalSpatialRecord70* originalPolygon,
    std::span<const OriginalSpatialRecord70* const> originalList) noexcept;

struct OriginalNavAdmissionRequest {
    std::uint32_t startRecordIndex=0;
    OriginalVU0Vec4 query{};
    float maximumDistance=0;
    bool filterEnabled=false;
    bool originalFilterListExists=false;
    std::span<const OriginalSpatialRecord70* const> originalFilterList;
};
struct OriginalNavAdmissionResult {
    std::vector<std::uint32_t> sourceRecordIndicesInDepthFirstOrder;
    std::size_t candidateRecordsVisited=0;
};

// Recover 0x220AA4: visit guard, optional 0x21F89C membership,
// 0x21B468 source centroid, strict float distance threshold,
// 0x21B038 mark, append 0x38 record and DFS through 0x21B404.
// The host uses an iterative DFS to avoid C++ stack overflow.
OriginalNavAdmissionResult originalNavAdmit_220aa4(
    std::span<const OriginalSpatialRecord70> originalRecords,
    std::span<const std::byte> originalNavBytes,
    const OriginalNavAdmissionRequest&,
    OriginalNavEpochState&);

// Reconstruct the 0x220C00 source links ONLY between admitted nodes;
// retain original edge type and the source adjacency slot.
OriginalNavRouteWorkspace originalNavBuildAdmittedWorkspace_220c00(
    std::span<const OriginalSpatialRecord70> originalRecords,
    std::span<const std::byte> originalNavBytes,
    const OriginalNavAdmissionResult&);

// A source-backed *partial* 0x2207D0 pipeline for admission,
// relaxation and a nearest-endpoint route. Actual 0x2207D0 mode and
// fallback via 0x21F8B8 are not fully translated; do not interpret
// this convenience function as complete retail game pathfinding.
std::vector<const OriginalSpatialRecord70*> originalNavRouteFromSource_2207d0(
    std::span<const OriginalSpatialRecord70> originalRecords,
    std::span<const std::byte> originalNavBytes,
    const OriginalNavAdmissionRequest&,
    OriginalNavRouteSplineCalls* originalSpline=nullptr);
} // namespace ae3
