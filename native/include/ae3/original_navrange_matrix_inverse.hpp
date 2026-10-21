#pragma once
#include "ae3/original_navrange_mmi_pack.hpp"
#include <array>
#include <cstddef>

namespace ae3 {
// Source-derived mathematical reconstruction of 0x359F44..0x35A178.
// Retail computes 4D cofactors using VU0 cross products, calculates
// Q=1/determinant at 0x35A14C and stores four 16-byte matrix columns.
// This reproduces the cofactor/adjugate result, not bit-exact VU0
// rounding, flags or exceptional-value behavior.
inline OriginalNavrangeMatrix4 originalNavrangeInverseCofactors_359f44(
    const OriginalNavrangeMatrix4& input) noexcept {
    float a[4][4]{};
    for(std::size_t column=0;column<4;column++){
        const auto& v=input[column];
        a[0][column]=v.x; a[1][column]=v.y;
        a[2][column]=v.z; a[3][column]=v.w;
    }
    float cofactor[4][4]{};
    for(std::size_t row=0;row<4;row++){
        for(std::size_t col=0;col<4;col++){
            float m[3][3]{};
            std::size_t mr=0;
            for(std::size_t r=0;r<4;r++){
                if(r==row)continue;
                std::size_t mc=0;
                for(std::size_t c=0;c<4;c++){
                    if(c==col)continue;
                    m[mr][mc++]=a[r][c];
                }
                ++mr;
            }
            const float determinant3=
                m[0][0]*(m[1][1]*m[2][2]-m[1][2]*m[2][1])
                -m[0][1]*(m[1][0]*m[2][2]-m[1][2]*m[2][0])
                +m[0][2]*(m[1][0]*m[2][1]-m[1][1]*m[2][0]);
            cofactor[row][col]=((row+col)&1u)?-determinant3:determinant3;
        }
    }
    float determinant=0.f;
    for(std::size_t c=0;c<4;c++)
        determinant+=a[0][c]*cofactor[0][c];
    // Original VDIV uses VF0.w (=1). Do not invent an identity fallback.
    const float reciprocalDeterminant=1.f/determinant;
    OriginalNavrangeMatrix4 output{};
    for(std::size_t col=0;col<4;col++){
        const auto& r=cofactor[col]; // adjugate transpose
        output[col]={r[0]*reciprocalDeterminant,r[1]*reciprocalDeterminant,
                     r[2]*reciprocalDeterminant,r[3]*reciprocalDeterminant};
    }
    return output;
}
// Retail caller at 0x21EFA4 -> 0x21EFDC constructs quaternion matrix,
// copies 4 matrix columns, and inverts it. Input is already-extracted
// original quaternion data; the game's resource extraction is unresolved.
inline OriginalNavrangeMatrix4 originalNavrangeInverseQuaternion_21efa4_21efdc(
    OriginalNavrangeGpr128 originalQuaternionWords) noexcept {
    const auto originalMatrix=originalNavrangeQuaternionMatrix_359e74(
        originalQuaternionWords);
    return originalNavrangeInverseCofactors_359f44(originalMatrix);
}
} // namespace ae3
