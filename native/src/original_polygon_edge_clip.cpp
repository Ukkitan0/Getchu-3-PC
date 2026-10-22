#include "ae3/original_polygon_edge_clip.hpp"
#include "ae3/original_actor_ray_geometry.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
namespace ae3 {
bool originalPolygonClipEdge_21ee00(
    const OriginalVU0Vec4& from,const OriginalVU0Vec4& to,
    const OriginalVU0Vec4& origin,const OriginalVU0Vec4& direction,
    OriginalPolygonRayDistanceBounds_21bd3c& bounds) noexcept {
    // 0x21EE10..0x21EE5C: original edge VSUB and original
    // world-up VOPMULA/VOPMSUB source inward half-plane normal.
    const OriginalVU0Vec4 normal{to.z-from.z,0.0f,-(to.x-from.x),0.0f};
    // 0x21EE68: exact same native source 0x4748AC XYZ dot as
    // earlier recovered actor 0x4C1B3C projection-ray preparation.
    const float denom=originalVU0DotXYZ_4748ac(normal,direction);
    // Owner retail 0x6E3EF8 raw original single 0x358637BD (~1e-6).
    constexpr float parallelEpsilon=std::bit_cast<float>(std::uint32_t{0x358637bdu});
    if(std::fabs(denom)<parallelEpsilon) {
        // 0x21EE90 / 0x21EEA0 two original XYZ dots;
        // 0x21EEA8 original C.OLE.S edge <= ray position.
        const float boundary=originalVU0DotXYZ_4748ac(normal,from);
        const float position=originalVU0DotXYZ_4748ac(normal,origin);
        return boundary<=position;
    }
    // 0x21EEDC / 0x21EEEC and original FPU SUB/DIV.
    const float boundary=originalVU0DotXYZ_4748ac(normal,from);
    const float position=originalVU0DotXYZ_4748ac(normal,origin);
    const float numerator=boundary-position;
    const float fraction=numerator/denom;
    // 0x21EEF8: C.OLT.S 0,denom.
    if(0.0f<denom)
        // 0x21EF14 MAX.S at packet+0x20.
        bounds.originalLowerAt20=std::max(bounds.originalLowerAt20,fraction);
    else
        // 0x21EF30 MIN.S at packet+0x24.
        bounds.originalUpperAt24=std::min(bounds.originalUpperAt24,fraction);
    // 0x21EF2C C.OLT.S lower,upper. Strictly <.
    return bounds.originalLowerAt20<bounds.originalUpperAt24;
}
} // namespace ae3
