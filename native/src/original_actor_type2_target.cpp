#include "ae3/original_actor_type2_target.hpp"
#include <stdexcept>

namespace ae3 {
OriginalActorType2Target_57bdb8 originalActorType2Target_57bdb8(
    std::span<const OriginalSpatialRecord70> sourceRecords,
    std::span<const std::uint32_t> sourceRoute) {
    if(sourceRoute.size()<2)
        throw std::out_of_range("Original type-2 route requires second polygon");
    const auto nextSourceIndex=sourceRoute[1];
    if(nextSourceIndex>=sourceRecords.size())
        throw std::out_of_range("Original type-2 route destination outside spatial registry");
    const auto& target=sourceRecords[nextSourceIndex];
    OriginalActorType2Target_57bdb8 result{};
    result.targetPolygonAt50=&target;
    for(std::size_t i=0;i<3;++i)
        result.rawOriginalVertices[i]=originalSpatialVector_21b458(target,i);
    return result;
}
} // namespace ae3
