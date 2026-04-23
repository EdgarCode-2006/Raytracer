#ifndef MATRIX4X4_HPP_
#define MATRIX4X4_HPP_

#include "math.hpp"
#include <cmath>
#include <numbers>

namespace Math {

struct Matrix4x4 {
    double m[4][4] = {};

    static Matrix4x4 identity() noexcept
    {
        Matrix4x4 r;
        r.m[0][0] = r.m[1][1] = r.m[2][2] = r.m[3][3] = 1.0;
        return r;
    }

    static Matrix4x4 translation(const Vector3D& t) noexcept
    {
        Matrix4x4 r = identity();
        r.m[0][3] = t.x();
        r.m[1][3] = t.y();
        r.m[2][3] = t.z();
        return r;
    }

    static Matrix4x4 scale(const Vector3D& s) noexcept
    {
        Matrix4x4 r = identity();
        r.m[0][0] = s.x();
        r.m[1][1] = s.y();
        r.m[2][2] = s.z();
        return r;
    }

    static Matrix4x4 rotationX(double deg) noexcept
    {
        double a = deg * std::numbers::pi / 180.0;
        Matrix4x4 r = identity();
        r.m[1][1] = std::cos(a);
        r.m[1][2] = -std::sin(a);
        r.m[2][1] = std::sin(a);
        r.m[2][2] = std::cos(a);
        return r;
    }

    static Matrix4x4 rotationY(double deg) noexcept
    {
        double a = deg * std::numbers::pi / 180.0;
        Matrix4x4 r = identity();
        r.m[0][0] = std::cos(a);
        r.m[0][2] = std::sin(a);
        r.m[2][0] = -std::sin(a);
        r.m[2][2] = std::cos(a);
        return r;
    }

    static Matrix4x4 rotationZ(double deg) noexcept
    {
        double a = deg * std::numbers::pi / 180.0;
        Matrix4x4 r = identity();
        r.m[0][0] = std::cos(a);
        r.m[0][1] = -std::sin(a);
        r.m[1][0] = std::sin(a);
        r.m[1][1] = std::cos(a);
        return r;
    }

    Matrix4x4 operator*(const Matrix4x4& b) const noexcept
    {
        Matrix4x4 r;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                for (int k = 0; k < 4; k++)
                    r.m[i][j] += m[i][k] * b.m[k][j];
        return r;
    }

    Vector3D transformPoint(const Vector3D& p) const noexcept
    {
        return {
            m[0][0]*p.x() + m[0][1]*p.y() + m[0][2]*p.z() + m[0][3],
            m[1][0]*p.x() + m[1][1]*p.y() + m[1][2]*p.z() + m[1][3],
            m[2][0]*p.x() + m[2][1]*p.y() + m[2][2]*p.z() + m[2][3]
        };
    }

    Vector3D transformDirection(const Vector3D& d) const noexcept
    {
        return {
            m[0][0]*d.x() + m[0][1]*d.y() + m[0][2]*d.z(),
            m[1][0]*d.x() + m[1][1]*d.y() + m[1][2]*d.z(),
            m[2][0]*d.x() + m[2][1]*d.y() + m[2][2]*d.z()
        };
    }
};

}

#endif
