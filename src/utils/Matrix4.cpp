#include "utils/Matrix4.hpp"
#include <cstdlib>

namespace RayTracer {

    Matrix4 Matrix4::inverse() const noexcept {
        // flatten to 1D for better visibility
        const double *s = &m[0][0];
        double inv[16]; // transposed cofactor matrix
        
        // knowing that : Cij​= (−1)^(i+j) * det(Mij​)
        inv[0] = s[5]*s[10]*s[15] - s[5]*s[11]*s[14] - s[9]*s[6]*s[15] + s[9]*s[7]*s[14] + s[13]*s[6]*s[11] - s[13]*s[7]*s[10]; // C00
        inv[4] = -s[4]*s[10]*s[15] + s[4]*s[11]*s[14] + s[8]*s[6]*s[15] - s[8]*s[7]*s[14] - s[12]*s[6]*s[11] + s[12]*s[7]*s[10]; // C01
        inv[8] = s[4]*s[9]*s[15] - s[4]*s[11]*s[13] - s[8]*s[5]*s[15] + s[8]*s[7]*s[13] + s[12]*s[5]*s[11] - s[12]*s[7]*s[9]; // C02
        inv[12] = -s[4]*s[9]*s[14] + s[4]*s[10]*s[13] + s[8]*s[5]*s[14] - s[8]*s[6]*s[13] - s[12]*s[5]*s[10] + s[12]*s[6]*s[9]; // C03

        inv[1] = -s[1]*s[10]*s[15] + s[1]*s[11]*s[14] + s[9]*s[2]*s[15] - s[9]*s[3]*s[14] - s[13]*s[2]*s[11] + s[13]*s[3]*s[10]; // C10
        inv[5] = s[0]*s[10]*s[15] - s[0]*s[11]*s[14] - s[8]*s[2]*s[15] + s[8]*s[3]*s[14] + s[12]*s[2]*s[11] - s[12]*s[3]*s[10]; // C11
        inv[9] = -s[0]*s[9]*s[15] + s[0]*s[11]*s[13] + s[8]*s[1]*s[15] - s[8]*s[3]*s[13] - s[12]*s[1]*s[11] + s[12]*s[3]*s[9]; // C12
        inv[13] = s[0]*s[9]*s[14] - s[0]*s[10]*s[13] - s[8]*s[1]*s[14] + s[8]*s[2]*s[13] + s[12]*s[1]*s[10] - s[12]*s[2]*s[9]; // C13

        inv[2] = s[1]*s[6]*s[15] - s[1]*s[7]*s[14] - s[5]*s[2]*s[15] + s[5]*s[3]*s[14] + s[13]*s[2]*s[7] - s[13]*s[3]*s[6]; // C20
        inv[6] = -s[0]*s[6]*s[15] + s[0]*s[7]*s[14] + s[4]*s[2]*s[15] - s[4]*s[3]*s[14] - s[12]*s[2]*s[7] + s[12]*s[3]*s[6]; // C21
        inv[10] = s[0]*s[5]*s[15] - s[0]*s[7]*s[13] - s[4]*s[1]*s[15] + s[4]*s[3]*s[13] + s[12]*s[1]*s[7] - s[12]*s[3]*s[5]; // C22
        inv[14] = -s[0]*s[5]*s[14] + s[0]*s[6]*s[13] + s[4]*s[1]*s[14] - s[4]*s[2]*s[13] - s[12]*s[1]*s[6] + s[12]*s[2]*s[5]; // C23

        inv[3] = -s[1]*s[6]*s[11] + s[1]*s[7]*s[10] + s[5]*s[2]*s[11] - s[5]*s[3]*s[10] - s[9]*s[2]*s[7] + s[9]*s[3]*s[6]; // C30
        inv[7] = s[0]*s[6]*s[11] - s[0]*s[7]*s[10] - s[4]*s[2]*s[11] + s[4]*s[3]*s[10] + s[8]*s[2]*s[7] - s[8]*s[3]*s[6]; // C31
        inv[11] = -s[0]*s[5]*s[11] + s[0]*s[7]*s[9] + s[4]*s[1]*s[11] - s[4]*s[3]*s[9] - s[8]*s[1]*s[7] + s[8]*s[3]*s[5]; // C32
        inv[15] = s[0]*s[5]*s[10] - s[0]*s[6]*s[9] - s[4]*s[1]*s[10] + s[4]*s[2]*s[9] + s[8]*s[1]*s[6] - s[8]*s[2]*s[5]; // C33

        // det = a00 * C00 + a01 * C01 + a02 * C02 + a03 * C03
        double det = s[0]*inv[0] + s[1]*inv[4] + s[2]*inv[8] + s[3]*inv[12];
        // if det == 0 -> singular matrix return the identity matrix
        if (std::abs(det) < 1e-15)
            return Matrix4{};
        det = 1.0 / det;
        Matrix4 result;

        // M⁻¹ = 1 / det(M) * adj(M)
        // adj(M) = C^T
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                result.m[i][j] = inv[i * 4 + j] * det; // i * 4 + j for the transposed cofactor index (adj(M))
        return result;
    }

    Math::Vector3D Matrix4::transformPoint(const Math::Vector3D &p) const noexcept {
        return {
            m[0][0]*p.x() + m[0][1]*p.y() + m[0][2]*p.z() + m[0][3],
            m[1][0]*p.x() + m[1][1]*p.y() + m[1][2]*p.z() + m[1][3],
            m[2][0]*p.x() + m[2][1]*p.y() + m[2][2]*p.z() + m[2][3]
        };
    }

    Math::Vector3D Matrix4::transformDirection(const Math::Vector3D &d) const noexcept {
        return {
            m[0][0]*d.x() + m[0][1]*d.y() + m[0][2]*d.z(),
            m[1][0]*d.x() + m[1][1]*d.y() + m[1][2]*d.z(),
            m[2][0]*d.x() + m[2][1]*d.y() + m[2][2]*d.z()
        };
    }
    
    AABB Matrix4::transformAABB(const AABB& box) const noexcept {
        const Math::Vector3D& mn = box.min();
        const Math::Vector3D& mx = box.max();
        const Math::Vector3D corners[8] = {
            {mn.x(), mn.y(), mn.z()}, {mx.x(), mn.y(), mn.z()},
            {mn.x(), mx.y(), mn.z()}, {mx.x(), mx.y(), mn.z()},
            {mn.x(), mn.y(), mx.z()}, {mx.x(), mn.y(), mx.z()},
            {mn.x(), mx.y(), mx.z()}, {mx.x(), mx.y(), mx.z()}
        };
        AABB result;
        for (const auto& c : corners)
            result.expand(transformPoint(c));
        return result;
    }
    
}