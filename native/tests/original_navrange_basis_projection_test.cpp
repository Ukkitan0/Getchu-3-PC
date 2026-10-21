#include "ae3/original_navrange_basis_projection.hpp"
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
using namespace ae3;
#define ck(v) do { if(!(v)) throw std::runtime_error("FAILED: " #v); } while(false)
static bool eq(float a,float b,float epsilon=1.e-5f){return std::fabs(a-b)<=epsilon*(1.f+std::fabs(b));}
int main() {
    const std::array<OriginalVU0Vec4,3> axes{{
        {2.f,0.f,0.f,4.f},{0.f,3.f,0.f,-6.f},{0.f,0.f,-5.f,10.f}}};
    const auto x=originalNavrangePreparedBasisProjections_21f024_21f1b8(
        {5.f,-7.f,11.f,12.f},axes);
    ck(eq(x.axisProjections[0],5.f));
    ck(eq(x.axisProjections[1],-7.f));
    ck(eq(x.axisProjections[2],-11.f));
    ck(eq(x.bases[0].squaredXYZ,4.f) && eq(x.bases[1].squaredXYZ,9.f));
    ck(eq(x.bases[2].squaredXYZ,25.f));
    ck(eq(x.bases[0].reciprocalLength,0.5f));
    // Source VMULq.xyzw scales W although projected scalar only sums XYZ.
    ck(eq(x.bases[0].value.w,2.f));
    ck(eq(x.bases[1].value.w,-2.f));
    ck(eq(x.bases[2].value.w,2.f));
    const OriginalVU0Vec4 oblique{3.f,4.f,12.f,26.f};
    const auto y=originalNavrangeNormalizeBasis_21f024(oblique);
    ck(eq(y.squaredXYZ,169.f));
    ck(eq(y.reciprocalLength,1.f/13.f));
    ck(eq(y.value.x,3.f/13.f));
    ck(eq(y.value.y,4.f/13.f));
    ck(eq(y.value.z,12.f/13.f));
    ck(eq(originalNavrangeProjectionXYZ_21f0b0(
        {3.f,4.f,12.f,200.f},y.value),13.f));
    // No fabricated normalization fallback for a degenerate basis.
    const auto z=originalNavrangeNormalizeBasis_21f024({0.f,0.f,0.f,0.f});
    ck(z.squaredXYZ==0.f);
    ck(!std::isfinite(z.reciprocalLength));
    ck(std::isnan(z.value.x));
    // FPU numerical artifacts remain permitted until retail-VU parity tests.
    for(int i=1;i<=1000;++i) {
        const float q=static_cast<float>(i)*0.25f;
        const auto a=originalNavrangeNormalizeBasis_21f024({q,0.f,0.f,0.f});
        ck(eq(a.value.x,1.f));
        ck(eq(a.value.y,0.f));
        const auto v=originalNavrangeProjectionXYZ_21f0b0({q,0.f,0.f,10.f},a.value);
        ck(eq(v,q));
    }
}
