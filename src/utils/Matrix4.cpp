#include "utils/Matrix4.hpp"
#include <cstdlib>

namespace RayTracer {

    Matrix4 Matrix4::inverse() const noexcept {
        // flatten to 1D for better visibility
        const double *s = &m[0][0];
        double inv[16]; // transposed cofactor matrix
        
        // knowing that : Cij​= (−1)^(i+j) * det(Mij​)
        inv[0] = s[5]*s[10]*s[15] - s[5]*s[11]*s[14] - s[9]*s[6]*s[15] + s[9]*s[7]*s[14] + s[13]*s[6]*s[11] - s[13]*s[7]*s[10];
        inv[4] = -s[4]*s[10]*s[15] + s[4]*s[11]*s[14] + s[8]*s[6]*s[15] - s[8]*s[7]*s[14] - s[12]*s[6]*s[11] + s[12]*s[7]*s[10];
        inv[8] = s[4]*s[9]*s[15] - s[4]*s[11]*s[13] - s[8]*s[5]*s[15] + s[8]*s[7]*s[13] + s[12]*s[5]*s[11] - s[12]*s[7]*s[9];
        inv[12] = -s[4]*s[9]*s[14] + s[4]*s[10]*s[13] + s[8]*s[5]*s[14] - s[8]*s[6]*s[13] - s[12]*s[5]*s[10] + s[12]*s[6]*s[9];
        
        // det = a00 * C00 + a01 * C01 + a02 * C02 + a03 * C03
        double det = s[0]*inv[0] + s[1]*inv[4] + s[2]*inv[8] + s[3]*inv[12];
        // if det == 0 -> singular matrix return the identity matrix
        if (std::abs(det) < 1e-15)
            return Matrix4{};
        Matrix4 result;

        return result;
    }
}