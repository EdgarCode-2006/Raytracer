#ifndef MATH_HPP_
#define MATH_HPP_
#include <cassert>
#include <cmath>

namespace Math {

    class Vector3D {
        public:
            double coords[3];

            constexpr Vector3D() noexcept : coords{0.0, 0.0, 0.0} {}

            constexpr Vector3D(double x, double y, double z) noexcept : coords{x, y, z} {}

            constexpr double operator[](int i) const noexcept {
                return coords[i];
            }

            constexpr double& operator[](int i) noexcept {
                return coords[i];
            }

            constexpr double x() const noexcept {
                return coords[0];
            }
            constexpr double y() const noexcept {
                return coords[1];
            }
            constexpr double z() const noexcept {
                return coords[2];
            }

            constexpr double& x() noexcept {
                return coords[0];
            }
            constexpr double& y() noexcept {
                return coords[1];
            }
            constexpr double& z() noexcept {
                return coords[2];
            }

            constexpr Vector3D operator+(const Vector3D& b) const noexcept {
                return {coords[0] + b.coords[0],
                        coords[1] + b.coords[1],
                        coords[2] + b.coords[2]};
            }

            constexpr Vector3D operator-(const Vector3D& b) const noexcept {
                return {coords[0] - b.coords[0],
                        coords[1] - b.coords[1],
                        coords[2] - b.coords[2]};
            }

            constexpr bool operator==(const Vector3D& b) const noexcept {
                return (coords[0] == b.coords[0] &&
                        coords[1] == b.coords[1] &&
                        coords[2] == b.coords[2]);
            }

            constexpr Vector3D operator*(double t) const noexcept {
                return {coords[0] * t,
                        coords[1] * t,
                        coords[2] * t};
            }

            constexpr Vector3D operator*(const Vector3D& other) const noexcept {
                return {coords[0] * other.x(),
                        coords[1] * other.y(),
                        coords[2] * other.z()};
            }

            constexpr Vector3D operator/(double t) const noexcept {
                double inv = 1.0 / t;
                return {coords[0] * inv,
                        coords[1] * inv,
                        coords[2] * inv};
            }

            constexpr Vector3D& operator+=(const Vector3D& b) noexcept {
                coords[0] += b.coords[0];
                coords[1] += b.coords[1];
                coords[2] += b.coords[2];
                return *this;
            }

            constexpr Vector3D& operator+=(double t) noexcept {
                coords[0] += t;
                coords[1] += t;
                coords[2] += t;
                return *this;
            }

            constexpr Vector3D& operator-=(const Vector3D& b) noexcept {
                coords[0] -= b.coords[0];
                coords[1] -= b.coords[1];
                coords[2] -= b.coords[2];
                return *this;
            }

            constexpr Vector3D& operator*=(double t) noexcept {
                coords[0] *= t;
                coords[1] *= t;
                coords[2] *= t;
                return *this;
            }

            constexpr Vector3D& operator/=(double t) noexcept {
                double inv = 1.0 / t;
                coords[0] *= inv;
                coords[1] *= inv;
                coords[2] *= inv;
                return *this;
            }

            constexpr double dot(const Vector3D& b) const noexcept {
                return coords[0] * b.coords[0]
                    + coords[1] * b.coords[1]
                    + coords[2] * b.coords[2];
            }

            constexpr Vector3D cross(const Vector3D& b) const noexcept {
                return {
                    coords[1] * b.coords[2] - coords[2] * b.coords[1],
                    coords[2] * b.coords[0] - coords[0] * b.coords[2],
                    coords[0] * b.coords[1] - coords[1] * b.coords[0]
                };
            }

            double length() const noexcept {
                return std::sqrt(
                    coords[0] * coords[0] +
                    coords[1] * coords[1] +
                    coords[2] * coords[2]
                );
            }

            Vector3D normalize() const noexcept {
                double len = length();
                if (len == 0.0)
                    return {};
                double inv = 1.0 / len;
                return {coords[0] * inv,
                        coords[1] * inv,
                        coords[2] * inv};
            }
        };

        constexpr inline Vector3D operator*(double t, const Vector3D& v) noexcept {
            return v * t;
        }
}

constexpr inline Math::Vector3D min(const Math::Vector3D& a, const Math::Vector3D& b) noexcept {
    return {
        a.coords[0] < b.coords[0] ? a.coords[0] : b.coords[0],
        a.coords[1] < b.coords[1] ? a.coords[1] : b.coords[1],
        a.coords[2] < b.coords[2] ? a.coords[2] : b.coords[2]
    };
}

constexpr inline Math::Vector3D max(const Math::Vector3D& a, const Math::Vector3D& b) noexcept {
    return {
        a.coords[0] > b.coords[0] ? a.coords[0] : b.coords[0],
        a.coords[1] > b.coords[1] ? a.coords[1] : b.coords[1],
        a.coords[2] > b.coords[2] ? a.coords[2] : b.coords[2]
    };
}

constexpr inline Math::Vector3D clamp(const Math::Vector3D& v, const Math::Vector3D& minV, const Math::Vector3D& maxV) noexcept {
    return {
        v.coords[0] < minV.coords[0] ? minV.coords[0] :
        (v.coords[0] > maxV.coords[0] ? maxV.coords[0] : v.coords[0]),

        v.coords[1] < minV.coords[1] ? minV.coords[1] :
        (v.coords[1] > maxV.coords[1] ? maxV.coords[1] : v.coords[1]),

        v.coords[2] < minV.coords[2] ? minV.coords[2] :
        (v.coords[2] > maxV.coords[2] ? maxV.coords[2] : v.coords[2])
    };
}

#endif
